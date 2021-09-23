//
// Created by PinkySmile on 18/09/2021
//

#include "ACharacter.hpp"

namespace Battle
{
	ACharacter::ACharacter(const std::string &frameData)
	{
		this->_moves = FrameData::loadFile(frameData);
	}

	void ACharacter::render() const
	{
		AObject::render();
	}

	void ACharacter::update()
	{
		AObject::update();
	}

	void ACharacter::init(bool side)
	{
		this->_direction = side;
		if (side) {
			this->_position = {-600, 200};
		} else {
			this->_position = {600, 200};
		}
	}
}
