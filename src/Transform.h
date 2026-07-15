#pragma once
#include "Vector.h"
#include <cmath>
#include <numbers>
struct Transform {
    Vector position = { 0, 0, 0 };
    float scale = 1.f;
    Vector rotation = { 0, 0, 0 };
};
enum class RotateOrder {
    RO_YXZ = 0,
    RO_XYZ
};
static Vector rotate(const Vector &vec, const Vector &rotationP, const RotateOrder &ro = RotateOrder::RO_YXZ) {
    if (rotationP == Vector{ 0, 0, 0 }) {
        return vec;
    }

    float sinx, siny, sinz, cosx, cosy, cosz;
    Vector rotation = { float(rotationP.x * (std::numbers::pi * 2)) / 360.f, float(rotationP.y * (std::numbers::pi * 2)) / 360.f, float(rotationP.z * (std::numbers::pi * 2)) / 360.f };

    sinx = std::sin(rotation.x);
    siny = std::sin(rotation.y);
    sinz = std::sin(rotation.z);

    cosx = std::cos(rotation.x);
    cosy = std::cos(rotation.y);
    cosz = std::cos(rotation.z);

    Vector result = {};
    if (ro == RotateOrder::RO_YXZ) {
        // Yaw
        Vector yrotated;
        yrotated.y = vec.y;
        yrotated.x = vec.x * cosy + vec.z * (-siny);
        yrotated.z = vec.x * siny + vec.z * cosy;

        // Pitch
        Vector xrotated;
        xrotated.x = yrotated.x;
        xrotated.y = yrotated.y * cosx + yrotated.z * (-sinx);
        xrotated.z = yrotated.y * sinx + yrotated.z * cosx;

        // Roll
        Vector zrotated;
        zrotated.z = xrotated.z;
        zrotated.x = xrotated.x * cosz + xrotated.y * (-sinz);
        zrotated.y = xrotated.x * sinz + xrotated.y * cosz;

        result = zrotated;
    } else if (ro == RotateOrder::RO_XYZ) {
        Vector xrotated;
        xrotated.x = vec.x;
        xrotated.y = vec.y * cosx + vec.z * (-sinx);
        xrotated.z = vec.y * sinx + vec.z * cosx;

        Vector yrotated;
        yrotated.y = xrotated.y;
        yrotated.x = xrotated.x * cosy + xrotated.z * (-siny);
        yrotated.z = xrotated.x * siny + xrotated.z * cosy;

        Vector zrotated;
        zrotated.z = yrotated.z;
        zrotated.x = yrotated.x * cosz + yrotated.y * (-sinz);
        zrotated.y = yrotated.x * sinz + yrotated.y * (cosz);

        result = zrotated;
    }
    return result;
}
inline Vector transformVertex(const Vector &vec, const Transform &tf, RotateOrder ro = RotateOrder::RO_YXZ) {
    return ((rotate(vec, tf.rotation, ro) * tf.scale) + tf.position);
}
