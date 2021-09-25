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
		if (
			this->_action < ACTION_GROUND_HIGH_NEUTRAL_BLOCK ||
			this->_action > ACTION_AIR_HIT
		)
			this->_hitSpeed = {0, 0};
		else
			this->_hitSpeed *= this->_blockStun / (this->_blockStun + 1.f);
		this->_position.x += this->_pushSpeed;
		this->_position += this->_hitSpeed;
		this->_pushSpeed *= 0.5;
		if (this->_pushSpeed < 0.5)
			this->_pushSpeed = 0;
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
		if (input.horizontalAxis) {
			if (std::copysign(1, input.horizontalAxis) == std::copysign(1, this->_dir)) {
				this->_startMove(ACTION_WALK_FORWARD);
				return;
			}
			this->_startMove(ACTION_WALK_BACKWARD);
		} else
			this->_startMove(ACTION_IDLE);
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
		if (this->_action == action)
			return false;
		if (this->_action <= ACTION_WALK_BACKWARD)
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
		if (this->_action == ACTION_5N) {
			this->_forceStartMove(ACTION_IDLE);
			return;
		}
		AObject::_onMoveEnd();
	}

	void ACharacter::hit(IObject &other, const FrameData *data)
	{
		this->_pushSpeed = data->pushBack * -this->_dir;
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
			this->_hitSpeed = data->hitSpeed;
			logger.debug(std::to_string(this->_blockStun) + " hitstun frames");
			//this->_pushSpeed = data->pushBlock * -this->_dir;
		} else {
			this->_forceStartMove(ACTION_GROUND_HIGH_NEUTRAL_BLOCK);
			this->_blockStun = data->blockStun;
			this->_pushSpeed = data->pushBlock * -this->_dir;
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
}
