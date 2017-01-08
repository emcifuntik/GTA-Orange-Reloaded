#pragma once
namespace rage {
	class bgraColor {
		bgraColor(color_t rgba) {
			this->red = rgba.red;
			this->green = rgba.green;
			this->blue = rgba.blue;
			this->alpha = rgba.alpha;
		};
		bgraColor(DWORD hex) {
			Utils::HexToRGBA(hex, red, green, blue, alpha);
		};
		bgraColor(unsigned char blue, unsigned char green, unsigned char red, unsigned char alpha) {
			this->red = red;
			this->green = green;
			this->blue = blue;
			this->alpha = alpha;
		};
		~bgraColor() {};
		color_t ToRGBA(){return{ red, green, blue, alpha };}
		unsigned char blue;
		unsigned char green;
		unsigned char red;
		unsigned char alpha;
	};
}