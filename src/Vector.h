#pragma once
#include <cmath>
struct Vector {
    float x;
    float y;
    float z;
    Vector(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
    friend Vector operator+(const Vector, const Vector);
    friend Vector operator+(const Vector, const float);
    friend Vector operator*(const Vector, const Vector);
    friend Vector operator*(const Vector, const float);
    friend Vector operator/(const Vector, const Vector);
    friend Vector operator/(const Vector, const float);
    friend Vector operator-(const Vector, const Vector);
    friend Vector operator-(const Vector, const float);
    friend Vector operator-(const Vector);
    friend bool operator==(const Vector, const Vector);
    friend bool operator!=(const Vector vec, const Vector);
    friend Vector operator*(const float, const Vector);
    friend Vector operator/(const float, const Vector);
    friend Vector operator-(const Vector, const Vector);
    friend Vector cross(const Vector, const Vector);
    friend float dot(const Vector, const Vector);
    friend float length(const Vector);
    friend Vector normalize(const Vector);
};
inline Vector cross(const Vector vec1, const Vector vec2) {
    Vector cross = {};
    cross.x = (vec1.y * vec2.z) - (vec1.z * vec2.y);
    cross.y = (vec1.z * vec2.x) - (vec1.x * vec2.z);
    cross.z = (vec1.x * vec2.y) - (vec1.y * vec2.x);
    return cross;
}
inline float dot(const Vector first, const Vector second) {
    return ((first.x * second.x) + (first.y * second.y) + (first.z * second.z));
}
inline float length(const Vector vec) {
    return std::sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}
inline Vector normalize(const Vector vec) {
    return (vec / length(vec));
}
