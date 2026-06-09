#define _CRT_SECURE_NO_WARNINGS
#include "Object.h"
#include "Logging.h"
#include "Timer.h"

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

// Triangle
Triangle::Triangle() {
    p[0] = {};
    p[1] = {};
    p[2] = {};
    material.color = { 255, 0, 0 };
    material.reflectiveness = 0;
    material.specular = -1;
}

Triangle::Triangle(const Vector _p[3], Vector _normal, Colour color, float specular, float reflectiveness) {
    this->p[0] = _p[0];
    this->p[1] = _p[1];
    this->p[2] = _p[2];
    this->material.color = color;
    this->material.specular = specular;
    this->material.reflectiveness = reflectiveness;
    this->normal = (_normal == Vector{ 0, 0, 0 }) ? cross((p[1] - p[0]), (p[2] - p[0])) : _normal;
}

Triangle::Triangle(const Vector _p[3], Vector _normal, Material _material) {
    this->p[0] = _p[0];
    this->p[1] = _p[1];
    this->p[2] = _p[2];
    this->material = _material;
    this->normal = (_normal == Vector{ 0, 0, 0 }) ? cross((p[1] - p[0]), (p[2] - p[0])) : normal;
}

Vector Triangle::calculateNormal() {
    // anti clockwise
    normal = cross((p[1] - p[0]), (p[2] - p[0]));
    return normal;
}

Vector Triangle::getCentroid() {
    return ((p[0] + p[1] + p[2]) / 3);
}

// Mesh
Mesh::Mesh() {
    vertices = {};
    normals = {};
    texture = {};
    faces = {};
    triangles = {};
    boundingBox = {};
    material = {};
}
Mesh::Mesh(std::vector<Vector> vertex, std::vector<Vector> normal, std::vector<Texture> text, std::vector<Face> face, Colour color, float reflectiveness, float specular) {
    vertices = vertex;
    normals = normal;
    texture = text;
    faces = face;
    material.color = color;
    material.reflectiveness = reflectiveness;
    material.specular = specular;
}
void Mesh::initTriangles() {
    Vector lowest, highest;
    int count = 0;
    for (const Vector &vertex : vertices) {
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
    if (triangles.size()) {
        return;
    }
    triangles.clear();
    triangles.reserve(faces.size() * 3);
    for (const Face &face : faces) {
        Vector p[3] = {
            vertices[face.index[0].vert],
            vertices[face.index[1].vert],
            vertices[face.index[2].vert]
        };
        triangles.push_back(p[0]);
        triangles.push_back(p[1]);
        triangles.push_back(p[2]);
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