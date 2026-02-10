#pragma once
#include "Typedefs.h"
#include "Vector.h"
struct Colour {
	u8 R;
	u8 G;
	u8 B;
public:
	Colour() {
		R = 0;
		G = 0;
		B = 0;
	}
	Colour(unsigned char R, unsigned char G, unsigned char B) {
		this->R = R;
		this->G = G;
		this->B = B;
	}

	bool operator==(const Colour& op)const {
		return (this->R == op.R && this->G == op.G && this->B == op.B);
	}
	Colour operator*(const float num) const {
		Vector newcol = { float(this->R * num),float(this->G * num),float(this->B * num) };
		if (newcol.x > 255) {
			newcol.x = 255;
		}
		if (newcol.y > 255) {
			newcol.y = 255;
		}
		if (newcol.z > 255) {
			newcol.z = 255;
		}
		return { u8(newcol.x),u8(newcol.y),u8(newcol.z) };
	}
	Colour operator+(const Colour& col) const {
		Vector newcol = { float(this->R + col.R),float(this->G + col.G),float(this->B + col.B) };
		if (newcol.x > 255) {
			newcol.x = 255;
		}
		if (newcol.y > 255) {
			newcol.y = 255;
		}
		if (newcol.z > 255) {
			newcol.z = 255;
		}
		return { u8(newcol.x),u8(newcol.y),u8(newcol.z) };
	}
	float luminance() {
		return ((0.2126f * float(R)) + (0.7152f * float(G)) + (0.0722f * float(B)));
	}
};
INTERNAL_W Colour hexToRGB(u32 hex) {
	Colour color;
	color.R = u8((hex >> 16) & 0xff);
	color.G = u8((hex >> 8) & 0xff);
	color.B = u8(hex & 0xff);
	return color;
}
INTERNAL_W u32 rgbtoHex(const Colour& RGB) {
	return u32((RGB.R << 16) | (RGB.G << 8) | RGB.B);
}