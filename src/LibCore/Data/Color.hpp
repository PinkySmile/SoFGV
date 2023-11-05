//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_COLOR_HPP
#define SOFGV_COLOR_HPP


namespace SpiralOfFate
{
	union Color {
		struct {
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		};
		unsigned value;

		Color() {};
		Color(unsigned v) : value(v) {}
		Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) : r(r), g(g), b(b), a(a) {}

		bool operator==(const Color &other) const
		{
			return this->value == other.value;
		}

		bool operator!=(const Color &other) const
		{
			return !(*this == other);
		}
	};
}


#endif //SOFGV_COLOR_HPP
