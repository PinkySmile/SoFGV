//
// Created by PinkySmile on 18/09/2021
//

#include "ACharacter.hpp"

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

		auto inputs = this->_input->getInputs();

		this->_processInput(inputs);
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
		if (side) {
			this->_position = {300, 0};
		} else {
			this->_position = {700, 0};
		}
	}

	void ACharacter::consumeEvent(const sf::Event &event)
	{
		this->_input->consumeEvent(event);
	}

	void ACharacter::_processInput(const InputStruct &input)
	{
		auto data = this->getCurrentFrameData();

		if (data->dFlag.airborne && this->_executeAirborneMoves(input))
			return;
		else if (!data->dFlag.airborne && this->_executeGroundMoves(input))
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
		return false;
	}

	bool ACharacter::_executeGroundMoves(const InputStruct &input)
	{
		return  (input.n && input.n <= 4 && this->_startMove(ACTION_5N));
	}

	bool ACharacter::_canStartMove(unsigned action, const FrameData &data)
	{
		if (action == this->_action && data.oFlag.jab && data.oFlag.cancelable)
			return true;
		if (this->_action <= ACTION_WALK_BACKWARD)
			return true;
		return false;
	}

	void ACharacter::_onMoveEnd()
	{
		if (this->_action == ACTION_5N) {
			this->_forceStartMove(ACTION_IDLE);
			return;
		}
		AObject::_onMoveEnd();
	}
}
