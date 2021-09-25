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
			this->_position = {-150, 0};
		} else {
			this->_position = {150, 0};
		}
	}

	void ACharacter::consumeEvent(const sf::Event &event)
	{
		this->_input->consumeEvent(event);
	}
}
