#include "Vector.h"
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