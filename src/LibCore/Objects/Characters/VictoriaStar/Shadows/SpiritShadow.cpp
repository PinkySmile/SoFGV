//
// Created by PinkySmile on 06/05/23.
//

#include "SpiritShadow.hpp"

namespace SpiralOfFate
{
	SpiritShadow::SpiritShadow(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		bool tint
	) :
		Shadow(frameData, hp, direction, pos, owner, id, tint ? sf::Color{0x33, 0xCC, 0xCC} : sf::Color::White, ANIMBLOCK_SPIRIT_ACTIVATED)
	{
	}

	Shadow *SpiritShadow::create(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		bool tint
	){
		return new SpiritShadow(frameData, hp, direction, pos, owner, id, tint);
	}
}