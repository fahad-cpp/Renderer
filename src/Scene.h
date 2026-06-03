#pragma once
#include "Object.h"
#include "Transform.h"
struct Instance {
    Mesh *mesh = nullptr;
    Transform transform = { { 0, 0, 0 }, 1.f, { 0, 0, 0 } };
    Box boundingBox = { -INFINITY, INFINITY };
    // Returns Bounding Box in world space
    Box getBoundingBox() {
        if (mesh == nullptr) {
            return {};
        }
        bool boundingBoxInitialized = !((boundingBox.lowest == INFINITY) && (boundingBox.highest == -INFINITY));
        if (!boundingBoxInitialized) {
            // find lowest and highest point in bounding box
            Vector lowest = { INFINITY, INFINITY, INFINITY };
            Vector highest = { -INFINITY, -INFINITY, -INFINITY };
            for (const Vector &vert : mesh->vertices) {
                Vector vertex = transformVertex(vert, transform);
                lowest.x = vertex.x < lowest.x ? vertex.x : lowest.x;
                lowest.y = vertex.y < lowest.y ? vertex.y : lowest.y;
                lowest.z = vertex.z < lowest.z ? vertex.z : lowest.z;

                highest.x = vertex.x > highest.x ? vertex.x : highest.x;
                highest.y = vertex.y > highest.y ? vertex.y : highest.y;
                highest.z = vertex.z > highest.z ? vertex.z : highest.z;
            }
            boundingBox = { highest, lowest };
            return boundingBox;
        } else {
            return boundingBox;
        }
    }
    void applyTransform(const Transform &tf) {
        transform.scale = tf.scale * transform.scale;
        transform.position = tf.position + transform.position;
        transform.rotation = tf.rotation + transform.rotation;
        boundingBox.lowest = INFINITY;
        boundingBox.highest = -INFINITY;
        getBoundingBox();
    }
};
enum LightType {
    LT_POINT,
    LT_DIRECTIONAL,
    LT_AMBIENT
};
struct Light {
    LightType type;
    Vector pos;
    Vector direction;
    float intensity;
};
struct Scene {
    std::vector<Sphere> spheres;
    std::vector<Triangle> triangles;
    std::vector<Instance> instances;
    std::vector<Light> lights;
};
