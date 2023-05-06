//3
// Created by PinkySmile on 06/05/23.
//

#include "NeutralShadow.hpp"

namespace SpiralOfFate
{
	NeutralShadow::NeutralShadow(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		bool tint
	) :
		Shadow(frameData, hp, direction, pos, owner, id, tint ? sf::Color{0xA6, 0xA6, 0xA6} : sf::Color::White, ANIMBLOCK_NORMAL_ACTIVATED)
	{
	}

	Shadow *NeutralShadow::create(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		bool tint
	){
		return new NeutralShadow(frameData, hp, direction, pos, owner, id, tint);
	}
}