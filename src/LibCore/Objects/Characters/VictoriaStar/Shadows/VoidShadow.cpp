//
// Created by PinkySmile on 06/05/23.
//

#include "VoidShadow.hpp"

namespace SpiralOfFate
{
	VoidShadow::VoidShadow(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		bool tint
	) :
		Shadow(frameData, hp, direction, pos, owner, id, tint ? sf::Color{0x67, 0x03, 0x3D} : sf::Color::White, 6)
	{
	}

	Shadow *VoidShadow::create(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		bool tint
	){
		return new VoidShadow(frameData, hp, direction, pos, owner, id, tint);
	}
}