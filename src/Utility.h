#pragma once
template <typename T>
void swap(T &a, T &b) {
    T c;
    c = a;
    a = b;
    b = c;
}
template <typename T>
void clamp(T &num, T minLimit, T maxLimit) {
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
bool isIn(T value, T lower, T higher) {
    if ((value >= higher) || (value <= lower)) {
        return false;
    }
    return true;
}

float getMax(const float &n1, const float &n2);
