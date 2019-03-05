#ifndef BATTLE_DIRECTIONS_HPP
#define BATTLE_DIRECTIONS_HPP


namespace Directions {
	enum Directions {
		None	= 0,
		Up	= 1 << 0,
		Right	= 1 << 1,
		Down	= 1 << 2,
		Left	= 1 << 3
	};
}

#endif //BATTLE_DIRECTIONS_HPP
