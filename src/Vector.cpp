#include "Vector.h"
#include <cmath>
Vector operator+(const Vector vec1, const Vector vec) {
    return { (vec1.x + vec.x), (vec1.y + vec.y), (vec1.z + vec.z) };
}
Vector operator+(const Vector vec1, const float vec) {
    return { (vec1.x + vec), (vec1.y + vec), (vec1.z + vec) };
}
Vector operator*(const Vector vec1, const Vector vec) {
    return { (vec1.x * vec.x), (vec1.y * vec.y), (vec1.z * vec.z) };
}
Vector operator*(const Vector vec1, const float vec) {
    return { (vec1.x * vec), (vec1.y * vec), (vec1.z * vec) };
}
Vector operator/(const Vector vec1, const Vector vec) {
    return { (vec1.x / vec.x), (vec1.y / vec.y), (vec1.z / vec.z) };
}
Vector operator/(const Vector vec1, const float vec) {
    return { (vec1.x / vec), (vec1.y / vec), (vec1.z / vec) };
}
Vector operator-(const Vector vec1, const Vector vec) {
    return { (vec1.x - vec.x), (vec1.y - vec.y), (vec1.z - vec.z) };
}
Vector operator-(const Vector vec1, const float vec) {
    return { (vec1.x - vec), (vec1.y - vec), (vec1.z - vec) };
}
Vector operator-(const Vector vec1) {
    return { (-vec1.x), (-vec1.y), (-vec1.z) };
}
bool operator==(const Vector vec1, const Vector vec) {
    return ((vec.x == vec1.x) && (vec.y == vec1.y) && (vec.z == vec1.z));
}
bool operator!=(const Vector vec, const Vector vec2) {
    return ((vec2.x != vec.x) || (vec2.y != vec.y) || (vec2.z != vec.z));
}
Vector operator*(const float num, const Vector vec) {
    return { (vec.x * num), (vec.y * num), (vec.z * num) };
}
Vector operator/(const float num, const Vector vec) {
    return { (num / vec.x), (num / vec.y), (num / vec.z) };
}
Vector cross(const Vector vec1, const Vector vec2) {
    Vector cross = {};
    cross.x = (vec1.y * vec2.z) - (vec1.z * vec2.y);
    cross.y = (vec1.z * vec2.x) - (vec1.x * vec2.z);
    cross.z = (vec1.x * vec2.y) - (vec1.y * vec2.x);
    return cross;
}
float dot(const Vector first, const Vector second) {
    return ((first.x * second.x) + (first.y * second.y) + (first.z * second.z));
}
float length(const Vector vec) {
    return std::sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}
Vector normalize(const Vector vec) {
    return (vec / length(vec));
}
