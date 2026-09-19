#define _CRT_SECURE_NO_WARNINGS
#include "Object.h"
#include "Logging.h"
#include "Timer.h"

#include <cstdio>
#include <fstream>
#include <vector>

// Sphere
bool operator==(const Sphere &sphere1, const Sphere &sphere2) {
    return ((sphere1.color == sphere2.color) && (sphere1.specular == sphere2.specular) && (sphere1.reflectiveness == sphere2.reflectiveness));
}

// Box
bool operator==(const Box &box2, const Box &box) {
    return ((box.lowest == box2.lowest) && (box.highest == box2.highest));
}
Vector getTriangleNormal(const Triangle &triangle) {
    return cross(triangle.points[1] - triangle.points[0], triangle.points[2] - triangle.points[0]);
}

// Mesh
Mesh::Mesh() {
    vertices = {};
    normals = {};
    texture = {};
    faces = {};
    triangleData = {};
    boundingBox = {};
    material = {};
}
Mesh::Mesh(std::vector<Vector> vertex, std::vector<Vector> normal, std::vector<Texture> text, std::vector<Face> face, Material material) {
    vertices = vertex;
    normals = normal;
    texture = text;
    faces = face;
    this->material = material;
}
void Mesh::initTriangles() {
    Vector lowest, highest;
    int count = 0;
    for (const Vector vertex : vertices) {
        if (count == 0) {
            lowest = vertex;
            highest = vertex;
        }
        if (vertex.x < lowest.x)
            lowest.x = vertex.x;
        if (vertex.y < lowest.y)
            lowest.y = vertex.y;
        if (vertex.z < lowest.z)
            lowest.z = vertex.z;
        if (vertex.x > highest.x)
            highest.x = vertex.x;
        if (vertex.y > highest.y)
            highest.y = vertex.y;
        if (vertex.z > highest.z)
            highest.z = vertex.z;
        count++;
    }
    boundingBox.lowest = lowest;
    boundingBox.highest = highest;
    this->getTriangles();
}

void Mesh::getTriangles() {
    if (triangleData.size()) {
        return;
    }
    triangleData.clear();
    triangleData.reserve(faces.size());
    for (const Face &face : faces) {
        const Vector p[3] = {
            vertices[face.index[0].vert],
            vertices[face.index[1].vert],
            vertices[face.index[2].vert]
        };
        const Vector n[3] = {
            normals[face.index[0].norm],
            normals[face.index[1].norm],
            normals[face.index[2].norm],
        };
        triangleData.emplace_back(Triangle{ { p[0], p[1], p[2] }, { n[0], n[1], n[2] } });
    }
}
inline static bool isNumeric(char c) {
    return (c >= '0' && c <= '9');
}
inline static char *getfloat(char *ptr, float *value) {
    while (!isNumeric(*ptr) && (*ptr != '-')) {
        ptr++;
    }
    char *end = ptr;
    while (isNumeric(*end) || (*end == '.') || (*end == '-')) {
        end++;
    }
    std::from_chars(ptr, end, *value);
    return end;
}
inline static char *getuint(char *ptr, uint32_t *value) {
    while (!isNumeric(*ptr)) {
        ptr++;
    }
    char *end = ptr;
    while (isNumeric(*end)) {
        end++;
    }
    std::from_chars(ptr, end, *value);
    return end;
}
// get floats seperated by space
inline static Vector get3floats(char *ptr) {
    Vector res;
    ptr = getfloat(ptr, &res.x);
    ptr = getfloat(ptr + 1, &res.y);
    ptr = getfloat(ptr + 1, &res.z);
    return res;
}

inline static void getIndices(char *ptr, uint32_t &v, uint32_t &t, uint32_t &n) {
    while (!isNumeric(*ptr)) {
        ptr++;
    }
    ptr = getuint(ptr, &v);
    if (*ptr != '/') {
        return;
    }
    ptr++;
    if (*ptr != '/') {
        ptr = getuint(ptr, &t);
    }

    if (*ptr != '/') {
        return;
    }
    ptr++;
    ptr = getuint(ptr, &n);
}
Mesh loadOBJ(const std::string &filename, const Material material) {
    Timer timer;
    LOG_INFO("Loading " << filename);
    std::vector<Vector> vertices = {};
    std::vector<Vector> normals = {};
    std::vector<Texture> textures = {};
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
            Vector position = get3floats(line.data() + 2);
            vertices.emplace_back(position);
        } else if (ptr[0] == 'v' && ptr[1] == 't' && (ptr[2] == ' ' || ptr[2] == '\t')) {
            // float u, v, w;
            // std::sscanf(line.c_str(), "vt %f %f %f", &u, &v, &w);
            // Texture newtext({ u, v, w });
            // textures.emplace_back(newtext);
        } else if (ptr[0] == 'v' && ptr[1] == 'n' && (ptr[2] == ' ' || ptr[2] == '\t')) {
            Vector newnorm = get3floats(line.data() + 3);
            normals.emplace_back(newnorm);
        } else if (ptr[0] == 'f' && (ptr[1] == ' ' || ptr[1] == '\t')) {
            std::istringstream stream(line.c_str() + 1);
            std::vector<Index> faceIndices;
            faceIndices.reserve(3);
            std::string vertex;
            // Handle arbitrary amount of vertices in a face
            while (stream >> vertex) {
                uint32_t v = 0, t = 0, n = 0;
                getIndices(vertex.data(), v, t, n);
                faceIndices.emplace_back(v - 1, t - 1, n - 1);
            }
            if (faceIndices.size() < 3) {
                LOG_ERROR("Less than 3 points in face: " << filename << "\n");
                return {};
            }

            // Use TRIANGLE_FAN ordering
            for (std::size_t i = 2; i < faceIndices.size(); i++) {
                faces.emplace_back(Face{ faceIndices[0], faceIndices[i - 1], faceIndices[i] });
            }
        }

        // skip until EOF or newline
        while ((*ptr != '\0') && *ptr != '\n')
            ptr++;
        // skip newline
        if (*ptr == '\n')
            ptr++;
    }
    Mesh mesh = { vertices, normals, textures, faces };
    mesh.material = material;
    mesh.initTriangles();
    timer.Stop();
    LOG_SUCCESS("Loaded " << filename << ":" << timer.dtms << "ms");
    return mesh;
}