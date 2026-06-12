#include "Renderer.h"
#include "Colour.h"
#include "Globals.h"
#include "Hash.h"
#include "Logging.h"
#include "Object.h"
#include "PostProcess.h"
#include "Scene.h"
#include "Timer.h"
#include "Transform.h"
#include "Utility.h"
#include "Vector.h"
#include "Window.h"
#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <minmax.h>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace Renderer {
void clearScreen(uint32_t color) {
    uint32_t *pixel = (uint32_t *)renderState.memory;
    float *dep = (float *)depthBuffer;
    int bufferSize = renderState.width * renderState.height;
    for (int i = 0; i < bufferSize; i++) {
        *pixel++ = color;
        *dep++ = 0;
    }
}
// Put pixel (x and y specify viewport coordinates)
// this means x=0,y=0 will be on center
void putPixel(const int x, const int y, const Colour &color) {
    const uint32_t hexColor = rgbtoHex(color);
    const uint32_t idx = (x + renderState.width / 2) + (((renderState.height / 2) - y) * renderState.width);
    std::lock_guard<std::mutex> lock(pixelLocks[idx]);
    ((uint32_t *)renderState.memory)[idx] = hexColor;
}
// put pixel Direct (x and y specify buffer value)
// x=0,y=0 will be on top left
void putPixelD(const int x, const int y, const Colour &color) {
    uint32_t hexColor = rgbtoHex(color);
    uint32_t idx = x + (y * renderState.width);
    std::lock_guard<std::mutex> lock(pixelLocks[idx]);
    ((uint32_t *)renderState.memory)[idx] = hexColor;
}
void renderDepthBuffer() {
    for (uint32_t y = 0; y < renderState.height; y++) {
        for (uint32_t x = 0; x < renderState.width; x++) {
            uint32_t index = x + (y * renderState.width);
            float value = ((float *)depthBuffer)[index];
            clamp(value, 0.f, 1.f);
            Colour color = { (uint8_t)((value) * 255.f), (uint8_t)((value) * 255.f), (uint8_t)((value) * 255.f) };
            putPixelD(x, y, color);
        }
    }
}
Colour getPixel(const int x, const int y) {
    uint32_t *pixel = (uint32_t *)renderState.memory + x + (y * renderState.width);
    Colour result = hexToRGB(*pixel);
    return result;
}
void drawSquare(float x, float y, int size, Colour color) {
    x -= size * 0.5f;
    y -= size * 0.5f;
    for (int i = int(y); i < y + size; i++) {
        for (int j = int(x); j < x + size; j++) {
            putPixel(j, i, color);
        }
    }
}
void drawNoise() {
    for (int y = -(canvas.y / 2.f); y < (canvas.y / 2.f); y++) {
        for (int x = -(canvas.x / 2.f); x < (canvas.x / 2.f); x++) {
            Colour color = { uint8_t(rand() % 256), uint8_t(rand() % 256), uint8_t(rand() % 256) };
            putPixel(x, y, color);
        }
    }
}
void printPPM(const std::string &filename) {
    uint32_t sbsize = (renderState.width * renderState.height) * sizeof(uint32_t);
    uint32_t *buffer = (uint32_t *)malloc(sbsize);
    if (buffer) {
        memcpy((void *)buffer, renderState.memory, sbsize);
        ppmThreads.push_back(std::thread(Renderer::exportToPPM, filename, buffer, renderState.width, renderState.height));
    } else {
        LOG_WARN("Failed to allocate a buffer");
    }
}

void exportToPPM(const std::string &filename, uint32_t *buffer, int width, int height) {
    Timer timer;
    FILE *file;
    fopen_s(&file, filename.c_str(), "w");
    if (!file) {
        LOG_ERROR("Failed to open file : " << filename << "\n");
        return;
    }
    std::string output = "";
    output += ("P3\n" + std::to_string(width) + ' ' + std::to_string(height) + "\n255\n");
    int bufferSize = width * height;
    for (int i = 0; i < bufferSize; i++) {
        uint32_t currentColor = buffer[i];
        output += std::string((std::to_string((currentColor >> 16) & 0xFF)) + ' ' + std::to_string((currentColor >> 8) & 0xFF) + ' ' + std::to_string(currentColor & 0xFF) + '\n');
    }
    fwrite(output.data(), sizeof(char), output.length(), file);
    fclose(file);
    timer.Stop();
    free(buffer);
    LOG_SUCCESS(("Exported to " + filename + " took:" + std::to_string(timer.dtms) + "ms\n"));
}

void drawLine(Vector a, Vector b, const Colour &color) {
    float dy = b.y - a.y;
    float dx = b.x - a.x;
    if (abs(dx) > abs(dy)) {
        if (a.x > b.x) {
            swap(a, b);
        }
        float aspectRatio = 0;
        if (dx != 0)
            aspectRatio = (dy / dx);
        float y = a.y;
        for (int x = a.x; x <= b.x; x++) {
            if (x >= (canvas.x / 2) || x <= -(canvas.x / 2) || y >= (canvas.y / 2) || y <= -(canvas.y / 2)) {
                y += aspectRatio;
            } else {
                putPixel(x, y, color);
                y += aspectRatio;
            }
        }
    } else {
        if (a.y > b.y) {
            swap(a, b);
        }
        float aspectRatio = 0;
        if (dy != 0)
            aspectRatio = (dx / dy);
        float x = a.x;
        for (int y = a.y; y <= b.y; y++) {
            if (x >= (canvas.x / 2) || x <= -(canvas.x / 2) || y >= (canvas.y / 2) || y <= -(canvas.y / 2)) {
                x += aspectRatio;
            } else {
                putPixel(x, y, color);
                x += aspectRatio;
            }
        }
    }
}

Vector canvasToViewport(float x, float y) {
    return { x * (vpWidth / canvas.x), y * (vpHeight / canvas.y), d };
}
std::pair<float, float> viewportToCanvas(float x, float y) {
    return { x * (canvas.x / vpWidth), y * (canvas.y / vpHeight) };
}
Vector projectVertex(const Vector &v) {
    // Perspective Projection
    std::pair<float, float> result = viewportToCanvas(((v.x * d) / v.z), ((v.y * d) / v.z));
    return { result.first, result.second, d };
}
void interpolate(float x0, float y0, float x1, float y1, std::vector<float> &arr) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    float aspectratio = (dy != 0) ? (dx / dy) : 0.00001;
    float x = x0;

    size_t size = abs(int(y1) - int(y0));
    arr.reserve(size);
    for (int y = int(y0); y < int(y1); y++) {
        arr.push_back(x);
        x += aspectratio;
    }
}
void interpolate(Vector x0, float y0, Vector x1, float y1, std::vector<Vector> &arr) {
    size_t size = abs(int(y1) - int(y0));
    arr.reserve(size);
    for (int y = int(y0); y < int(y1); y++) {
        float t = float(y - int(y0)) / float(size);
        Vector x = lerp(x0, x1, t);
        arr.push_back(x);
    }
}
void drawVerticesTriangle(const Vector p[3], const Vector n[3], const Material &material, bool wireframe) {
    Vector projected[3] = {
        projectVertex(p[0]),
        projectVertex(p[1]),
        projectVertex(p[2]),
    };
    Vector norm[3] = {
        n[0],
        n[1],
        n[2]
    };

    projected[0].z = p[0].z;
    projected[1].z = p[1].z;
    projected[2].z = p[2].z;

    if (projected[0].y > projected[1].y) {
        std::swap(projected[0], projected[1]);
        std::swap(norm[0], norm[1]);
    }
    if (projected[0].y > projected[2].y) {
        std::swap(projected[0], projected[2]);
        std::swap(norm[0], norm[2]);
    }
    if (projected[1].y > projected[2].y) {
        std::swap(projected[1], projected[2]);
        std::swap(norm[1], norm[2]);
    }

    if (wireframe) {
        drawLine(projected[0], projected[1], material.color);
        drawLine(projected[1], projected[2], material.color);
        drawLine(projected[0], projected[2], material.color);
        return;
    }

    float z0 = (1.f / projected[0].z), z1 = (1.f / projected[1].z), z2 = (1.f / projected[2].z);

    size_t size01 = uint32_t(projected[1].y - projected[0].y);
    size_t size12 = uint32_t(projected[2].y - projected[1].y);
    size_t size02 = uint32_t(projected[2].y - projected[0].y);

    std::vector<float> x01 = {};
    std::vector<float> x12 = {};
    std::vector<float> x02 = {};
    // Reserve space for x01 + x12 because we concatenate later
    x01.reserve(size01 + size12);
    x12.reserve(size12);
    x02.reserve(size02);

    std::vector<float> z01 = {};
    std::vector<float> z12 = {};
    std::vector<float> z02 = {};

    std::vector<Vector> n01 = {};
    std::vector<Vector> n12 = {};
    std::vector<Vector> n02 = {};
    // Reserve space for z01 + z12 because we concatenate later
    z01.reserve(size01 + size12);
    z12.reserve(size12);
    z02.reserve(size02);

    n01.reserve(size01 + size12);
    n12.reserve(size12);
    n02.reserve(size02);

    interpolate(projected[0].x, projected[0].y, projected[1].x, projected[1].y, x01);
    interpolate(projected[1].x, projected[1].y, projected[2].x, projected[2].y, x12);
    interpolate(projected[0].x, projected[0].y, projected[2].x, projected[2].y, x02);

    interpolate(z0, projected[0].y, z1, projected[1].y, z01);
    interpolate(z1, projected[1].y, z2, projected[2].y, z12);
    interpolate(z0, projected[0].y, z2, projected[2].y, z02);

    interpolate(norm[0], projected[0].y, norm[1], projected[1].y, n01);
    interpolate(norm[1], projected[1].y, norm[2], projected[2].y, n12);
    interpolate(norm[0], projected[0].y, norm[2], projected[2].y, n02);

    // Concatenate short sides
    for (const float &x : x12) {
        x01.push_back(x);
    }

    for (const float &z : z12) {
        z01.push_back(z);
    }

    for (const Vector &nrm : n12) {
        n01.push_back(nrm);
    }

    uint32_t middle = uint32_t(x02.size() / 2.f);
    std::vector<float> *xleft = nullptr;
    std::vector<float> *xright = nullptr;
    std::vector<float> *zleft = nullptr;
    std::vector<float> *zright = nullptr;
    std::vector<Vector> *nleft = nullptr;
    std::vector<Vector> *nright = nullptr;

    if ((!x02.size())) {
        return;
    }
    // Find left and right
    if (x02[middle] < x01[middle]) {
        xleft = &x02;
        xright = &x01;

        zleft = &z02;
        zright = &z01;

        nleft = &n02;
        nright = &n01;
    } else {
        xleft = &x01;
        xright = &x02;

        zleft = &z01;
        zright = &z02;

        nleft = &n01;
        nright = &n02;
    }

    bool rtShadows = sceneSettings.lightingMode == LightingMode::LIGHT_SHADOWS;
    bool noLight = sceneSettings.lightingMode == LightingMode::NO_LIGHT;
    for (int y = int(projected[0].y); y < int(projected[2].y); y++) {
        uint32_t scanline = uint32_t(y - int(projected[0].y));
        float lz = (*zleft)[scanline];
        float rz = (*zright)[scanline];
        int lx = (*xleft)[scanline];
        int rx = (*xright)[scanline];
        Vector ln = (*nleft)[scanline];
        Vector rn = (*nright)[scanline];

        // interpolate z
        std::vector<float> zsegment = {};
        std::vector<Vector> nsegment = {};
        uint32_t zsegmentSize = uint32_t(rx - lx) > renderState.width ? renderState.width : (rx - lx);

        zsegment.reserve(zsegmentSize);
        interpolate(lz, float(lx), rz, float(rx), zsegment);
        interpolate(ln, float(lx), rn, float(rx), nsegment);
        for (int x = lx; x < rx; x++) {
            if ((!isIn(float(x), -canvas.x / 2.f, canvas.x / 2.f) || !isIn(float(y), -canvas.y / 2.f, canvas.y / 2.f))) {
                continue;
            }
            int screenx = x + (renderState.width / 2);
            int screeny = (renderState.height / 2) - y;
            uint32_t index = (screeny * renderState.width) + screenx;
            std::lock_guard<std::mutex> lock(pixelLocks[index]);
            float dep = ((float *)depthBuffer)[index];
            float invz = zsegment[x - lx];
            Vector normal = nsegment[x - lx];
            normalize(normal);
            float z = 1.f / invz;
            if (invz <= dep) {
                continue;
            }

            Vector point = canvasToViewport(x * z / d, y * z / d);
            point.z = z;
            point = transformVertex(point, camera, RotateOrder::RO_XYZ);
            Vector direction = camera.position - point;
            direction = direction / length(direction);

            // Colour color = { uint8_t(normal.x * 255), uint8_t(normal.y * 255), uint8_t(normal.z * 255)  };
            Colour color = material.color;
            color = color * ((noLight) ? 1.f : computeLight(point, normal, direction, material.specular, rtShadows));
            ((float *)depthBuffer)[index] = invz;
            ((uint32_t *)renderState.memory)[index] = rgbtoHex(color);
        }
    }
}
void drawBox(const Box &box, const Transform &tf, bool inTriangle) {
    // The tf transform is inverse camera tranform to convert world space box into
    // camera space box
    const Colour red = { 255, 0, 0 };
    // Points of a box
    Vector p[8] = {
        // Front Points
        { box.lowest.x, box.lowest.y, box.lowest.z },
        { box.lowest.x, box.highest.y, box.lowest.z },
        { box.highest.x, box.highest.y, box.lowest.z },
        { box.highest.x, box.lowest.y, box.lowest.z },
        // Back Points
        { box.lowest.x, box.lowest.y, box.highest.z },
        { box.lowest.x, box.highest.y, box.highest.z },
        { box.highest.x, box.highest.y, box.highest.z },
        { box.highest.x, box.lowest.y, box.highest.z }
    };
    int psize = 8;
    Vector projected[8];
    for (int i = 0; i < psize; i++) {
        p[i] = transformVertex(p[i], tf);
        projected[i] = projectVertex(p[i]);
    }
    if (inTriangle) {
        Vector tris[12][3] = {
            // Front 0,1,2,3
            { p[3], p[2], p[1] },
            { p[3], p[1], p[0] },
            // back 4,5,6,7
            { p[6], p[5], p[4] },
            { p[7], p[6], p[4] },
            // left 4,5,1,0
            { p[0], p[1], p[5] },
            { p[0], p[5], p[4] },
            // right 3,2,6,5
            { p[7], p[6], p[2] },
            { p[7], p[2], p[3] },
            // top 1,5,6,2
            { p[2], p[6], p[5] },
            { p[2], p[5], p[1] },
            // Bottom 4,0,3,7
            { p[7], p[3], p[0] },
            { p[7], p[0], p[4] },

        };
        std::vector<Triangle> boxTriangles;
        std::vector<Vector> inTris = {};
        inTris.reserve(12 * 3);
        for (int i = 0; i < 12; i++) {
            Vector normal = cross((tris[i][1] - tris[i][0]), (tris[i][2] - tris[i][0]));
            std::vector<Triangle> tri{
                {
                    tris[i][0],
                    tris[i][1],
                    tris[i][2],
                },
                { normal,
                  normal,
                  normal }
            };
            std::vector<Triangle> clippedTris = {};
            clipTriangle(tri, clippedTris);
            for (const Triangle &triangle : clippedTris) {
                boxTriangles.push_back(triangle);
            }
        }
        drawVertices(boxTriangles, Material{ -1, 0.f,red }, true, false);
        return;
    }
    // Front lines
    drawLine(projected[0], projected[1], red);
    drawLine(projected[1], projected[2], red);
    drawLine(projected[2], projected[3], red);
    drawLine(projected[3], projected[0], red);
    // Back lines
    drawLine(projected[4], projected[5], red);
    drawLine(projected[5], projected[6], red);
    drawLine(projected[6], projected[7], red);
    drawLine(projected[7], projected[4], red);
    // Side lines
    drawLine(projected[0], projected[4], red);
    drawLine(projected[1], projected[5], red);
    drawLine(projected[2], projected[6], red);
    drawLine(projected[3], projected[7], red);
}

float intersectRaySphere(const Vector &O, const Vector &D, const Sphere &sphere) {
    const float r = sphere.radius;
    const Vector CO = O - sphere.center;

    float a = dot(D, D);
    float b = 2 * dot(CO, D);
    float c = dot(CO, CO) - (r * r);

    float discriminant = (b * b) - (4 * a * c);

    if (discriminant < 0) {
        return INT_MAX;
    }
    const float t = (-b - sqrt(discriminant)) / (2 * a);
    return t;
}
float intersectRayTriangle(const Vector &O, const Vector &D, Triangle &triangle) {
    float t = 0;
    Vector N = triangle.normals[0];
    float NdotRay = dot(N, D);
    if (NdotRay > 0)
        return INT_MAX;
    float d = -dot(N, triangle.points[0]);
    t = -(dot(N, O) + d) / NdotRay;
    if (t < 0)
        return INT_MAX;

    Vector P = O + (t * D);

    Vector C;

    // edge 0
    Vector edge = triangle.points[1] - triangle.points[0];
    Vector pLine = P - triangle.points[0];
    C = cross(edge, pLine);

    if (dot(N, C) < 0)
        return INT_MAX; // Point is outside/Rightside edge 0;

    // edge 1
    edge = triangle.points[2] - triangle.points[1];
    pLine = P - triangle.points[1];
    C = cross(edge, pLine);

    if (dot(N, C) < 0)
        return INT_MAX; // Point is outside/Rightside edge 1;

    // edge 2
    edge = triangle.points[0] - triangle.points[2];
    pLine = P - triangle.points[2];
    C = cross(edge, pLine);

    if (dot(N, C) < 0)
        return INT_MAX; // Point is outside/Rightside edge 2;

    return t;
}
bool RayIntersectsBox(const Vector &O, const Vector &D, const Box &box) {
    Vector invDir = 1.0f / D;
    float tmin, tmax, tymin, tymax, tzmin, tzmax;
    if (invDir.x >= 0) {
        tmin = (box.lowest.x - O.x) * invDir.x;
        tmax = (box.highest.x - O.x) * invDir.x;
    } else {
        tmin = (box.highest.x - O.x) * invDir.x;
        tmax = (box.lowest.x - O.x) * invDir.x;
    }

    if (invDir.y >= 0) {
        tymin = (box.lowest.y - O.y) * invDir.y;
        tymax = (box.highest.y - O.y) * invDir.y;
    } else {
        tymin = (box.highest.y - O.y) * invDir.y;
        tymax = (box.lowest.y - O.y) * invDir.y;
    }

    if ((tmin > tymax) || (tymin > tmax)) {
        return false;
    }

    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    if (invDir.z >= 0) {
        tzmin = (box.lowest.z - O.z) * invDir.z;
        tzmax = (box.highest.z - O.z) * invDir.z;
    } else {
        tzmin = (box.highest.z - O.z) * invDir.z;
        tzmax = (box.lowest.z - O.z) * invDir.z;
    }
    if ((tmin > tzmax) || (tmax < tzmin)) {
        return false;
    }
    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;
    if (tmin < 0) {
        return false;
    }
    return true;
}
HitData closestIntersection(const Vector &O, const Vector &D, float tMin, float tMax) {
    HitData hitData = {};
    hitData.intersection = INT_MAX;
    // Sphere intersection
    for (Sphere &sphere : scene.spheres) {
        float sphereInt = intersectRaySphere(O, D, sphere);
        if (isIn(sphereInt, tMin, tMax) && (sphereInt < hitData.intersection)) {
            hitData.intersection = sphereInt;
            Vector P = O + (D * sphereInt);
            hitData.normal = P - sphere.center;
            Material material;
            material = { sphere.specular, sphere.reflectiveness, sphere.color };
            hitData.material = material;
        }
    }
    // Triangle
    for (Triangle &triangle : scene.triangles) {
        float triangleInt = intersectRayTriangle(O, D, triangle);
        if (isIn(triangleInt, tMin, tMax) && triangleInt < hitData.intersection) {
            hitData.intersection = triangleInt;
            hitData.normal = triangle.normals[0];
            hitData.material = Material{ -1, 0.f, {255,0,0} };
        }
    }
    // Mesh
    for (const Instance &instance : scene.instances) {
        std::vector<Triangle> &triangles = instance.mesh->triangleData;
        Box mbb = instance.boundingBox;
        if (sceneSettings.debugState == DebugState::DS_BOUNDING_BOX) {
            if (!RayIntersectsBox(O, D, mbb)) {
                continue;
            } else {
                hitData = {};
                hitData.material.color = { 255, 0, 0 };
                hitData.intersection = 0.f;
                return hitData;
            }
        } else if (!RayIntersectsBox(O, D, mbb)) {
            continue;
        }
        for (size_t i = 0; i < triangles.size(); i++) {
            Vector p[3] = {
                transformVertex(triangles[i].points[0], instance.transform),
                transformVertex(triangles[i].points[1], instance.transform),
                transformVertex(triangles[i].points[2], instance.transform),
            };
            Triangle tri;
            tri.points[0] = p[0];
            tri.points[1] = p[1];
            tri.points[2] = p[2];

            float triangleInt = intersectRayTriangle(O, D, tri);
            if (isIn(triangleInt, tMin, tMax) && triangleInt < hitData.intersection) {
                hitData.intersection = triangleInt;
                hitData.material = instance.mesh->material;
                hitData.normal = tri.normals[0];
                break;
            }
        }
    }
    return hitData;
}
Vector reflectRay(const Vector &R, const Vector &N) {
    return (2 * (N * dot(R, N)) - R);
}
float computeLight(const Vector &P, const Vector &N, const Vector V, float s, bool rtShadows) {
    float i = 0.f;
    for (const Light &light : scene.lights) {
        // L = direction of the light
        Vector L = {};
        float distance = 0;
        float radius = light.intensity * 256;
        if (light.type == LT_AMBIENT) {
            i += light.intensity;
        } else {
            if (light.type == LT_DIRECTIONAL) {
                L = -light.direction;
                distance = INT_MAX;
            } else if (light.type == LT_POINT) {
                L = (light.pos - P);
                distance = length(L);
            }
            float shadowT = rtShadows ? closestIntersection(P, L, 0.000001, distance).intersection : INT_MAX;
            if (shadowT > 1) {
                shadowT = INT_MAX;
            }
            // cast a shadow if an object intersected between light and point
            if (shadowT != INT_MAX) {
                continue;
            }
            // Diffuse reflection
            float nDotL = dot(N, L);
            float diffuse = 0;
            if (nDotL > 0) {
                diffuse = nDotL / (length(N) * length(L));
            }
            // specular reflection
            float specular = 0;
            if (s != -1 && light.type == LT_POINT) {
#if 0
                //Phong
                Vector R = reflectRay(L, N);
                float vDotR = dot(V, R);
                if (vDotR > 0) {
                    i += light.intensity * pow((vDotR / (length(R) * length(V))), s);
                }
#else

                // Blinn - Phong
                Vector Nn = normalize(N);
                Vector Ln = normalize(L);
                Vector Vn = normalize(V);
                Vector H = normalize(Ln + Vn);
                specular = pow(getMax(dot(Nn, H), 0.f), s);
#endif
            }
            float attinuation = (light.type == LT_POINT) ? clampv(1 - distance / radius, 0.f, 1.f) : 1.f;
            i += (specular + diffuse) * light.intensity * attinuation;
        }
    }
    return i;
}
// returns a signed distance from the point to the plane
float planeIntersection(const Plane &plane, const Vector &point) {
    return ((dot(point, plane.normal)) + plane.offset);
}
float edgePlaneIntersection(const Plane &plane, const Vector &A, const Vector &B) {
    // P = A + t(B - A)
    // <N,P> + D = 0
    // <N,(A + t(B - A))> + D = 0
    // <N,A> + t(<N,B - A>) + D = 0
    // <N,A> + t(<N,B - A>) = -D
    // <N,A> + t = -D / <N,(B - A)>
    // t = -D - <N,A>/ <N,(B - A)>
    return (-plane.offset - dot(plane.normal, A)) / dot(plane.normal, (B - A));
}
void clipPlane(const Plane &plane, const std::vector<Triangle> &in, std::vector<Triangle> &out) {
    for (size_t vc = 0; vc < in.size(); vc++) {
        Vector tp[3] = {
            in[vc].points[0],
            in[vc].points[1],
            in[vc].points[2],
        };
        Vector tn[3] = {
            in[vc].normals[0],
            in[vc].normals[1],
            in[vc].normals[2],
        };
        float d1 = planeIntersection(plane, tp[0]);
        float d2 = planeIntersection(plane, tp[1]);
        float d3 = planeIntersection(plane, tp[2]);
        // All points of a triangle are inside the plane
        if ((d1 >= 0.f) && (d2 >= 0.f) && (d3 >= 0.f)) {
            out.emplace_back(Triangle{ { tp[0], tp[1], tp[2] }, { tn[0], tn[1], tn[2] } });
            continue;
        }
        // All points of a triangle are outside the plane
        else if ((d1 < 0.f) && (d2 < 0.f) && (d3 < 0.f)) {
            continue;
        }

        int inCount = 0;
        bool isin[3] = { false, false, false };
        if (d1 > 0.f) {
            inCount++;
            isin[0] = true;
        }
        if (d2 > 0.f) {
            inCount++;
            isin[1] = true;
        }
        if (d3 > 0.f) {
            inCount++;
            isin[2] = true;
        }
        int invec[2] = { -1, -1 };
        int outvec[2] = { -1, -1 };
        int j = 0, k = 0;
        for (int v = 0; v < 3; v++) {
            if (isin[v]) {
                invec[j] = v;
                j++;
            } else {
                outvec[k] = v;
                k++;
            }
        }

        if (inCount == 1) {
            Vector A, B, C;
            A = tp[invec[0]];
            B = tp[outvec[0]];
            C = tp[outvec[1]];

            Vector AN, BN, CN;
            AN = tn[invec[0]];
            BN = tn[outvec[0]];
            CN = tn[outvec[1]];

            float edgeIntAB = edgePlaneIntersection(plane, A, B);
            float edgeIntAC = edgePlaneIntersection(plane, A, C);

            B = A + (edgeIntAB * (B - A));
            C = A + (edgeIntAC * (C - A));

            BN = AN + (edgeIntAB * (BN - AN));
            CN = AN + (edgeIntAC * (CN - AN));

            Vector p[3];
            p[invec[0]] = A;
            p[outvec[0]] = B;
            p[outvec[1]] = C;

            Vector n[3];
            n[invec[0]] = AN;
            n[outvec[0]] = BN;
            n[outvec[1]] = CN;

            out.emplace_back(Triangle{ { p[0], p[1], p[2] }, { n[0], n[1], n[2] } });
        } else if (inCount == 2) {
            Vector A, B, C;
            A = tp[invec[0]];
            B = tp[invec[1]];
            C = tp[outvec[0]];

            Vector AN, BN, CN;
            AN = tn[invec[0]];
            BN = tn[invec[1]];
            CN = tn[outvec[0]];

            float edgeIntAC = edgePlaneIntersection(plane, A, C);
            float edgeIntBC = edgePlaneIntersection(plane, B, C);

            Vector newB;
            newB = B + (edgeIntBC * (C - B));
            Vector newBN;
            newBN = BN + (edgeIntBC * (CN - BN));
            C = A + (edgeIntAC * (C - A));
            CN = AN + (edgeIntAC * (CN - AN));
            Vector p1[3];
            p1[invec[0]] = A;
            p1[invec[1]] = B;
            p1[outvec[0]] = newB;

            Vector n1[3];
            n1[invec[0]] = AN;
            n1[invec[1]] = BN;
            n1[outvec[0]] = newBN;

            Vector p2[3];
            p2[invec[0]] = newB;
            p2[invec[1]] = C;
            p2[outvec[0]] = A;

            Vector n2[3];
            n2[invec[0]] = newBN;
            n2[invec[1]] = CN;
            n2[outvec[0]] = AN;

            out.emplace_back(Triangle{ { p1[0], p1[1], p1[2] }, { n1[0], n1[1], n1[2] } });
            out.emplace_back(Triangle{ { p2[0], p2[1], p2[2] }, { n2[0], n2[1], n2[2] } });
        }
    }
}
void clipTriangle(const std::vector<Triangle> &in, std::vector<Triangle> &out) {
    out.reserve(in.size());
    std::vector<Triangle> clippedBuffer;
    clippedBuffer = in;
    for (int i = 0; i < 6; i++) {
        if (i % 2 == 0) {
            out.clear();
            clipPlane(planes[i], clippedBuffer, out);
        } else {
            clippedBuffer.clear();
            clipPlane(planes[i], out, clippedBuffer);
        }
    }
    out = clippedBuffer;
}
void modelSpaceToDrawable(const Vector p[3], const Vector n[3], const Transform &transform, std::vector<Triangle> &outData) {
    Vector vert[3];
    Vector norm[3];
    norm[0] = n[0];
    norm[1] = n[1];
    norm[2] = n[2];
    // Model space to world space
    vert[0] = transformVertex(p[0], transform);
    vert[1] = transformVertex(p[1], transform);
    vert[2] = transformVertex(p[2], transform);

    norm[0] = rotate(norm[0], transform.rotation);
    norm[1] = rotate(norm[1], transform.rotation);
    norm[2] = rotate(norm[2], transform.rotation);

    // World space to camera space
    vert[0] = vert[0] - camera.position;
    vert[1] = vert[1] - camera.position;
    vert[2] = vert[2] - camera.position;

    vert[0] = rotate(vert[0], -camera.rotation);
    vert[1] = rotate(vert[1], -camera.rotation);
    vert[2] = rotate(vert[2], -camera.rotation);


    const bool backFaceCulling = sceneSettings.bfc;
    Vector normal = cross(vert[1] - vert[0], vert[2] - vert[0]);
    Vector PO = -vert[0];

    // Backface culling
    if (!(dot(normal, PO) > 0.f) && backFaceCulling) {
        return;
    }

    // Frustum culling
    const std::vector<Triangle> triData = { Triangle{ vert[0], vert[1], vert[2], norm[0], norm[1], norm[2] } };
    std::vector<Triangle> clippedData;
    clipTriangle(triData, clippedData);

    outData.reserve(outData.size() + clippedData.size());
    for (const Triangle &tri : clippedData) {
        outData.push_back(tri);
    }
}
void modelSpaceToDrawableThr(const std::vector<Triangle> &triangleData, const Transform &transform, std::vector<Triangle> &outData, uint32_t start, uint32_t end) {
    outData.reserve(outData.size() + (end - start));
    for (uint32_t i = start; i < end; i++) {

        Vector triangle[3] = {
            triangleData[i].points[0],
            triangleData[i].points[1],
            triangleData[i].points[2]
        };
        Vector normals[3] = {
            triangleData[i].normals[0],
            triangleData[i].normals[1],
            triangleData[i].normals[2]
        };
        modelSpaceToDrawable(triangle, normals, transform, outData);
    }
}

void getDrawableTriangles(const std::vector<Triangle> &triangleData, const Transform &transform, std::vector<Triangle> &outData, bool multithread) {
    if (!multithread) {
        for (size_t i = 0; i < triangleData.size(); i++) {
            Vector triangle[3] = {
                triangleData[i].points[0],
                triangleData[i].points[1],
                triangleData[i].points[2]
            };
            Vector normals[3] = {
                triangleData[i].normals[0],
                triangleData[i].normals[1],
                triangleData[i].normals[2]
            };
            modelSpaceToDrawable(triangle, normals, transform, outData);
        }
    } else {
        const uint32_t threadSize = std::thread::hardware_concurrency();
        uint32_t triSize = triangleData.size();
        uint32_t triPerThread = triSize / threadSize;
        uint32_t remainingTris = triSize % threadSize;
        std::vector<std::thread> triProcessThr(threadSize);
        std::vector<std::vector<Triangle>> outTrisArr(threadSize);
        uint32_t start = 0;
        for (uint32_t i = 0; i < threadSize; i++) {
            uint32_t end = start + triPerThread + ((i < remainingTris) ? 1 : 0);
            triProcessThr[i] = std::thread(Renderer::modelSpaceToDrawableThr, std::cref(triangleData), std::cref(transform), std::ref(outTrisArr[i]), start, end);
            start = end;
        }
        for (uint32_t i = 0; i < threadSize; i++) {
            triProcessThr[i].join();
        }
        size_t triangleVectorSize=0;
        for (const std::vector<Triangle> &tris : outTrisArr) {
            triangleVectorSize += tris.size();
        }

        outData.reserve(triangleVectorSize);
        for (const std::vector<Triangle> &tris : outTrisArr) {
            for (const Triangle &tri : tris) {
                outData.push_back(tri);
            }
        }
    }
}
void drawVerticesThr(const std::vector<Triangle> &triangleData, const Material &material, bool wireframe, uint32_t start, uint32_t end) {
    for (uint32_t i = start; i < end; i++) {
        Vector p[3] = {
            triangleData[i].points[0],
            triangleData[i].points[1],
            triangleData[i].points[2],
        };
        Vector n[3] = {
            triangleData[i].normals[0],
            triangleData[i].normals[1],
            triangleData[i].normals[2],
        };
        drawVerticesTriangle(p, n, material, wireframe);
    }
}
void drawVertices(const std::vector<Triangle> &triangleData, const Material &material, bool wireframe, bool multithread) {
    if (!multithread) {
        for (size_t i = 0; i < triangleData.size(); i++) {
            Vector p[3] = {
                triangleData[i].points[0],
                triangleData[i].points[1],
                triangleData[i].points[2]
            };
            Vector n[3] = {
                triangleData[i].normals[0],
                triangleData[i].normals[1],
                triangleData[i].normals[2],
            };
            drawVerticesTriangle(p, n, material, wireframe);
        }
    } else {
        const uint32_t threadSize = std::thread::hardware_concurrency();
        uint32_t triSize = triangleData.size();
        uint32_t triPerThread = triSize / threadSize;
        uint32_t remainingTris = triSize % threadSize;
        std::vector<std::thread> drawVerticesThr(threadSize);
        uint32_t start = 0;
        for (uint32_t i = 0; i < threadSize; i++) {
            uint32_t end = start + triPerThread + ((i < remainingTris) ? 1 : 0);
            drawVerticesThr[i] = std::thread(Renderer::drawVerticesThr, std::cref(triangleData), std::cref(material), wireframe, start, end);
            start = end;
        }
        for (uint32_t i = 0; i < threadSize; i++) {
            drawVerticesThr[i].join();
        }
    }
}
void renderMesh(const Mesh &mesh, const Transform &transform, bool multithread) {
    std::vector<Triangle> triData = {};
    getDrawableTriangles(mesh.triangleData, transform, triData, multithread);

    sceneSettings.triSeenCount += triData.size();
    bool drawWireframe = (sceneSettings.debugState == DebugState::DS_TRIANGLE);
    drawVertices(triData, mesh.material, drawWireframe, multithread);
}
Colour traceRay(const Vector &O, const Vector &D, float tMin, float tMax, int recursionLimit) {
    HitData hitData = closestIntersection(O, D, tMin, tMax);
    float closestT = hitData.intersection;
    Colour bgColor = { 100, 100, 100 };
    if (closestT == INT_MAX) {
        return bgColor;
    }

    // P = point of the intersection
    Vector P = O + (D * closestT);
    // N = normal at the point
    Vector N = hitData.normal;
    // Normalizing the normal
    N = N / length(N);
    float light = computeLight(P, N, -D, hitData.material.specular);
    Colour localColor = (hitData.material.color * light);
    float r = hitData.material.reflectiveness;
    if (recursionLimit <= 0 || r <= 0.f) {
        return localColor;
    }

    Vector R = reflectRay(-D, N);
    Colour reflectedColor = traceRay(P, R, 0.001, INT_MAX, recursionLimit - 1);

    return (localColor * (1.f - r)) + (reflectedColor * r);
}
void rayTraceThr(const int threadNum, const int threadCount) {
    float ycount = (canvas.y / threadCount);
    float ymin = ycount * threadNum;
    float ymax = ymin + ycount;
    for (float y = ymin; y < ymax; y++) {
        for (float x = 0; x < renderState.width; x++) {
            Vector direction = canvasToViewport(x - (canvas.x / 2.f), (canvas.y / 2.f) - y);
            direction = rotate(direction, camera.rotation, RotateOrder::RO_XYZ);
            direction = direction / length(D);
            Colour result = traceRay(camera.position, direction, 1, INT_MAX, 3);
            putPixelD(x, y, result);
        }
    }
}
void rayTrace() {
    clearScreen(0x000000);
    for (float y = 0; y < renderState.height; y++) {
        int scanlineDone = y + 1;
        LOG_INFO("\rScanlines Done:" << scanlineDone << '/' << (renderState.width) << ':' << int((scanlineDone / (renderState.width)) * 100) << "%" << std::flush);
        for (float x = 0; x < renderState.width; x++) {
            D = canvasToViewport(x - (canvas.x / 2), (canvas.y / 2) - y);
            D = rotate(D, camera.rotation, RotateOrder::RO_XYZ);
            D = D / length(D);
            Colour result = traceRay(camera.position, D, 1, INT_MAX, 3);
            putPixelD(x, y, result);
        }
    }
}
void renderScene() {
    clearScreen(0x646464);
    static std::unordered_map<Sphere, Mesh> sphereMeshCache = {};
    sceneSettings.triSeenCount = 0;
    // Render meshes
    for (const Instance &ins : scene.instances) {
        renderMesh(*ins.mesh, ins.transform);
    }
    // Render spheres
    for (const Sphere &sphere : scene.spheres) {
        Mesh sphereM = {};
        // cache spheres if not already
        if (sphereMeshCache.find(sphere) == sphereMeshCache.end()) {
            sphereM = loadOBJ("res/Models/Sphere.obj", sphere.color, sphere.reflectiveness, sphere.specular);
            sphereMeshCache[sphere] = sphereM;
        } else {
            sphereM = sphereMeshCache[sphere];
        }
        Vector offset = Vector{ 0, -0.2f, 0 };
        float scale = 0.4f;
        Transform transform = { (sphere.center + offset), sphere.radius * scale };
        Instance sphereIns{ &sphereM, transform };
        renderMesh(*sphereIns.mesh, sphereIns.transform);
    }
    // Render scene triangles
    bool isWireframe = (sceneSettings.debugState == DebugState::DS_TRIANGLE);
    for (const Triangle &striangle : scene.triangles) {
        std::vector<Triangle> drawableTris = {};
        Vector points[3] = {
            striangle.points[0],
            striangle.points[1],
            striangle.points[2],
        };
        Vector normals[3] = {
            striangle.normals[0],
            striangle.normals[1],
            striangle.normals[2],
        };
        modelSpaceToDrawable(points, normals, { { 0, 0, 0 }, 1.f, { 0, 0, 0 } }, drawableTris);

        drawVertices(drawableTris, {}, isWireframe, false);
    }
    // Apply AA
    if (sceneSettings.antiAliasing && (sceneSettings.debugState != DebugState::DS_TRIANGLE)) {
        PostProcess::FXAA();
    }
    if (sceneSettings.debugState == DebugState::DS_BOUNDING_BOX) {
        // Draw Bounding boxes
        for (Instance &ins : scene.instances) {
            Box box = ins.getBoundingBox();
            box.highest = box.highest - camera.position;
            box.lowest = box.lowest - camera.position;
            Transform ttf = { { 0, 0, 0 }, 1, -camera.rotation };
            drawBox(box, ttf);
        }
    }
}
}; // namespace Renderer
