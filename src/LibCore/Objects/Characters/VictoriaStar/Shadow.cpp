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
}