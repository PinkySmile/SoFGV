//
// Created by PinkySmile on 29/03/23.
//

#include "Shadow.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	Shadow::Shadow(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		const sf::Color &tint,
		unsigned activateBlock
	) :
		SubObject(id, owner),
		_activateBlock(activateBlock)
	{
		this->_position = pos;
		this->_dir = direction ? 1 : -1;
		this->_direction = direction;
		this->_team = 4;
		this->_moves[0] = frameData;
		this->_hp = this->_baseHp = hp;
		this->_sprite.setColor(tint);
	}

	void Shadow::_onMoveEnd(const FrameData &lastData)
	{
		if (this->_actionBlock == ANIMBLOCK_SPAWNING) {
			this->_actionBlock = ANIMBLOCK_IDLE;
			my_assert2(this->_moves.at(this->_action).size() > this->_actionBlock, "Shadow " + std::to_string(this->_action) + " is missing block " + std::to_string(this->_actionBlock));
		} else if (this->_actionBlock == ANIMBLOCK_RESURRECT) {
			this->_actionBlock = this->_activateBlock;
			my_assert2(this->_moves.at(this->_action).size() > this->_actionBlock, "Shadow " + std::to_string(this->_action) + " is missing block " + std::to_string(this->_actionBlock));
		} else if (this->_actionBlock != ANIMBLOCK_IDLE)
			this->_dead = true;
		Object::_onMoveEnd(lastData);
	}

	void Shadow::getHit(IObject &other, const FrameData *data)
	{
		auto dmg = data->damage;
		bool isP[2] = {
			&other == &*game->battleMgr->getLeftCharacter(),
			&other == &*game->battleMgr->getRightCharacter()
		};
		auto byOwner = isP[this->getOwner()];

		this->_ownerKilled |= byOwner;
		if (dmg >= this->_hp || byOwner)
			this->_die();
		else
			this->_hp -= dmg;
		Object::getHit(other, data);

		auto o = reinterpret_cast<Shadow *>(&other);

		if (o->getTeam() != this->getOwner() || !o->_hasHit)
			o->_hitStop = data->hitPlayerHitStop;
	}

	void Shadow::_die()
	{
		this->_hp = 0;
		this->_actionBlock = ANIMBLOCK_DYING;
		my_assert2(this->_moves.at(this->_action).size() > this->_actionBlock, "Shadow " + std::to_string(this->_action) + " is missing block " + std::to_string(this->_actionBlock));
	}

	void Shadow::activate()
	{
		if (this->_actionBlock >= ANIMBLOCK_NORMAL_ACTIVATED)
			return;
		this->_ownerKilled = false;
		if (this->_actionBlock == ANIMBLOCK_DYING) {
			this->_animationCtr = this->getCurrentFrameData()->duration - this->_animationCtr - 1;
			this->_animation = this->_moves.at(this->_action)[this->_actionBlock].size() - this->_animation - 1;
			this->_actionBlock = ANIMBLOCK_RESURRECT;
		} else {
			this->_animationCtr = 0;
			this->_animation = 0;
			this->_actionBlock = this->_activateBlock;
		}
	}

	unsigned int Shadow::getBufferSize() const
	{
		return Object::getBufferSize() + sizeof(Data);
	}

	void Shadow::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		Object::copyToBuffer(data);
		game->logger.verbose("Saving Shadow (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
		dat->_invincibleTime = this->_invincibleTime;
		dat->_boxSize = this->_boxSize;
		dat->_loopInfo = this->_loopInfo;
		dat->_idleCounter = this->_idleCounter;
	}

	void Shadow::restoreFromBuffer(void *data)
	{
		Object::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		this->_invincibleTime = dat->_invincibleTime;
		this->_boxSize = dat->_boxSize;
		this->_loopInfo = dat->_loopInfo;
		this->_idleCounter = dat->_idleCounter;
		game->logger.verbose("Restored Shadow @" + std::to_string((uintptr_t)dat));
	}

	size_t Shadow::printDifference(const char *msgStart, void *data1, void *data2) const
	{
		auto length = Object::printDifference(msgStart, data1, data2);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		if (dat1->_invincibleTime != dat2->_invincibleTime)
			game->logger.fatal(std::string(msgStart) + "Shadow::_invincibleTime: " + std::to_string(dat1->_invincibleTime) + " vs " + std::to_string(dat2->_invincibleTime));
		if (dat1->_boxSize != dat2->_boxSize)
			game->logger.fatal(std::string(msgStart) + "Shadow::_boxSize: " + std::to_string(dat1->_boxSize) + " vs " + std::to_string(dat2->_boxSize));
		if (dat1->_loopInfo != dat2->_loopInfo)
			game->logger.fatal(std::string(msgStart) + "Shadow::_loopInfo: {" + std::to_string(dat1->_loopInfo.first) + "," + std::to_string(dat1->_loopInfo.second) + "} vs {" + std::to_string(dat2->_loopInfo.first) + "," + std::to_string(dat2->_loopInfo.second) + "}");
		if (dat1->_idleCounter != dat2->_idleCounter)
			game->logger.fatal(std::string(msgStart) + "Shadow::_idleCounter: " + std::to_string(dat1->_idleCounter) + " vs " + std::to_string(dat2->_idleCounter));
		return length + sizeof(Data);
	}

	void Shadow::update()
	{
		if (this->_hitStop) {
			this->_hitStop--;
			this->_computeFrameDataCache();
			return;
		}
		this->_idleCounter += this->_actionBlock == ANIMBLOCK_IDLE && this->_idleCounter < 120;
		this->_tickMove();
		if (this->_invincibleTime)
			this->_invincibleTime--;
		this->_boxSize += this->getCurrentFrameData()->manaGain;
		this->_computeFrameDataCache();
		this->_applyNewAnimFlags();
		this->_applyMoveAttributes();
	}

	void Shadow::setInvincible(unsigned int time)
	{
		this->_invincibleTime = time;
	}

	void Shadow::_applyNewAnimFlags()
	{
		if (!this->_newAnim)
			return;

		auto data = this->getCurrentFrameData();

		if (data->specialMarker == 0)
			return Object::_applyNewAnimFlags();
		if (data->specialMarker == 1) {
			this->_loopInfo.first = this->_animation;
			return Object::_applyNewAnimFlags();
		}
		if (this->_loopInfo.second == 0)
			this->_loopInfo.second = data->specialMarker;
		if (--this->_loopInfo.second == 0)
			return Object::_applyNewAnimFlags();
		this->_animation = this->_loopInfo.first;
		this->_computeFrameDataCache();
		Object::_applyNewAnimFlags();
	}

	int Shadow::getLayer() const
	{
		return -100;
	}

	bool Shadow::wasOwnerKilled() const
	{
		return this->_ownerKilled;
	}

	void Shadow::_computeFrameDataCache()
	{
		Object::_computeFrameDataCache();
		if (this->_invincibleTime) {
			this->_fdCache.dFlag.invulnerable = true;
			this->_fdCache.dFlag.grabInvulnerable = true;
			this->_fdCache.dFlag.projectileInvul = true;
		}
		for (auto &box : this->_fdCache.hitBoxes) {
			box.pos.x -= this->_boxSize / 2;
			box.pos.y -= this->_boxSize / 2;
			box.size.x += this->_boxSize;
			box.size.y += this->_boxSize;
		}
	}
}