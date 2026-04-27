#pragma once
#include "Colour.h"
#include "Vector.h"
#include "Logging.h"
#include <vector>
struct Material {
	Colour color = { 0,0,0 };
	float specular = -1.f;
	float reflectiveness = 0.f;
};
struct HitData {
	float intersection = float(INFINITY_V);
	Vector normal = { 0,0,0 };
	Material material = {};
};
struct Sphere {
	Vector center = { 0,0,0 };
	float radius = 0.f;
	Colour color = { 0,0,0 };
	float specular = -1.f;
	float reflectiveness = 0.f;
	bool operator==(const Sphere& sphere) const {
		return ((sphere.color == this->color) && (sphere.specular == this->specular) && (sphere.reflectiveness == this->reflectiveness));
	}
};

struct Triangle {
	Vector p[3];
	Vector normal;
	Material material;

	Triangle() {
		p[0] = {};
		p[1] = {};
		p[2] = {};
		material.color = { 255,0,0 };
		material.reflectiveness = 0;
		material.specular = -1;
	}

	Triangle(const Vector _p[3], Vector _normal = { 0,0,0 }, Colour color = { 0,0,0 }, float specular = -1, float reflectiveness = 0) {
		this->p[0] = _p[0];
		this->p[1] = _p[1];
		this->p[2] = _p[2];
		this->material.color = color;
		this->material.specular = specular;
		this->material.reflectiveness = reflectiveness;
		this->normal = cross((p[1] - p[0]), (p[2] - p[0]));
	}

	Triangle(const Vector _p[3], Vector _normal = { 0,0,0 }, Material _material = { Colour{0,0,0},-1.f,0.f }) {
		this->p[0] = _p[0];
		this->p[1] = _p[1];
		this->p[2] = _p[2];
		this->material = _material;
		this->normal = (_normal == Vector{0, 0, 0}) ? cross((p[1] - p[0]), (p[2] - p[0])): normal;
	}

	Vector calculateNormal(){
		//anti clockwise
		normal = cross((p[1] - p[0]), (p[2] - p[0]));
		return normal;
	}

	Vector getCentroid() {
		return ((p[0] + p[1] + p[2]) / 3);
	}

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
	int vert;
	int text;
	int norm;
};
struct Face {
	Index index[3];
};
struct Box {
	Vector highest;
	Vector lowest;
	friend bool operator==(const Box& box2, const Box& box) {
		return ((box.lowest == box2.lowest) && (box.highest == box2.highest));
	}
};
struct Mesh {
	std::vector<Vector> vertices;
	std::vector<Vector> normals;
	std::vector<Texture> texture;
	std::vector<Face> faces;
	std::vector<Triangle> triangles;
	Box boundingBox;
	Material material;
	Mesh() {
		vertices = {};
		normals = {};
		texture = {};
		faces = {};
		triangles = {};
		boundingBox = {};
		material = {};
	}
	Mesh(std::vector<Vector> vertex, std::vector<Vector> normal = {}, std::vector<Texture> text = {}, std::vector<Face> face = {}, std::vector<Triangle> triangle = {}, Colour color = { 0,0,0 }, float reflectiveness = 0.f, float specular = -1) {
		vertices = vertex;
		normals = normal;
		texture = text;
		faces = face;
		triangles = triangle;
		material.color = color;
		material.reflectiveness = reflectiveness;
		material.specular = specular;
	}
	void initTriangles() {
		Vector lowest, highest;
		int count = 0;
		if (triangles.size()) {
			return;
		}
		for (const Face& face : faces) {
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
		for (const Vector& vertex : vertices) {
			if (count == 0) {
				lowest = vertex;
				highest = vertex;
			}
			if (vertex.x < lowest.x)lowest.x = vertex.x;
			if (vertex.y < lowest.y)lowest.y = vertex.y;
			if (vertex.z < lowest.z)lowest.z = vertex.z;
			if (vertex.x > highest.x)highest.x = vertex.x;
			if (vertex.y > highest.y)highest.y = vertex.y;
			if (vertex.z > highest.z)highest.z = vertex.z;
			count++;
		}
		boundingBox.lowest = lowest;
		boundingBox.highest = highest;
	}
};
