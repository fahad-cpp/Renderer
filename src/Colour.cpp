#include "Colour.h"
#include <algorithm>
#include <cstdint>

Colour::Colour() {
    R = 0;
    G = 0;
    B = 0;
}
Colour::Colour(uint8_t _R, uint8_t _G, uint8_t _B) {
    this->R = _R;
    this->G = _G;
    this->B = _B;
}

bool operator==(const Colour color1, const Colour color2) {
    return ((color1.R == color2.R) && (color1.G == color2.G) && (color1.B == color2.B));
}
Colour operator*(const Colour color, const float num) {
    uint16_t R = std::clamp(uint16_t(color.R * num), uint16_t(0), uint16_t(255));
    uint16_t G = std::clamp(uint16_t(color.G * num), uint16_t(0), uint16_t(255));
    uint16_t B = std::clamp(uint16_t(color.B * num), uint16_t(0), uint16_t(255));
    return { uint8_t(R), uint8_t(G), uint8_t(B) };
}
Colour operator+(const Colour color1, const Colour color2) {
    uint16_t R = std::clamp(uint16_t(color1.R + color2.R), uint16_t(0), uint16_t(255));
    uint16_t G = std::clamp(uint16_t(color1.G + color2.G), uint16_t(0), uint16_t(255));
    uint16_t B = std::clamp(uint16_t(color1.B + color2.B), uint16_t(0), uint16_t(255));
    return { uint8_t(R), uint8_t(G), uint8_t(B) };
}

float Colour::luminance() {
    return ((0.2126f * float(R)) + (0.7152f * float(G)) + (0.0722f * float(B)));
}