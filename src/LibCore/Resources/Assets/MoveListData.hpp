//
// Created by PinkySmile on 04/02/2022.
//

#ifndef SOFGV_MOVELISTDATA_HPP
#define SOFGV_MOVELISTDATA_HPP


#include <vector>
#include <string>
#include <map>
#include "Resources/MSVCMacros.hpp"

namespace SpiralOfFate
{
	enum MoveSprite {
		SPRITE_1,
		SPRITE_2,
		SPRITE_3,
		SPRITE_4,
		SPRITE_6,
		SPRITE_7,
		SPRITE_8,
		SPRITE_9,
		SPRITE_H1,
		SPRITE_H2,
		SPRITE_H3,
		SPRITE_H4,
		SPRITE_H6,
		SPRITE_H7,
		SPRITE_H8,
		SPRITE_H9,
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
		SPRITE_ALSO_AIR,
		SPRITE_TOGETHER
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

	extern MYDLL_API const std::array<std::string, NB_SPRITES> spritesPaths;
	extern MYDLL_API const std::vector<unsigned> defaultMoveOrder;
	extern MYDLL_API const std::vector<unsigned> defaultCommandOrder;
	extern MYDLL_API const std::map<unsigned, MoveData> defaultMoveData;
}


#endif //SOFGV_MOVELISTDATA_HPP
