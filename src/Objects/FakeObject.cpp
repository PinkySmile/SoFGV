//
// Created by Gegel85 on 05/02/2022.
//

#include "FakeObject.hpp"
#include "ACharacter.hpp"

Battle::FakeObject::FakeObject(const std::map<unsigned int, std::vector<std::vector<FrameData>>> &frameData)
{
	this->showBoxes = true;
	this->_moves = frameData;
}

void Battle::FakeObject::render() const
{
	auto oldPos = this->_position;

	const_cast<FakeObject *>(this)->_position = {700 , 190};
	AObject::render();
	const_cast<FakeObject *>(this)->_position = oldPos;
}

void Battle::FakeObject::_onMoveEnd(const Battle::FrameData &lastData)
{
	if (this->_action == ACTION_BEING_KNOCKED_DOWN)
		return this->_forceStartMove(ACTION_KNOCKED_DOWN);

	if (this->_action == ACTION_KNOCKED_DOWN)
		return this->_forceStartMove(ACTION_NEUTRAL_TECH);

	if (this->_action == ACTION_CROUCHING)
		return this->_forceStartMove(ACTION_CROUCH);

	auto idleAction = this->_isGrounded() ? (lastData.dFlag.crouch ? ACTION_CROUCH : ACTION_IDLE) : ACTION_FALLING;

	if (this->_action == ACTION_BACKWARD_AIR_TECH || this->_action == ACTION_FORWARD_AIR_TECH || this->_action == ACTION_UP_AIR_TECH || this->_action == ACTION_DOWN_AIR_TECH)
		return this->_forceStartMove(idleAction);
	if (this->_action == ACTION_BACKWARD_TECH || this->_action == ACTION_FORWARD_TECH || this->_action == ACTION_NEUTRAL_TECH)
		return this->_forceStartMove(idleAction);
	if (this->_action == ACTION_STANDING_UP)
		return this->_forceStartMove(idleAction);
	if (this->_action == ACTION_AIR_TECH_LANDING_LAG)
		return this->_forceStartMove(idleAction);
	if (this->_action == ACTION_FORWARD_DASH)
		return this->_forceStartMove(idleAction);
	if (this->_action == ACTION_BACKWARD_DASH)
		return this->_forceStartMove(idleAction);
	if (this->_action == ACTION_HARD_LAND)
		return this->_forceStartMove(idleAction);
	if (
		this->_action >= ACTION_5N ||
		this->_action == ACTION_LANDING
	)
		return this->_forceStartMove(idleAction);

	if (this->_action >= ACTION_AIR_DASH_1 && this->_action <= ACTION_AIR_DASH_9)
		return this->_forceStartMove(this->_isGrounded() ? ACTION_HARD_LAND : ACTION_FALLING);
	if (this->_action == ACTION_NEUTRAL_JUMP || this->_action == ACTION_FORWARD_JUMP || this->_action == ACTION_BACKWARD_JUMP)
		return this->_forceStartMove(idleAction);
	if (this->_action == ACTION_NEUTRAL_AIR_JUMP || this->_action == ACTION_FORWARD_AIR_JUMP || this->_action == ACTION_BACKWARD_AIR_JUMP)
		return this->_forceStartMove(idleAction);
	if (this->_action == ACTION_NEUTRAL_HIGH_JUMP || this->_action == ACTION_FORWARD_HIGH_JUMP || this->_action == ACTION_BACKWARD_HIGH_JUMP)
		return this->_forceStartMove(idleAction);
	AObject::_onMoveEnd(lastData);
}
