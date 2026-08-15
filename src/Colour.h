#ifndef COLOUR_H
#define COLOUR_H
#include <cstdint>
struct Colour {
    uint8_t R;
    uint8_t G;
    uint8_t B;

  public:
    Colour();
    Colour(uint8_t _R, uint8_t _G, uint8_t _B);

    friend bool operator==(const Colour color1, const Colour color2);
    friend Colour operator*(const Colour color, const float num);
    friend Colour operator+(const Colour color1, const Colour color2);
    float luminance();
};
inline Colour hexToRGB(uint32_t hex) {
    Colour color;
    color.R = uint8_t((hex >> 16) & 0xff);
    color.G = uint8_t((hex >> 8) & 0xff);
    color.B = uint8_t(hex & 0xff);
    return color;
}
inline uint32_t rgbtoHex(const Colour RGB) {
    return uint32_t(uint32_t(RGB.R << 16) | uint32_t(RGB.G << 8) | uint32_t(RGB.B));
}

#endif
