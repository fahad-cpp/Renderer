#include "Object.h"
#include <vector>

//Sphere
bool operator==(const Sphere& sphere1,const Sphere &sphere2) {
    return ((sphere1.color == sphere2.color) && (sphere1.specular == sphere2.specular) && (sphere1.reflectiveness == sphere2.reflectiveness));
}

//Box
bool operator==(const Box &box2, const Box &box) {
    return ((box.lowest == box2.lowest) && (box.highest == box2.highest));
}

//Triangle
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
    this->normal = (_normal == Vector{0,0,0})?cross((p[1] - p[0]), (p[2] - p[0])):_normal;
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

//Mesh
Mesh::Mesh() {
    vertices = {};
    normals = {};
    texture = {};
    faces = {};
    triangles = {};
    boundingBox = {};
    material = {};
}
Mesh::Mesh(std::vector<Vector> vertex, std::vector<Vector> normal, std::vector<Texture> text, std::vector<Face> face, std::vector<Triangle> triangle, Colour color, float reflectiveness, float specular) {
    vertices = vertex;
    normals = normal;
    texture = text;
    faces = face;
    triangles = triangle;
    material.color = color;
    material.reflectiveness = reflectiveness;
    material.specular = specular;
}
void Mesh::initTriangles() {
    Vector lowest, highest;
    int count = 0;
    if (triangles.size()) {
        return;
    }
    for (const Face &face : faces) {
        triangles.reserve(faces.size());
        Triangle triangle;
        Vector v1, v2, v3;
        v1 = vertices.at(face.index[0].vert - 1);
        v2 = vertices.at(face.index[1].vert - 1);
        v3 = vertices.at(face.index[2].vert - 1);
        triangle.material.reflectiveness = material.reflectiveness;
        triangle.material.specular = material.specular;
        triangle.material.color = this->material.color;
        triangle.p[0] = v1;
        triangle.p[1] = v2;
        triangle.p[2] = v3;
        triangle.normal = normals.at(face.index[0].norm - 1);
        triangles.push_back(triangle);
    }
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
