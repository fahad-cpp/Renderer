#include "Renderer.h"
#include "Colour.h"
#include "Globals.h"
#include "Hash.h"
#include "Logging.h"
#include "Object.h"
#include "PostProcess.h"
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
    arr.resize(size);
    int idx = 0;
    for (int y = int(y0); y < int(y1); y++) {
        arr[idx] = x;
        idx++;
        x += aspectratio;
    }
}
void drawVerticesTriangle(const Vector p[3], const Material &material, bool wireframe) {
    Vector projected[3] = {
        projectVertex(p[0]),
        projectVertex(p[1]),
        projectVertex(p[2]),
    };

    projected[0].z = p[0].z;
    projected[1].z = p[1].z;
    projected[2].z = p[2].z;

    if (projected[0].y > projected[1].y) {
        std::swap(projected[0], projected[1]);
    }
    if (projected[0].y > projected[2].y) {
        std::swap(projected[0], projected[2]);
    }
    if (projected[1].y > projected[2].y) {
        std::swap(projected[1], projected[2]);
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
    // Reserve space for z01 + z12 because we concatenate later
    z01.reserve(size01 + size12);
    z12.reserve(size12);
    z02.reserve(size02);

    interpolate(projected[0].x, projected[0].y, projected[1].x, projected[1].y, x01);
    interpolate(projected[1].x, projected[1].y, projected[2].x, projected[2].y, x12);
    interpolate(projected[0].x, projected[0].y, projected[2].x, projected[2].y, x02);

    interpolate(z0, projected[0].y, z1, projected[1].y, z01);
    interpolate(z1, projected[1].y, z2, projected[2].y, z12);
    interpolate(z0, projected[0].y, z2, projected[2].y, z02);

    // Concatenate short sides
    for (const float &x : x12) {
        x01.push_back(x);
    }

    for (const float &z : z12) {
        z01.push_back(z);
    }

    uint32_t middle = uint32_t(x02.size() / 2.f);
    std::vector<float> *xleft = nullptr;
    std::vector<float> *xright = nullptr;
    std::vector<float> *zleft = nullptr;
    std::vector<float> *zright = nullptr;

    if ((!x02.size())) {
        return;
    }
    // Find left and right
    if (x02[middle] < x01[middle]) {
        xleft = &x02;
        xright = &x01;

        zleft = &z02;
        zright = &z01;
    } else {
        xleft = &x01;
        xright = &x02;

        zleft = &z01;
        zright = &z02;
    }

    Vector worldSpace[3] = {
        transformVertex(p[0], camera, RotateOrder::RO_XYZ),
        transformVertex(p[1], camera, RotateOrder::RO_XYZ),
        transformVertex(p[2], camera, RotateOrder::RO_XYZ)
    };

    Vector normal = cross((worldSpace[1] - worldSpace[0]), (worldSpace[2] - worldSpace[0]));
    normal = normal / length(normal);

    bool rtShadows = sceneSettings.lightingMode == LightingMode::LIGHT_SHADOWS;
    bool noLight = sceneSettings.lightingMode == LightingMode::NO_LIGHT;
    for (int y = int(projected[0].y); y < int(projected[2].y); y++) {
        uint32_t scanline = uint32_t(y - int(projected[0].y));
        float lz = (*zleft)[scanline];
        float rz = (*zright)[scanline];
        int lx = (*xleft)[scanline];
        int rx = (*xright)[scanline];

        // interpolate z
        std::vector<float> zsegment = {};
        uint32_t zsegmentSize = uint32_t(rx - lx) > renderState.width ? renderState.width : (rx - lx);
        zsegment.reserve(zsegmentSize);
        interpolate(lz, float(lx), rz, float(rx), zsegment);

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
            float z = 1.f / invz;
            if (invz <= dep) {
                continue;
            }

            Vector point = canvasToViewport(x * z / d, y * z / d);
            point.z = z;
            point = transformVertex(point, camera, RotateOrder::RO_XYZ);
            Vector direction = point - camera.position;
            direction = direction / length(direction);

            Colour color = material.color * ((noLight) ? 1.f : computeLight(point, normal, direction, material.specular, rtShadows));
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
        std::vector<Vector> triangles = {};
        std::vector<Vector> inTris = {};
        inTris.reserve(12 * 3);
        for (int i = 0; i < 12; i++) {
            Triangle tri(tris[i], { 0, 0, 0 }, Material{});
            std::vector<Triangle> clippedTris = clipTriangle(tri);
            for (const Triangle &ct : clippedTris) {
                triangles.push_back(ct.p[0]);
                triangles.push_back(ct.p[1]);
                triangles.push_back(ct.p[2]);
            }
        }
        drawVertices(triangles, Material{ red, -1, 0.f }, true, false);
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
    Vector N = triangle.calculateNormal();
    float NdotRay = dot(N, D);
    if (NdotRay > 0)
        return INT_MAX;
    float d = -dot(N, triangle.p[0]);
    t = -(dot(N, O) + d) / NdotRay;
    if (t < 0)
        return INT_MAX;

    Vector P = O + (t * D);

    Vector C;

    // edge 0
    Vector edge = triangle.p[1] - triangle.p[0];
    Vector pLine = P - triangle.p[0];
    C = cross(edge, pLine);

    if (dot(N, C) < 0)
        return INT_MAX; // Point is outside/Rightside edge 0;

    // edge 1
    edge = triangle.p[2] - triangle.p[1];
    pLine = P - triangle.p[1];
    C = cross(edge, pLine);

    if (dot(N, C) < 0)
        return INT_MAX; // Point is outside/Rightside edge 1;

    // edge 2
    edge = triangle.p[0] - triangle.p[2];
    pLine = P - triangle.p[2];
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
            material = { sphere.color, sphere.specular, sphere.reflectiveness };
            hitData.material = material;
        }
    }
    // Triangle
    for (Triangle &triangle : scene.triangles) {
        float triangleInt = intersectRayTriangle(O, D, triangle);
        if (isIn(triangleInt, tMin, tMax) && triangleInt < hitData.intersection) {
            hitData.intersection = triangleInt;
            hitData.normal = triangle.normal;
            hitData.material = triangle.material;
        }
    }
    // Mesh
    for (const Instance &instance : scene.instances) {
        std::vector<Vector> &triangles = instance.mesh->triangles;
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
        for (size_t i = 0; i < triangles.size(); i += 3) {
            Vector p[3] = {
                transformVertex(triangles[i], instance.transform),
                transformVertex(triangles[i + 1], instance.transform),
                transformVertex(triangles[i + 2], instance.transform),
            };
            Triangle tri(p, { 0, 0, 0 }, Material{});

            float triangleInt = intersectRayTriangle(O, D, tri);
            if (isIn(triangleInt, tMin, tMax) && triangleInt < hitData.intersection) {
                hitData.intersection = triangleInt;
                hitData.material = instance.mesh->material;
                hitData.normal = tri.calculateNormal();
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
        float tMax = 0;
        if (light.type == LT_AMBIENT) {
            i += light.intensity;
        } else {
            if (light.type == LT_DIRECTIONAL) {
                L = { -light.direction.x, -light.direction.y, -light.direction.z };
                tMax = INT_MAX;
            } else if (light.type == LT_POINT) {
                L = (light.pos - P);
                tMax = length(L);
            }
            float shadowT = rtShadows ? closestIntersection(P, L, 0.000001, tMax).intersection : INT_MAX;
            if (shadowT > 1) {
                shadowT = INT_MAX;
            }
            // cast a shadow if an object intersected between light and point
            if (shadowT != INT_MAX) {
                continue;
            }
            // Diffuse reflection
            float nDotL = dot(N, L);
            if (nDotL > 0) {
                i += (light.intensity * nDotL / (length(N) * length(L)));
            }
            // specular reflection
            if (s != -1) {
                Vector R = reflectRay(L, N);
                float vDotR = dot(V, R);
                if (vDotR > 0) {
                    i += light.intensity * pow((vDotR / (length(R) * length(V))), s);
                }
            }
        }
    }
    return i;
}
// returns a signed distance from the point to the plane
inline float planeIntersection(Plane &plane, const Vector &point) {
    return ((dot(point, plane.normal)) + plane.offset);
}
float edgePlaneIntersection(Plane &plane, const Vector &A, const Vector &B) {
    // P = A + t(B - A)
    // <N,P> + D = 0
    // <N,(A + t(B - A))> + D = 0
    // <N,A> + t(<N,B - A>) + D = 0
    // <N,A> + t(<N,B - A>) = -D
    // <N,A> + t = -D / <N,(B - A)>
    // t = -D - <N,A>/ <N,(B - A)>
    return (-plane.offset - dot(plane.normal, A)) / dot(plane.normal, (B - A));
}
std::vector<Triangle> clipTriangle(const Triangle &tri) {
    std::vector<Triangle> triangles = { tri };
    for (int i = 0; i < 6; i++) {
        std::vector<Triangle> planeClipped = {};
        for (const Triangle &t : triangles) {

            float d1 = planeIntersection(planes[i], t.p[0]);
            float d2 = planeIntersection(planes[i], t.p[1]);
            float d3 = planeIntersection(planes[i], t.p[2]);
            // All points of a triangle are inside the plane
            if ((d1 >= 0.f) && (d2 >= 0.f) && (d3 >= 0.f)) {
                planeClipped.push_back(t);
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
            int invec[3] = { -1, -1, -1 };
            int outvec[3] = { -1, -1, -1 };
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
                A = t.p[invec[0]];
                B = t.p[outvec[0]];
                C = t.p[outvec[1]];

                float edgeIntAB = edgePlaneIntersection(planes[i], A, B);
                float edgeIntAC = edgePlaneIntersection(planes[i], A, C);

                B = A + (edgeIntAB * (B - A));
                C = A + (edgeIntAC * (C - A));

                Vector p[3];
                p[invec[0]] = A;
                p[outvec[0]] = B;
                p[outvec[1]] = C;
                Triangle newTri(p, (t.normal / length(t.normal)), t.material);
                planeClipped.push_back(newTri);
            } else if (inCount == 2) {
                Vector A, B, C;
                A = t.p[invec[0]];
                B = t.p[invec[1]];
                C = t.p[outvec[0]];

                float edgeIntAC = edgePlaneIntersection(planes[i], A, C);
                float edgeIntBC = edgePlaneIntersection(planes[i], B, C);

                Vector newB;
                newB = B + (edgeIntBC * (C - B));
                C = A + (edgeIntAC * (C - A));
                Vector p1[3];
                p1[invec[0]] = A;
                p1[invec[1]] = B;
                p1[outvec[0]] = newB;
                Vector p2[3];
                p2[invec[0]] = newB;
                p2[invec[1]] = C;
                p2[outvec[0]] = A;
                Triangle newTri1(p1, (t.normal / length(t.normal)), t.material);
                Triangle newTri2(p2, (t.normal / length(t.normal)), t.material);
                planeClipped.push_back(newTri1);
                planeClipped.push_back(newTri2);
            }
        }
        triangles.clear();
        triangles = planeClipped;
    }
    return { triangles };
}
void modelSpaceToDrawable(const Vector p[3], const Transform &transform, std::vector<Vector> &outTris) {
    Vector moved[3];
    // Model space to world space
    moved[0] = transformVertex(p[0], transform);
    moved[1] = transformVertex(p[1], transform);
    moved[2] = transformVertex(p[2], transform);

    // World space to camera space
    moved[0] = moved[0] - camera.position;
    moved[1] = moved[1] - camera.position;
    moved[2] = moved[2] - camera.position;

    moved[0] = rotate(moved[0], -camera.rotation);
    moved[1] = rotate(moved[1], -camera.rotation);
    moved[2] = rotate(moved[2], -camera.rotation);

    const bool backFaceCulling = sceneSettings.bfc;
    Triangle newTri;
    Vector PO = -moved[0];
    newTri.p[0] = moved[0];
    newTri.p[1] = moved[1];
    newTri.p[2] = moved[2];
    Vector normal = newTri.calculateNormal();
    normal = normal / length(normal);

    // Backface culling
    if (!(dot(normal, PO) > 0.f) && backFaceCulling) {
        return;
    }

    // Frustum culling
    std::vector<Triangle> clippedTris = clipTriangle(newTri);

    // Save the drawable triangles in a vector
    for (const Triangle &tri : clippedTris) {
        outTris.push_back(tri.p[0]);
        outTris.push_back(tri.p[1]);
        outTris.push_back(tri.p[2]);
    }
}
void modelSpaceToDrawableThr(const std::vector<Vector> &inTris, const Transform &transform, std::vector<Vector> &outTris, uint32_t start, uint32_t end) {
    for (uint32_t i = start; i < end; i++) {

        Vector triangle[3] = {
            inTris[i * 3],
            inTris[i * 3 + 1],
            inTris[i * 3 + 2]
        };
        modelSpaceToDrawable(triangle, transform, outTris);
    }
}

void getDrawableTriangles(const std::vector<Vector> &inTris, const Transform &transform, std::vector<Vector> &outTris, bool multithread) {
    if (!multithread) {
        for (size_t i = 0; i < inTris.size(); i += 3) {
            Vector triangle[3] = {
                inTris[i],
                inTris[i + 1],
                inTris[i + 2]
            };
            modelSpaceToDrawable(triangle, transform, outTris);
        }
    } else {
        const uint32_t threadSize = std::thread::hardware_concurrency();
        uint32_t triSize = inTris.size() / 3;
        uint32_t triPerThread = triSize / threadSize;
        uint32_t remainingTris = triSize % threadSize;
        std::vector<std::thread> triProcessThr(threadSize);
        std::vector<std::vector<Vector>> outTrisArr(threadSize);
        uint32_t start = 0;
        for (uint32_t i = 0; i < threadSize; i++) {
            uint32_t end = start + triPerThread + ((i < remainingTris) ? 1 : 0);
            triProcessThr[i] = std::thread(Renderer::modelSpaceToDrawableThr, std::cref(inTris), std::cref(transform), std::ref(outTrisArr[i]), start, end);
            start = end;
        }
        for (uint32_t i = 0; i < threadSize; i++) {
            triProcessThr[i].join();
        }
        for (const std::vector<Vector> &tris : outTrisArr) {
            for (const Vector &point : tris) {
                outTris.push_back(point);
            }
        }
    }
}
void drawVerticesThr(const std::vector<Vector> &vertices, const Material &material, bool wireframe, uint32_t start, uint32_t end) {
    for (uint32_t i = start; i < end; i++) {
        Vector p[3] = {
            vertices[i * 3],
            vertices[i * 3 + 1],
            vertices[i * 3 + 2],
        };
        drawVerticesTriangle(p, material, wireframe);
    }
}
void drawVertices(const std::vector<Vector> &vertices, const Material &material, bool wireframe, bool multithread) {
    if (!multithread) {
        for (size_t i = 0; i < vertices.size(); i += 3) {
            Vector p[3] = {
                vertices[i],
                vertices[i + 1],
                vertices[i + 2]
            };
            drawVerticesTriangle(p, material, wireframe);
        }
    } else {
        const uint32_t threadSize = std::thread::hardware_concurrency();
        uint32_t triSize = vertices.size() / 3;
        uint32_t triPerThread = triSize / threadSize;
        uint32_t remainingTris = triSize % threadSize;
        std::vector<std::thread> drawVerticesThr(threadSize);
        uint32_t start = 0;
        for (uint32_t i = 0; i < threadSize; i++) {
            uint32_t end = start + triPerThread + ((i < remainingTris) ? 1 : 0);
            drawVerticesThr[i] = std::thread(Renderer::drawVerticesThr, std::cref(vertices), std::cref(material), wireframe, start, end);
            start = end;
        }
        for (uint32_t i = 0; i < threadSize; i++) {
            drawVerticesThr[i].join();
        }
    }
}
void renderMesh(const Mesh &mesh, const Transform &transform, bool multithread) {
    std::vector<Vector> vertices = {};
    getDrawableTriangles(mesh.triangles, transform, vertices, multithread);

    sceneSettings.triSeenCount += (vertices.size() / 3);
    bool drawWireframe = (sceneSettings.debugState == DebugState::DS_TRIANGLE);
    drawVertices(vertices, mesh.material, drawWireframe, multithread);
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
        std::vector<Vector> drawableTris = {};
        Vector points[3] = {
            striangle.p[0],
            striangle.p[1],
            striangle.p[2],
        };
        modelSpaceToDrawable(points, { { 0, 0, 0 }, 1.f, { 0, 0, 0 } }, drawableTris);

        for (size_t i = 0; i < drawableTris.size(); i += 3) {
            Vector p[3] = {
                drawableTris[i],
                drawableTris[i + 1],
                drawableTris[i + 2],
            };
            drawVerticesTriangle(p, striangle.material, isWireframe);
        }
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
