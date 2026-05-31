#include "Object.h"
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
    // triangles = {};
    boundingBox = {};
    material = {};
}
Mesh::Mesh(std::vector<Vector> vertex, std::vector<Vector> normal, std::vector<Texture> text, std::vector<Face> face, Colour color, float reflectiveness, float specular) {
    vertices = vertex;
    normals = normal;
    texture = text;
    faces = face;
    // triangles = triangle;
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
}

void Mesh::getTriangles(std::vector<Triangle> &tris) {
    tris.clear();
    tris.reserve(faces.size());
    for (const Face &face : faces) {
        Vector p[3] = {
            vertices[face.index[0].vert],
            vertices[face.index[1].vert],
            vertices[face.index[2].vert]
        };
        Triangle tri(p, { 0, 0, 0 }, material.color, material.specular, material.reflectiveness);
        tris.push_back(tri);
    }
}
