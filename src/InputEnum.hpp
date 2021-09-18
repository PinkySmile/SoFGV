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
		INPUT_N,
		INPUT_NEUTRAL = INPUT_N,
		INPUT_M,
		INPUT_MATTER = INPUT_M,
		INPUT_S,
		INPUT_SPIRIT = INPUT_S,
		INPUT_V,
		INPUT_VOID = INPUT_V,
		INPUT_A,
		INPUT_ASCEND = INPUT_A,
	};

	struct Input {
		int horizontalAxis;
		int verticalAxis;
		int n;
		int m;
		int s;
		int v;
		int a;
	};
}

#endif //BATTLE_INPUTENUM_HPP
