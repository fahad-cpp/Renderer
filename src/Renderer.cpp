#include "Renderer.h"
#include "Colour.h"
#include "Globals.h"
#include "Hash.h"
#include "Logging.h"
#include "Object.h"
#include "Timer.h"
#include "Transform.h"
#include "Utility.h"
#include "Vector.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <minmax.h>
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
            float *value = (((float *)depthBuffer) + x + (y * renderState.width));
            clamp(*value, 0.f, 1.f);
            Colour color = { (uint8_t)((*value) * 255.f), (uint8_t)((*value) * 255.f), (uint8_t)((*value) * 255.f) };
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

void drawLine(Vector &a, Vector &b, const Colour &color) {
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
Mesh loadOBJ(const std::string &filename, const Colour &color, float reflectiveness, float specular) {
    Timer timer;
    LOG_INFO("Loading " << filename);
    std::vector<Vector> vertices = {};
    std::vector<Vector> normals = {};
    std::vector<Texture> texture = {};
    std::vector<Face> faces = {};

    std::ifstream OBJFile(filename, std::ios::binary | std::ios::ate);
    if (!OBJFile) {
        LOG_ERROR("Cannot open file " << filename << "\n");
        return {};
    }

    size_t size = OBJFile.tellg();
    OBJFile.seekg(0);

    std::vector<char> buffer(size + 1);
    OBJFile.read(buffer.data(), size);
    buffer[size] = '\0';
    OBJFile.close();

    const char *ptr = buffer.data();
    std::string line;
    while (*ptr != '\0') {
        const char *end = ptr;
        while ((*end != '\0') && *end != '\n')
            end++;
        line = std::string(ptr, end - ptr);

        if (ptr[0] == 'v' && (ptr[1] == ' ' || ptr[1] == '\t')) {
            float x = 0, y = 0, z = 0;
            std::sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);
            vertices.emplace_back(x, y, z);
        } else if (ptr[0] == 'v' && ptr[1] == 't' && (ptr[2] == ' ' || ptr[2] == '\t')) {
            float u, v, w;
            std::sscanf(line.c_str(), "vt %f %f %f", &u, &v, &w);
            Texture newtext({ u, v, w });
            texture.emplace_back(newtext);
        } else if (ptr[0] == 'v' && ptr[1] == 'n' && (ptr[2] == ' ' || ptr[2] == '\t')) {
            float x, y, z;
            std::sscanf(line.c_str(), "vn %f %f %f", &x, &y, &z);
            Vector newnorm(x, y, z);
            normals.emplace_back(newnorm);
        } else if (ptr[0] == 'f' && (ptr[1] == ' ' || ptr[1] == '\t')) {
            //---Only works for 3 Vertices faces---
            uint32_t v[3] = {};
            uint32_t t[3] = {};
            uint32_t n[3] = {};
            Face newface = {};
            if (std::sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
                            &v[0], &t[0], &n[0],
                            &v[1], &t[1], &n[1],
                            &v[2], &t[2], &n[2]) == 9) {
                newface = {
                    Index{ v[0] - 1, t[0], n[0] },
                    Index{ v[1] - 1, t[1], n[1] },
                    Index{ v[2] - 1, t[2], n[2] }
                };
                faces.emplace_back(newface);
            } else if (std::sscanf(line.c_str(), "f %d//%d %d//%d %d//%d",
                                   &v[0], &n[0],
                                   &v[1], &n[1],
                                   &v[2], &n[2]) == 6) {
                newface = {
                    Index{ v[0] - 1, 0, n[0] },
                    Index{ v[1] - 1, 0, n[1] },
                    Index{ v[2] - 1, 0, n[2] }
                };
                faces.emplace_back(newface);
            } else {
                LOG_ERROR(("Unsupported face format :" + filename + "\n"));
                return {};
            }
        }

        while ((*ptr != '\0') && *ptr != '\n')
            ptr++;
        if (*ptr == '\n')
            ptr++;
    }
    Mesh mesh = { vertices, normals, texture, faces };
    mesh.material.color = color;
    mesh.material.specular = specular;
    mesh.material.reflectiveness = reflectiveness;
    mesh.initTriangles();
    timer.Stop();
    LOG_SUCCESS("Loaded " << filename << ":" << timer.dtms << "ms");
    return mesh;
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
void interpolate(int x0, int y0, int x1, int y1, std::vector<int> &arr) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    float aspectratio = (dy != 0) ? (float(dx) / dy) : 0.00001f;
    float x = x0;

    size_t size = y1 - y0;
    arr.resize(size);
    int idx = 0;
    for (int y = y0; y < y1; y++) {
        arr[idx] = int(x);
        idx++;
        x += aspectratio;
    }
}
void drawTriangle(const Triangle &t, bool wireframe) {
    Vector p1;
    Vector p2;
    Vector p3;
    // Project triangle on 2d viewport
    p1 = projectVertex(t.p[0]);
    p2 = projectVertex(t.p[1]);
    p3 = projectVertex(t.p[2]);
    p1.z = t.p[0].z;
    p2.z = t.p[1].z;
    p3.z = t.p[2].z;

    const Colour &color = t.material.color;

    // sort from top to bottom
    if (p1.y > p2.y) {
        swap(p1, p2);
    }
    if (p1.y > p3.y) {
        swap(p1, p3);
    }
    if (p2.y > p3.y) {
        swap(p2, p3);
    }

    if (wireframe) {
        // Draw wireframe triangle
        drawLine(p1, p2, color);
        drawLine(p2, p3, color);
        drawLine(p3, p1, color);
        return;
    }

    std::vector<float> x01;
    std::vector<float> x12;
    std::vector<float> x02;

    std::vector<float> z01;
    std::vector<float> z12;
    std::vector<float> z02;

    interpolate(p1.x, p1.y, p2.x, p2.y, x01);
    interpolate(p2.x, p2.y, p3.x, p3.y, x12);
    interpolate(p1.x, p1.y, p3.x, p3.y, x02);

    float z0 = (1.f / p1.z);
    float z1 = (1.f / p2.z);
    float z2 = (1.f / p3.z);
    interpolate(z0, p1.y, z1, p2.y, z01);
    interpolate(z1, p2.y, z2, p3.y, z12);
    interpolate(z0, p1.y, z2, p3.y, z02);

    // concatenate short sides in 0-1
    {
        int idx = x01.size();
        x01.resize(x01.size() + x12.size());
        for (uint32_t i = 0; i < x12.size(); i++) {
            x01.at(idx) = x12.at(i);
            idx++;
        }

        idx = z01.size();
        z01.resize(z01.size() + z12.size());
        for (uint32_t i = 0; i < z12.size(); i++) {
            z01.at(idx) = z12.at(i);
            idx++;
        }
    }

    int m = uint32_t(x02.size() / 2.f);
    std::vector<float> *xLeft = {};
    std::vector<float> *xRight = {};
    std::vector<float> *zLeft = {};
    std::vector<float> *zRight = {};

    // Finding left and right
    if (x02.size()) {
        if (x02[m] < x01[m]) {
            xLeft = &x02;
            xRight = &x01;

            zLeft = &z02;
            zRight = &z01;
        } else {
            xLeft = &x01;
            xRight = &x02;

            zLeft = &z01;
            zRight = &z02;
        }
    }

    // Camera space to world space
    Triangle newTri;
    newTri.p[0] = transformVertex(t.p[0], camera, RotateOrder::RO_XYZ);
    newTri.p[1] = transformVertex(t.p[1], camera, RotateOrder::RO_XYZ);
    newTri.p[2] = transformVertex(t.p[2], camera, RotateOrder::RO_XYZ);
    Vector N = newTri.calculateNormal();
    N = N / length(N);

    for (int y = int(p1.y); y < int(p3.y); y++) {
        int ny = int((renderState.height / 2.f) - y);
        float zL = (*zLeft)[y - int(p1.y)];
        float zR = (*zRight)[y - int(p1.y)];
        float xL = (*xLeft)[y - int(p1.y)];
        float xR = (*xRight)[y - int(p1.y)];
        std::vector<float> zSegment = {};
        interpolate(zL, xL, zR, xR, zSegment);
        for (int x = int(xL); x < int(xR); x++) {
            // Per Fragment
            float invZ = zSegment[x - int(xL)];
            float z = (1.f / invZ);
            Vector T = canvasToViewport((x * z) / d, (y * z) / d);
            T.z = z;
            if (isIn(float(x), float(-canvas.x / 2.f), float(canvas.x / 2.f)) && isIn(float(y), float(-canvas.y / 2.f), float(canvas.y / 2.f))) {
                int nx = int(x + (renderState.width / 2.0));
                // Pointer to depth buffer
                float *dep = ((float *)(depthBuffer)) + (ny * renderState.width) + nx;
                int tx = x + renderState.width / 2;
                int ty = (renderState.height / 2) - y;
                uint32_t idx = tx + (ty * renderState.width);
                std::lock_guard<std::mutex> lock(pixelLocks[idx]);
                if (invZ > (*dep)) {
                    // Point in camera space
                    Vector P = T;
                    // Camera space to world space
                    P = transformVertex(P, camera, RotateOrder::RO_XYZ);
                    Vector R = P - camera.position;
                    R = R / length(R);
                    const float light = computeLight(P, N, R, t.material.specular, false);
                    uint32_t hexColor = rgbtoHex(color * light);
                    ((uint32_t *)renderState.memory)[idx] = hexColor;
                    *dep = invZ;
                }
            }
        }
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

    std::vector<int> x01 = {};
    // Reserve space for x01 + x12 because we concatenate later
    x01.reserve(size01 + size12);
    std::vector<int> x12 = {};
    x12.reserve(size12);
    std::vector<int> x02 = {};
    x02.reserve(size02);

    std::vector<float> z01 = {};
    // Reserve space for z01 + z12 because we concatenate later
    z01.reserve(size01 + size12);
    std::vector<float> z12 = {};
    z12.reserve(size12);
    std::vector<float> z02 = {};
    z02.reserve(size02);

    interpolate(projected[0].x, projected[0].y, projected[1].x, projected[1].y, x01);
    interpolate(projected[1].x, projected[1].y, projected[2].x, projected[2].y, x12);
    interpolate(projected[0].x, projected[0].y, projected[2].x, projected[2].y, x02);

    interpolate(z0, projected[0].y, z1, projected[1].y, z01);
    interpolate(z1, projected[1].y, z2, projected[2].y, z12);
    interpolate(z0, projected[0].y, z2, projected[2].y, z02);

    // Concatenate short sides
    for (const int &x : x12) {
        x01.push_back(x);
    }

    for (const float &z : z12) {
        z01.push_back(z);
    }

    uint32_t middle = uint32_t(x02.size() / 2.f);
    std::vector<int> *xleft = nullptr;
    std::vector<int> *xright = nullptr;
    std::vector<float> *zleft = nullptr;
    std::vector<float> *zright = nullptr;
    if ((!x02.size())) {
        return;
    }
    // Find left and right
    if (x01[middle] > x02[middle]) {
        xleft = &x01;
        xright = &x02;

        zleft = &z01;
        zright = &z02;
    } else {
        xleft = &x02;
        xright = &x01;

        zleft = &z02;
        zright = &z01;
    }

    Vector worldSpace[3] = {
        transformVertex(p[0], camera, RotateOrder::RO_XYZ),
        transformVertex(p[1], camera, RotateOrder::RO_XYZ),
        transformVertex(p[2], camera, RotateOrder::RO_XYZ)
    };

    Vector normal = cross((worldSpace[1] - worldSpace[0]), (worldSpace[2] - worldSpace[0]));
    normal = normal / length(normal);

    for (int y = int(projected[0].y); y < int(projected[2].y); y++) {
        uint32_t scanline = uint32_t(y - int(projected[0].y));
        float lz = (*zleft)[scanline];
        float rz = (*zright)[scanline];
        int lx = (*xleft)[scanline];
        int rx = (*xright)[scanline];

        // interpolate z
        std::vector<float> zsegment = {};
        interpolate(lz, float(lx), rz, float(rx), zsegment);

        for (int x = lx; x < rx; x++) {
            float invz = zsegment[x - lx];
            float z = 1.f / invz;

            Vector point = canvasToViewport(x * z / d, y * z / d);
            point.z = z;
            Vector direction = point - camera.position;
            direction = direction / length(direction);

            int screenx = x + (renderState.width / 2.f);
            int screeny = (renderState.height / 2.f) - y;
            uint32_t index = (screeny * renderState.width) + screenx;
            if ((!isIn(screenx, 0, int(renderState.width)) || !isIn(screeny, 0, int(renderState.height)))) {
                continue;
            }
            float dep = ((float *)depthBuffer)[index];
            if (invz <= dep) {
                continue;
            }
            Colour color = material.color * computeLight(point, normal, direction,material.specular, false);
            ((float *)depthBuffer)[index] = invz;
            ((uint32_t *)renderState.memory)[index] = rgbtoHex(color);
        }
    }
}
void drawFaces(const std::vector<Vector> &vertices, const std::vector<Face> &faces, const Material &material, bool wireframe) {
    for (const Face &face : faces) {
        Vector p[3] = {
            vertices.at(face.index[0].vert),
            vertices.at(face.index[1].vert),
            vertices.at(face.index[2].vert)
        };
        drawVerticesTriangle(p, material, wireframe);
    }
}
void drawBox(Box box, Transform tf, bool inTriangle) {
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
    int psize = sizeof(p) / sizeof(p[0]);
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
        Triangle boxTris[12] = {
            { tris[0], Vector{ 0, 0, -1 }, Material{ red, -1.f, 0.f } },
            { tris[1], { 0, 0, -1 }, red },
            { tris[2], { 0, 0, 1 }, red },
            { tris[3], { 0, 0, 1 }, red },
            { tris[4], { -1, 0, 0 }, red },
            { tris[5], { -1, 0, 0 }, red },
            { tris[6], { 1, 0, 0 }, red },
            { tris[7], { 1, 0, 0 }, red },
            { tris[8], { 0, 1, 0 }, red },
            { tris[9], { 0, 1, 0 }, red },
            { tris[10], { 0, -1, 0 }, red },
            { tris[11], { 0, -1, 0 }, red },
        };
        size_t size = 12;
        std::vector<Triangle> triangles;
        for (size_t i = 0; i < size; i++) {
            triangles.push_back(boxTris[i]);
        }
        std::vector<Triangle> FinalTris;
        for (Triangle &t : triangles) {
            std::vector<Triangle> clippedTris = clipTriangle(t);
            for (Triangle &ct : clippedTris)
                FinalTris.push_back(ct);
        }
        for (Triangle &t : FinalTris) {
            drawTriangle(t, true);
        }
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
        float T = sphereInt;
        if (isIn(T, tMin, tMax) && (T < hitData.intersection)) {
            hitData.intersection = T;
            Vector P = O + (D * T);
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
        std::vector<Triangle> triangles = {};
        instance.mesh->getTriangles(triangles);
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
        for (Triangle &triangle : triangles) {
            Triangle tri;
            tri.p[0] = transformVertex(triangle.p[0], instance.transform);
            tri.p[1] = transformVertex(triangle.p[1], instance.transform);
            tri.p[2] = transformVertex(triangle.p[2], instance.transform);

            float triangleInt = intersectRayTriangle(O, D, tri);
            if (isIn(triangleInt, tMin, tMax) && triangleInt < hitData.intersection) {
                hitData.intersection = triangleInt;
                hitData.material = triangle.material;
                hitData.normal = tri.normal;
            }
        }
    }
    return hitData;
}
Vector reflectRay(const Vector &R, Vector &N) {
    return (2 * (N * dot(R, N)) - R);
}
float computeLight(Vector &P, Vector &N, const Vector V, float s, bool rtShadows) {
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
void FXAAthr(int threadNum, int threadCount, float edgeThreshold) {
    int yCount = (renderState.height - 2) / threadCount;
    int ymin = (threadNum * yCount) + 1;
    int ymax = ymin + yCount;
    for (int y = ymin; y < ymax; y++) {
        for (uint32_t x = 1; x < renderState.width - 1; x++) {
            Colour colorCenter = getPixel(x, y);
            Colour colorTop = getPixel(x, y - 1);
            Colour colorBottom = getPixel(x, y + 1);
            Colour colorLeft = getPixel(x - 1, y);
            Colour colorRight = getPixel(x + 1, y);

            float topLuma = colorTop.luminance();
            float bottomLuma = colorBottom.luminance();
            float leftLuma = colorLeft.luminance();
            float rightLuma = colorRight.luminance();

            float edgeHorizontal = std::abs(leftLuma - rightLuma);
            float edgeVertical = std::abs(topLuma - bottomLuma);

            bool isHorizontal = (edgeHorizontal >= edgeVertical);

            Colour blendColour;
            if (isHorizontal) {
                blendColour.R = ((colorTop.R + colorBottom.R) * 0.5f);
                blendColour.G = ((colorTop.G + colorBottom.G) * 0.5f);
                blendColour.B = ((colorTop.B + colorBottom.B) * 0.5f);
            } else {
                blendColour.R = ((colorLeft.R + colorRight.R) * 0.5f);
                blendColour.G = ((colorLeft.G + colorRight.G) * 0.5f);
                blendColour.B = ((colorLeft.B + colorRight.B) * 0.5f);
            }

            bool isEdge = (getMax(edgeHorizontal, edgeVertical) > edgeThreshold);

            if (isEdge) {
                putPixelD(x, y, blendColour);
            } else {
                putPixelD(x, y, colorCenter);
            }
        }
    }
}
void FXAA(bool multiThread) {
    float edgeThreshold = 0.f;
    if (!multiThread) {
        // Single Threaded FXAA
        for (uint32_t y = 1; y < (renderState.height - 1); y++) {
            for (uint32_t x = 1; x < (renderState.width - 1); x++) {
                Colour colorCenter = getPixel(x, y);
                Colour colorTop = getPixel(x, y - 1);
                Colour colorBottom = getPixel(x, y + 1);
                Colour colorLeft = getPixel(x - 1, y);
                Colour colorRight = getPixel(x + 1, y);

                float topLuma = colorTop.luminance();
                float bottomLuma = colorBottom.luminance();
                float leftLuma = colorLeft.luminance();
                float rightLuma = colorRight.luminance();

                float edgeHorizontal = std::abs(leftLuma - rightLuma);
                float edgeVertical = std::abs(topLuma - bottomLuma);

                bool isHorizontal = (edgeHorizontal >= edgeVertical);

                Colour blendColour;
                if (isHorizontal) {
                    blendColour.R = ((colorTop.R + colorBottom.R) * 0.5f);
                    blendColour.G = ((colorTop.G + colorBottom.G) * 0.5f);
                    blendColour.B = ((colorTop.B + colorBottom.B) * 0.5f);
                } else {
                    blendColour.R = ((colorLeft.R + colorRight.R) * 0.5f);
                    blendColour.G = ((colorLeft.G + colorRight.G) * 0.5f);
                    blendColour.B = ((colorLeft.B + colorRight.B) * 0.5f);
                }

                bool isEdge = (getMax(edgeHorizontal, edgeVertical) > edgeThreshold);

                if (isEdge) {
                    putPixelD(x, y, blendColour);
                } else {
                    putPixelD(x, y, colorCenter);
                }
            }
        }
    } else {
        // Multi-Threaded FXAA
        int threadCount = 12;
        std::vector<std::thread> tObjs(threadCount);
        for (int i = 0; i < threadCount; i++) {
            tObjs[i] = std::thread(FXAAthr, i, threadCount, 0.f);
        }
        for (int i = 0; i < threadCount; i++) {
            tObjs[i].join();
        }
    }
}
void drawTrianglesThr(const std::vector<Triangle> &tris, size_t start, size_t end, bool drawWireframe) {
    for (size_t i = start; i < end; i++) {
        drawTriangle(tris[i], drawWireframe);
    }
}
void drawTrianglesMultiThread(const std::vector<Triangle> &tris, bool drawWireframe, uint32_t numThreads) {
    size_t totalTriangles = tris.size();

    std::vector<std::thread> threads(numThreads);

    size_t trisPerThread = totalTriangles / numThreads;
    size_t remainingTriangles = totalTriangles % numThreads;

    size_t start = 0;
    for (uint32_t i = 0; i < numThreads; i++) {
        // distributing remaining triangles equally to starting n threads
        // where n is the number of remaining triangles
        size_t end = start + trisPerThread + ((i < remainingTriangles) ? 1 : 0);
        threads[i] = std::thread(&drawTrianglesThr, std::cref(tris), start, end, drawWireframe);
        start = end;
    }

    for (uint32_t i = 0; i < numThreads; i++) {
        threads[i].join();
    }
}
void modelSpaceToDrawable(const Triangle &triangle, const Transform &transform, std::vector<Triangle> &outTris) {
    Vector moved[3];
    // Model space to world space
    moved[0] = transformVertex(triangle.p[0], transform);
    moved[1] = transformVertex(triangle.p[1], transform);
    moved[2] = transformVertex(triangle.p[2], transform);

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
    newTri.material.color = triangle.material.color;

    // Frustum culling
    std::vector<Triangle> clippedTris = clipTriangle(newTri);

    // Save the drawable triangles in a vector
    for (const Triangle &ct : clippedTris) {
        outTris.push_back(ct);
    }
}
void MStoDrawableVertices(const std::vector<Vector> &verticesIn, const std::vector<Face> &facesIn, const Transform &transform, std::vector<Vector> &verticesOut, std::vector<Face> &facesOut) {
    verticesOut.reserve(verticesIn.size());
    facesOut.reserve(facesIn.size());
    std::unordered_map<Vector, uint32_t> indexMap = {};
    uint64_t indexCount = 0;
    for (const Vector &vertex : verticesIn) {
        Vector newVertex = vertex;
        // Model to world space
        newVertex = transformVertex(newVertex, transform);
        // world to camera space
        newVertex = newVertex - camera.position;
        newVertex = rotate(newVertex, -camera.rotation);

        verticesOut.push_back(newVertex);
        indexMap[newVertex] = indexCount;
        indexCount++;
    }
    for (const Face &face : facesIn) {
        const Vector &v1 = verticesOut[face.index[0].vert];
        const Vector &v2 = verticesOut[face.index[1].vert];
        const Vector &v3 = verticesOut[face.index[2].vert];
        // Backface culling
        Vector normal = cross((v2 - v1), (v3 - v1));
        normal = normal / length(normal);
        Vector PO = -v1;
        if (!(dot(normal, PO) > 0.f) && sceneSettings.bfc) {
            continue;
        }
        Vector p[3] = {
            v1,
            v2,
            v3
        };
        Triangle tri(p, { 0, 0, 0 }, Colour{ 0, 0, 0 });
        std::vector<Triangle> clippedTris = clipTriangle(tri);
        if (!clippedTris.size()) {
            continue;
        }
        for (const Triangle &ct : clippedTris) {
            Face newFace = {};
            for (int i = 0; i < 3; i++) {
                const Vector &vec = ct.p[i];

                std::unordered_map<Vector, uint32_t>::iterator it = indexMap.find(vec);

                if (it != indexMap.end()) {
                    newFace.index[i] = {
                        .vert = indexMap[vec],
                        .text = 0,
                        .norm = 0
                    };
                } else {
                    uint32_t ind = verticesOut.size();
                    verticesOut.push_back(vec);
                    indexMap[vec] = ind;
                    newFace.index[i] = {
                        .vert = ind,
                        .text = 0,
                        .norm = 0
                    };
                }
            }

            facesOut.push_back(newFace);
        }
    }
}
void modelSpaceToDrawableThr(const std::vector<Triangle> *inTris, const Transform &transform, std::vector<Triangle> *outTris, uint32_t start, uint32_t end) {
    for (uint32_t i = start; i < end; i++) {
        Triangle triangle = inTris->at(i);
        modelSpaceToDrawable(triangle, transform, *outTris);
    }
}

void getDrawableTriangles(const std::vector<Triangle> &inTris, const Transform &transform, std::vector<Triangle> &outTris, bool multithread) {
    if (!multithread) {
        for (const Triangle &triangle : inTris) {
            modelSpaceToDrawable(triangle, transform, outTris);
        }
    } else {
        const uint32_t threadSize = std::thread::hardware_concurrency();
        uint32_t triPerThread = inTris.size() / threadSize;
        uint32_t remainingTris = inTris.size() % threadSize;
        std::vector<std::thread> triProcessThr(threadSize);
        std::vector<std::vector<Triangle>> outTrisArr(threadSize);
        uint32_t start = 0;
        for (uint32_t i = 0; i < threadSize; i++) {
            uint32_t end = start + triPerThread + ((i < remainingTris) ? 1 : 0);
            triProcessThr[i] = std::thread(Renderer::modelSpaceToDrawableThr, &inTris, (transform), &outTrisArr[i], start, end);
            start = end;
        }
        for (uint32_t i = 0; i < threadSize; i++) {
            triProcessThr[i].join();
        }
        for (const std::vector<Triangle> &tris : outTrisArr) {
            for (const Triangle &triangle : tris) {
                outTris.push_back(triangle);
            }
        }
    }
}
void renderMesh(const Mesh &mesh, const Transform &transform, bool multithread) {
    std::vector<Vector> vertices = {};
    std::vector<Face> faces = {};
    MStoDrawableVertices(mesh.vertices, mesh.faces, transform, vertices, faces);

    sceneSettings.triSeenCount += faces.size();
    bool drawWireframe = (sceneSettings.debugState == DebugState::DS_TRIANGLE);
    if (true) { //! multithread) {
        drawFaces(vertices, faces, mesh.material, drawWireframe);
    } else if (faces.size()) {
        // Handle later
    }
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
        modelSpaceToDrawable(striangle, { { 0, 0, 0 }, 1.f, { 0, 0, 0 } }, drawableTris);

        for (const Triangle &tri : drawableTris) {
            drawTriangle(tri, isWireframe);
        }
    }
    // Apply AA
    if (sceneSettings.antiAliasing && (sceneSettings.debugState != DebugState::DS_TRIANGLE)) {
        FXAA();
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
void renderAO() {
}
}; // namespace Renderer
