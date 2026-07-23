#pragma once
#include <cmath>
struct Vector {
    float x;
    float y;
    float z;
    Vector(float _x = 0, float _y = 0, float _z = 0) {
        this->x = _x;
        this->y = _y;
        this->z = _z;
    }
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
