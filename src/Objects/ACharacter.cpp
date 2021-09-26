//
// Created by PinkySmile on 18/09/2021
//

#include "ACharacter.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"

namespace Battle
{
	ACharacter::ACharacter(const std::string &frameData, IInput *input) :
		_input(input)
	{
		this->_moves = FrameData::loadFile(frameData);
	}

	void ACharacter::render() const
	{
		AObject::render();
	}

	void ACharacter::update()
	{
		this->_input->update();
		AObject::update();
		if (this->_isGrounded())
			this->_jumpsUsed = 0;
		if (this->_blockStun) {
			this->_blockStun--;
			if (this->_blockStun == 0)
				this->_forceStartMove(ACTION_IDLE);
		} else {
			auto inputs = this->_input->getInputs();

			this->_processInput(inputs);
		}

		if (this->_position.x < 0)
			this->_position.x = 0;
		if (this->_position.y < 0)
			this->_position.y = 0;
		if (this->_position.x > 1000)
			this->_position.x = 1000;
		if (this->_position.y > 1000)
			this->_position.y = 1000;
	}

	void ACharacter::init(bool side)
	{
		this->_dir = side ? 1 : -1;
		this->_direction = side;
		this->_team = side;
		this->_hp = 10000;
		this->_maxJumps = 1;
		this->_baseGravity = this->_gravity = {0, -1};
		if (side) {
			this->_position = {200, 0};
		} else {
			this->_position = {800, 0};
		}
	}

	void ACharacter::consumeEvent(const sf::Event &event)
	{
		this->_input->consumeEvent(event);
	}

	void ACharacter::_processInput(const InputStruct &input)
	{
		auto data = this->getCurrentFrameData();

		if (
			(data->dFlag.airborne && this->_executeAirborneMoves(input)) ||
			(!data->dFlag.airborne && this->_executeGroundMoves(input))
		)
			return;
		if (input.verticalAxis > 0) {
			if (input.horizontalAxis) {
				if (std::copysign(1, input.horizontalAxis) == std::copysign(1, this->_dir)) {
					if (this->_startMove(ACTION_FORWARD_JUMP))
						return;
				} else if (this->_startMove(ACTION_BACKWARD_JUMP))
					return;
			} else if (this->_startMove(ACTION_NEUTRAL_JUMP))
				return;
		}
		if (this->_isGrounded()) {
			if (input.horizontalAxis) {
				if (std::copysign(1, input.horizontalAxis) == std::copysign(1, this->_dir)) {
					this->_startMove(ACTION_WALK_FORWARD);
					return;
				}
				this->_startMove(ACTION_WALK_BACKWARD);
			} else
				this->_startMove(ACTION_IDLE);
		} else
			this->_startMove(ACTION_FALLING);
	}

	bool ACharacter::_executeAirborneMoves(const InputStruct &input)
	{
		return  (input.n && input.n <= 4 && this->_startMove(ACTION_j5N));
	}

	bool ACharacter::_executeGroundMoves(const InputStruct &input)
	{
		return  (input.n && input.n <= 4 && this->_startMove(ACTION_5N));
	}

	bool ACharacter::_canStartMove(unsigned action, const FrameData &data)
	{
		auto currentData = this->getCurrentFrameData();

		if (action == this->_action && currentData->oFlag.jab && currentData->oFlag.cancelable && this->_hasHit)
			return true;
		if (action == ACTION_NEUTRAL_JUMP || action == ACTION_FORWARD_JUMP || action == ACTION_BACKWARD_JUMP)
			return this->_jumpsUsed <= this->_maxJumps && (this->_action <= ACTION_WALK_BACKWARD || this->_action == ACTION_FALLING || this->_action == ACTION_LANDING);
		if (this->_action == action)
			return false;
		if (action <= ACTION_WALK_BACKWARD || action == ACTION_FALLING || action == ACTION_LANDING)
			return (this->_action <= ACTION_WALK_BACKWARD || this->_action == ACTION_FALLING || this->_action == ACTION_LANDING);
		if (this->_action <= ACTION_LANDING)
			return true;
		return false;
	}

	void ACharacter::_onMoveEnd()
	{
		if (this->_blockStun && !this->_actionBlock) {
			this->_actionBlock++;
			AObject::_onMoveEnd();
			return;
		}
		if (
			this->_action >= ACTION_5N ||
			this->_action == ACTION_LANDING
		)
			return this->_forceStartMove(ACTION_IDLE);
		if (this->_action == ACTION_NEUTRAL_JUMP || this->_action == ACTION_FORWARD_JUMP || this->_action == ACTION_BACKWARD_JUMP)
			return this->_forceStartMove(ACTION_FALLING);
		AObject::_onMoveEnd();
	}

	void ACharacter::hit(IObject &other, const FrameData *data)
	{
		this->_speed.x += data->pushBack * -this->_dir;
		AObject::hit(other, data);
	}

	void ACharacter::getHit(IObject &other, const FrameData *data)
	{
		AObject::getHit(other, data);

		if (!data)
			return;
		if (!this->_isBlocking()) {
			this->_forceStartMove(data->hitSpeed.y <= 0 ? ACTION_GROUND_HIGH_HIT : ACTION_AIR_HIT);
			this->_hp -= data->damage;
			this->_blockStun = data->hitStun;
			this->_speed += data->hitSpeed;
			this->_speed.x *= -this->_dir;
			logger.debug(std::to_string(this->_blockStun) + " hitstun frames");
		} else {
			this->_forceStartMove(ACTION_GROUND_HIGH_NEUTRAL_BLOCK);
			this->_blockStun = data->blockStun;
			this->_speed.x += data->pushBlock * -this->_dir;
		}
		game.battleMgr->addHitStop(data->hitStop);
	}

	bool ACharacter::_isBlocking()
	{
		auto *data = this->getCurrentFrameData();

		if (this->_input->isPressed(this->_direction ? INPUT_LEFT : INPUT_RIGHT) && data->dFlag.canBlock)
			return true;
		switch (this->_action) {
		case ACTION_GROUND_HIGH_NEUTRAL_BLOCK:
		case ACTION_GROUND_HIGH_SPIRIT_BLOCK:
		case ACTION_GROUND_HIGH_MATTER_BLOCK:
		case ACTION_GROUND_HIGH_VOID_BLOCK:
		case ACTION_GROUND_LOW_NEUTRAL_BLOCK:
		case ACTION_GROUND_LOW_SPIRIT_BLOCK:
		case ACTION_GROUND_LOW_MATTER_BLOCK:
		case ACTION_GROUND_LOW_VOID_BLOCK:
		case ACTION_AIR_NEUTRAL_BLOCK:
		case ACTION_AIR_SPIRIT_BLOCK:
		case ACTION_AIR_MATTER_BLOCK:
		case ACTION_AIR_VOID_BLOCK:
			return this->_blockStun != 0;
		}
		return false;
	}

	bool ACharacter::_isGrounded() const
	{
		return this->_position.y <= 0;
	}

	void ACharacter::_forceStartMove(unsigned int action)
	{
		if (action == ACTION_NEUTRAL_JUMP || action == ACTION_FORWARD_JUMP || action == ACTION_BACKWARD_JUMP)
			this->_jumpsUsed++;
		AObject::_forceStartMove(action);
	}
}
