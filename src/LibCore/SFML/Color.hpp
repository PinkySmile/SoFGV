//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_COLOR_HPP
#define SOFGV_COLOR_HPP


#include <SFML/Graphics/Color.hpp>

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
		Color(const sf::Color &c) : r(c.r), g(c.g), b(c.b), a(c.a) {};
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

		void tint(const Color &other)
		{
			this->r = other.r * this->r / 255;
			this->g = other.g * this->g / 255;
			this->b = other.b * this->b / 255;
			this->a = other.a * this->a / 255;
		}

		operator sf::Color() const
		{
			return {this->r, this->g, this->b, this->a};
		}
	};
}


#endif //SOFGV_COLOR_HPP
