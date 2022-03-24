//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_COLOR_HPP
#define BATTLE_COLOR_HPP


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


#endif //BATTLE_COLOR_HPP
