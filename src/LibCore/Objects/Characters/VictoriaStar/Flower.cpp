//
// Created by PinkySmile on 11/06/23.
//

#include "Flower.hpp"
#include "Resources/Game.hpp"
#include "Objects/Characters/VictoriaStar/VictoriaStar.hpp"

namespace SpiralOfFate
{
	Flower::Flower(
		VictoriaStar *ownerObj,
		const std::vector<std::vector<FrameData>> &frameData,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id
	) :
		SubObject(id, owner),
		_owner(ownerObj)
	{
		this->_position = pos;
		this->_dir = direction ? 1 : -1;
		this->_direction = direction;
		this->_team = owner;
		this->_moves[0] = frameData;
	}

	void Flower::_onMoveEnd(const FrameData &lastData)
	{
		if (this->_actionBlock == 0) {
			this->_actionBlock = 1;
			my_assert2(this->_moves.at(this->_action).size() > this->_actionBlock, "Flower is missing block " + std::to_string(this->_actionBlock));
		}
		if (this->_actionBlock == 2)
			this->_dead = true;
		Object::_onMoveEnd(lastData);
	}

	void Flower::update()
	{
		Object::update();
		if (this->_actionBlock != 1)
			return;
		if (this->_owner->_action == ACTION_CROUCHING || this->_owner->_action == ACTION_CROUCH)
			return;
		this->_actionBlock = 2;
		this->_animation = 0;
		this->_newAnim = true;
	}
}