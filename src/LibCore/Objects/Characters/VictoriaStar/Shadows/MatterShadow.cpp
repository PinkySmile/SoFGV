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
		class Character *ownerObj,
		unsigned int id,
		bool tint
	) :
		Shadow(frameData, hp, direction, pos, owner, ownerObj, id, tint ? sf::Color{0xBB, 0x5E, 0x00} : sf::Color::White, ANIMBLOCK_MATTER_ACTIVATED)
	{
	}

	Shadow *MatterShadow::create(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		class Character *ownerObj,
		unsigned int id,
		bool tint
	)
	{
		return new MatterShadow(frameData, hp, direction, pos, owner, ownerObj, id, tint);
	}

	void MatterShadow::getHit(IObject &other, const FrameData *data)
	{
		Shadow::getHit(other, data);
		if (this->_hp == 0)
			return;
		this->_invincibleTime = 60;
		this->activate();
	}
}