//
// Created by PinkySmile on 06/05/23.
//

#include "MatterShadow.hpp"

namespace SpiralOfFate
{
	MatterShadow::MatterShadow(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		bool tint
	) :
		Shadow(frameData, hp, direction, pos, owner, id, tint ? sf::Color{0xBB, 0x5E, 0x00} : sf::Color::White, 4)
	{
	}

	Shadow *MatterShadow::create(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		bool tint
	){
		return new MatterShadow(frameData, hp, direction, pos, owner, id, tint);
	}
}