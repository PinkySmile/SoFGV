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
		this->_fakeData.setSlave();
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
		return Object::_onMoveEnd(lastData);
	}

	void Shadow::getHit(IObject &other, const FrameData *data)
	{
		auto dmg = data->damage;
		bool isP[2] = {
			&other == &*game->battleMgr->getLeftCharacter(),
			&other == &*game->battleMgr->getRightCharacter()
		};

		if (dmg >= this->_hp || isP[this->getOwner()])
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
	}

	void Shadow::restoreFromBuffer(void *data)
	{
		Object::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		this->_invincibleTime = dat->_invincibleTime;
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
		return length + sizeof(Data);
	}

	void Shadow::update()
	{
		Object::update();
		if (this->_invincibleTime)
			this->_invincibleTime--;
	}

	void Shadow::setInvincible(unsigned int time)
	{
		this->_invincibleTime = time;
	}

	const FrameData *Shadow::getCurrentFrameData() const
	{
		auto dat = Object::getCurrentFrameData();

		if (!this->_invincibleTime)
			return dat;
		this->_fakeData = *dat;

		return &this->_fakeData;
	}
}