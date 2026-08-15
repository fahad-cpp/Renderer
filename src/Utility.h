#pragma once
template <typename T>
inline void swap(T &a, T &b) {
    T c = a;
    a = b;
    b = c;
}
template <typename T>
inline void swap(T *a, T *b) {
    T c = *a;
    *a = *b;
    *b = c;
}
template <typename T>
inline void clamp(T &num, T minLimit, T maxLimit) {
    if (num < minLimit) {
        num = minLimit;
        return;
    } else if (num > maxLimit) {
        num = maxLimit;
        return;
    }
    return;
}

template <typename T>
inline T clampv(const T val, T minLimit, T maxLimit) {
    T num = val;
    if (num < minLimit) {
        num = minLimit;
    } else if (num > maxLimit) {
        num = maxLimit;
    }
    return num;
}

template <typename T>
inline bool isIn(const T value, const T lower, const T higher) {
    return ((value <= higher) && (value >= lower));
}

template <typename T>
inline T lerp(const T first, const T second, float t) {
    clamp(t, 0.f, 1.f);
    return (first + ((second - first) * t));
}

float getMax(const float n1, const float n2);
