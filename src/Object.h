#pragma once
#include "Colour.h"
#include "Vector.h"
#include <cmath>
#include <cstdint>
#include <vector>
struct Material {
    Colour color = { 0, 0, 0 };
    float specular = -1.f;
    float reflectiveness = 0.f;
};
struct HitData {
    float intersection = float(INFINITY);
    Vector normal = { 0, 0, 0 };
    Material material = {};
};
struct Sphere {
    Vector center = { 0, 0, 0 };
    float radius = 0.f;
    Colour color = { 0, 0, 0 };
    float specular = -1.f;
    float reflectiveness = 0.f;

    friend bool operator==(const Sphere &sphere1, const Sphere &sphere2);
};

struct Triangle {
    Vector p[3];
    Vector normal;
    Material material;

    Triangle();
    Triangle(const Vector _p[3], Vector _normal = { 0, 0, 0 }, Colour color = { 0, 0, 0 }, float specular = -1, float reflectiveness = 0);
    Triangle(const Vector _p[3], Vector _normal = { 0, 0, 0 }, Material _material = { Colour{ 0, 0, 0 }, -1.f, 0.f });
    Vector calculateNormal();
    Vector getCentroid();
};

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
    std::vector<Vector> triangles;
    Box boundingBox;
    Material material;
    Mesh();
    Mesh(std::vector<Vector> vertex, std::vector<Vector> normal = {}, std::vector<Texture> text = {}, std::vector<Face> face = {}, Colour color = { 0, 0, 0 }, float reflectiveness = 0.f, float specular = -1);
    void initTriangles();
    void getTriangles();
};
