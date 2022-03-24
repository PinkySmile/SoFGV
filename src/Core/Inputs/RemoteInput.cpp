//
// Created by Gegel85 on 27/09/2021.
//

#include <cstring>
#include "RemoteInput.hpp"
#include "../Logger.hpp"

namespace SpiralOfFate
{
	RemoteInput::RemoteInput()
	{
		this->_keyStates.reset();
		this->_keyDuration.fill(0);
	}

	bool RemoteInput::isPressed(InputEnum input) const
	{
		if (input == INPUT_RIGHT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) > 0;
		if (input == INPUT_LEFT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) < 0;
		return this->_keyStates[input];
	}

	InputStruct RemoteInput::getInputs() const
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

	void RemoteInput::update()
	{
		for (size_t i = 0; i < this->_keyStates.size(); i++)
			if (this->_keyStates[i])
				this->_keyDuration[i]++;
			else
				this->_keyDuration[i] = 0;
	}

	void RemoteInput::consumeEvent(const sf::Event &)
	{
	}

	std::string RemoteInput::getName() const
	{
		return "Remote input";
	}

	std::vector<std::string> RemoteInput::getKeyNames() const
	{
		return std::vector<std::string>{INPUT_NUMBER};
	}
}
