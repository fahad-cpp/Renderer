#pragma once
#include "Colour.h"
#include "Vector.h"
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

struct Material {
    float specular = -1.f;
    float reflectiveness = 0.f;
    Colour color = { 0, 0, 0 };
};
struct HitData {
    Material material = {};
    Vector normal = { 0, 0, 0 };
    float intersection = float(INFINITY);
};
struct Sphere {
    Vector center = { 0, 0, 0 };
    float radius = 0.f;
    float specular = -1.f;
    float reflectiveness = 0.f;
    Colour color = { 0, 0, 0 };

    friend bool operator==(const Sphere &sphere1, const Sphere &sphere2);
};

struct Triangle {
    Vector points[3];
    Vector normals[3];
};
Vector getTriangleNormal(const Triangle &triangle);
struct Plane {
    Vector normal;
    float offset;
};
struct Texture {
    float u;
    float v;
    float w;
};
struct Index {
    uint32_t vert;
    uint32_t text;
    uint32_t norm;
};
struct Face {
    Index index[3];
};
struct Box {
    Vector highest;
    Vector lowest;
    friend bool operator==(const Box &box2, const Box &box);
};
struct Mesh {
    std::vector<Vector> vertices;
    std::vector<Vector> normals;
    std::vector<Texture> texture;
    std::vector<Face> faces;
    std::vector<Triangle> triangleData;
    Box boundingBox;
    Material material;
    Mesh();
    Mesh(std::vector<Vector> vertex, std::vector<Vector> normal = {}, std::vector<Texture> text = {}, std::vector<Face> face = {}, Material material = {});
    void initTriangles();
    void getTriangles();
};

Mesh loadOBJ(const std::string &filename, const Material material);