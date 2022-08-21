//
// Created by PinkySmile on 27/09/2021.
//

#ifndef __ANDROID__
#include <cstring>
#include "RollbackInput.hpp"
#include "Logger.hpp"

namespace SpiralOfFate
{
	RollbackInput::RollbackInput()
	{
		this->_keyStates.reset();
		this->_keyDuration.fill(0);
	}

	bool RollbackInput::isPressed(InputEnum input) const
	{
		if (input == INPUT_RIGHT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) > 0;
		if (input == INPUT_LEFT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) < 0;
		return this->_keyStates[input];
	}

	InputStruct RollbackInput::getInputs() const
	{
		return {
			this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT],
			this->_keyDuration[INPUT_UP] - this->_keyDuration[INPUT_DOWN],
			this->_keyDuration[INPUT_N],
			this->_keyDuration[INPUT_M],
			this->_keyDuration[INPUT_S],
			this->_keyDuration[INPUT_V],
			this->_keyDuration[INPUT_A],
			this->_keyDuration[INPUT_D],
			this->_paused
		};
	}

	void RollbackInput::update()
	{
		for (size_t i = 0; i < this->_keyStates.size(); i++)
			if (this->_keyStates[i])
				this->_keyDuration[i]++;
			else
				this->_keyDuration[i] = 0;
	}

	void RollbackInput::consumeEvent(const sf::Event &)
	{
	}

	std::string RollbackInput::getName() const
	{
		return "Remote input";
	}

	std::vector<std::string> RollbackInput::getKeyNames() const
	{
		return std::vector<std::string>{INPUT_NUMBER};
	}
}
#endif