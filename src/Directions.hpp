#ifndef BATTLE_DIRECTIONS_HPP
#define BATTLE_DIRECTIONS_HPP


namespace Directions {
	enum Directions {
		None	= 0,
		Moving	= 1,
		Up	= 1 << 1,
		Down	= 1 << 2,
		Left	= 1 << 3,
		Right	= 1 << 4,
	};

#define IsMoving(dir)	(dir & Directions::Moving)
#define UpDir(dir)	(dir & Directions::Up) && !(dir & Directions::Down)
#define DownDir(dir)	(dir & Directions::Down) && !(dir & Directions::Up)
#define LeftDir(dir)	(dir & Directions::Left) && !(dir & Directions::Right)
#define RightDir(dir)	(dir & Directions::Right) && !(dir & Directions::Left)
}


#endif //BATTLE_DIRECTIONS_HPP
