#pragma once
#include "Object.h"
// Hashing
template <typename T>
inline void hashCombine(std::size_t &seed, const T &value) {
    seed ^= (std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}
template <>
struct std::hash<Vector> {
    std::size_t operator()(const Vector &vec) const {
        std::size_t seed = 0;
        hashCombine(seed, vec.x);
        hashCombine(seed, vec.y);
        hashCombine(seed, vec.z);
        return seed;
    }
};
template <>
struct std::hash<Colour> {
    std::size_t operator()(const Colour &color) const {
        return std::size_t(rgbtoHex(color));
    }
};

// Custom Sphere Hashing
template <>
struct std::hash<Sphere> {
    std::size_t operator()(const Sphere &sphere) const {
        std::size_t seed = 0;
        hashCombine(seed, sphere.center);
        hashCombine(seed, sphere.radius);
        hashCombine(seed, sphere.color);
        hashCombine(seed, sphere.reflectiveness);
        hashCombine(seed, sphere.specular);
        return seed;
    }
};
