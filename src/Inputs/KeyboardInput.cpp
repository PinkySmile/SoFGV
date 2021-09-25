//
// Created by Gegel85 on 25/09/2021.
//

#include "KeyboardInput.hpp"

namespace Battle
{
	KeyboardInput::KeyboardInput(const std::map<sf::Keyboard::Key, InputEnum> &keyMap) :
		_keyMap(keyMap)
	{
		this->_keyStates.fill(false);
		this->_keyDuration.fill(0);
	}

	void KeyboardInput::update()
	{
		for (int i = 0; i < INPUT_NUMBER; i++)
			if (this->_keyStates[i])
				this->_keyDuration[i]++;
	}

	void KeyboardInput::consumeEvent(const sf::Event &event)
	{
		if (event.type != sf::Event::KeyPressed && event.type != sf::Event::KeyReleased)
			return;

		auto it = this->_keyMap.find(event.key.code);

		if (it == this->_keyMap.end())
			return;
		if (this->_keyStates[it->second] != (event.type == sf::Event::KeyPressed)) {
			this->_keyDuration[it->second] = 0;
			this->_keyStates[it->second] = event.type == sf::Event::KeyPressed;
		}
	}

	bool KeyboardInput::isPressed(InputEnum input) const
	{
		return this->_keyStates[input];
	}

	InputStruct KeyboardInput::getInputs() const
	{
		return {
			this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT],
			this->_keyDuration[INPUT_UP] - this->_keyDuration[INPUT_DOWN],
			this->_keyDuration[INPUT_N],
			this->_keyDuration[INPUT_M],
			this->_keyDuration[INPUT_S],
			this->_keyDuration[INPUT_V],
			this->_keyDuration[INPUT_A],
		};
	}
}