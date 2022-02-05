//
// Created by Gegel85 on 04/02/2022.
//

#ifndef BATTLE_MOVELISTDATA_HPP
#define BATTLE_MOVELISTDATA_HPP


#include <vector>
#include <string>
#include <map>

namespace Battle
{
	enum MoveSprite {
		SPRITE_2,
		SPRITE_3,
		SPRITE_4,
		SPRITE_6,
		SPRITE_8,
		SPRITE_214,
		SPRITE_236,
		SPRITE_421,
		SPRITE_426,
		SPRITE_623,
		SPRITE_624,
		SPRITE_624684,
		SPRITE_N,
		SPRITE_D,
		SPRITE_M,
		SPRITE_S,
		SPRITE_V,
		SPRITE_A,
		NB_SPRITES,
		SPRITE_AIR = NB_SPRITES,
		SPRITE_TRANSFORM,
		SPRITE_PLUS,
		SPRITE_CHARGE,
		SPRITE_ALSO_AIR
	};

	struct MoveData {
		std::string name;
		std::string description;
		std::vector<MoveSprite> input;
		bool displayed = true;

		MoveData() = default;
		MoveData(
			std::string name,
			std::string description,
			std::vector<MoveSprite> input,
			bool displayed = true
		) :
			name(std::move(name)),
			description(std::move(description)),
			input(std::move(input)),
			displayed(displayed)
		{}

	};

	extern const std::vector<unsigned> defaultMoveOrder;
	extern const std::vector<unsigned> defaultCommandOrder;
	extern const std::map<unsigned, MoveData> defaultMoveData;
}


#endif //BATTLE_MOVELISTDATA_HPP
