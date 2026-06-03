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

    friend bool operator==(const Colour &color1, const Colour &color2);
    friend Colour operator*(const Colour &color, const float num);
    friend Colour operator+(const Colour &color1, const Colour &color2);
    float luminance();
};
Colour hexToRGB(uint32_t hex);
uint32_t rgbtoHex(const Colour &RGB);
#endif
