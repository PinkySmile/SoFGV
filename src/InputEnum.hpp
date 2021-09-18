//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_INPUTENUM_HPP
#define BATTLE_INPUTENUM_HPP

namespace Battle
{
	enum Input {
		INPUT_LEFT,
		INPUT_RIGHT,
		INPUT_UP,
		INPUT_DOWN,
		INPUT_A,
		INPUT_B,
		INPUT_C,
		INPUT_D,
		INPUT_E,
	};

	struct Input {
		int horizontalAxis;
		int verticalAxis;
		int a;
		int b;
		int c;
		int d;
		int e;
	};
}

#endif //BATTLE_INPUTENUM_HPP
