//
// Created by PinkySmile on 06/05/23.
//

#include "VoidShadow.hpp"
#include "Resources/Game.hpp"
#include "Objects/Characters/VictoriaStar/VictoriaStar.hpp"

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
		Shadow(frameData, hp, direction, pos, owner, id, tint ? sf::Color{0x67, 0x03, 0x3D} : sf::Color::White, ANIMBLOCK_VOID_ACTIVATED)
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
	)
	{
		return new VoidShadow(frameData, hp, direction, pos, owner, id, tint);
	}

	void VoidShadow::_onMoveEnd(const FrameData &lastData)
	{
		if (this->_actionBlock == this->_activateBlock) {
			this->_actionBlock = ANIMBLOCK_IDLE;
			this->_idleCounter = 0;
			this->_boxSize = 0;
			return Object::_onMoveEnd(lastData);
		}
		Shadow::_onMoveEnd(lastData);
	}

	void VoidShadow::update()
	{
		if (this->_hitStop) {
			this->_hitStop--;
			this->_computeFrameDataCache();
			return;
		}
		Shadow::update();
		if (this->_idleCounter != 120)
			return;

		auto color = this->_sprite.getColor();
		auto opponent = reinterpret_cast<VoidShadow *>(this->getOwner() ? game->battleMgr->getLeftCharacter() : game->battleMgr->getRightCharacter());

		if (!this->_attacking) {
			if (color.a <= 225)
				color.a += 30;
			else
				color.a = 255;
			this->_sprite.setColor(color);
			if (this->_actionBlock != ANIMBLOCK_IDLE)
				return;

			if (!VictoriaStar::isHitAction(opponent->_action))
				return;
			this->_attacking = true;
			this->_animationCtr = 0;
			this->_animation = 0;
			this->_invincibleTime = 60;
			this->_actionBlock = this->_activateBlock;
		}
		if (color.a < 30) {
			color.a = 0;
			this->_position = opponent->_position;
			this->_attacking = false;
		} else
			color.a -= 30;
		this->_sprite.setColor(color);
	}

	unsigned int VoidShadow::getBufferSize() const
	{
		return Shadow::getBufferSize() + sizeof(Data);
	}

	void VoidShadow::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Shadow::getBufferSize());

		Shadow::copyToBuffer(data);
		game->logger.verbose("Saving VoidShadow (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
		dat->_attacking = this->_attacking;
	}

	void VoidShadow::restoreFromBuffer(void *data)
	{
		Shadow::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Shadow::getBufferSize());

		this->_attacking = dat->_attacking;
		game->logger.verbose("Restored VictoriaStar @" + std::to_string((uintptr_t)dat));
	}

	size_t VoidShadow::printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const
	{
		auto length = Shadow::printDifference(msgStart, data1, data2, startOffset);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		game->logger.info("VoidShadow @" + std::to_string(startOffset + length));
		if (dat1->_attacking != dat2->_attacking)
			game->logger.fatal(std::string(msgStart) + "VoidShadow::_attacking: " + std::to_string(dat1->_attacking) + " vs " + std::to_string(dat2->_attacking));
		return length + sizeof(Data);
	}
}