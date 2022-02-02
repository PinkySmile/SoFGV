//
// Created by Gegel85 on 25/09/2021.
//

#include <fstream>
#include "KeyboardInput.hpp"

namespace Battle
{
	static std::vector<std::string> keyToString{
		"Unknown",
		"A",
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
		"L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",
		"Num0",
		"Num1",
		"Num2",
		"Num3",
		"Num4",
		"Num5",
		"Num6",
		"Num7",
		"Num8",
		"Num9",
		"Escape",
		"LControl",
		"LShift",
		"LAlt",
		"LSystem",
		"RControl",
		"RShift",
		"RAlt",
		"RSystem",
		"Menu",
		"LBracket",
		"RBracket",
		"Semicolon",
		"Comma",
		"Period",
		"Quote",
		"Slash",
		"Backslash",
		"Tilde",
		"Equal",
		"Hyphen",
		"Space",
		"Enter",
		"Backspace",
		"Tab",
		"PageUp",
		"PageDown",
		"End",
		"Home",
		"Insert",
		"Delete",
		"Add",
		"Subtract",
		"Multiply",
		"Divide",
		"Left",
		"Right",
		"Up",
		"Down",
		"Numpad0",
		"Numpad1",
		"Numpad2",
		"Numpad3",
		"Numpad4",
		"Numpad5",
		"Numpad6",
		"Numpad7",
		"Numpad8",
		"Numpad9",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",
		"F10",
		"F11",
		"F12",
		"F13",
		"F14",
		"F15",
		"Pause"
	};

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
		if (input == INPUT_RIGHT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) > 0;
		if (input == INPUT_LEFT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) > 0;
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
			this->_keyDuration[INPUT_D],
		};
	}

	std::string KeyboardInput::getName() const
	{
		return "Keyboard";
	}

	std::vector<std::string> KeyboardInput::getKeyNames() const
	{
		std::vector<std::string> result;

		result.resize(INPUT_NUMBER, "Not mapped");
		for (auto &pair : this->_keyMap)
			result[pair.second] = keyToString[pair.first + 1];
		return result;
	}

	void KeyboardInput::changeInput(InputEnum input, sf::Keyboard::Key key)
	{
		auto it = std::find_if(this->_keyMap.begin(), this->_keyMap.end(), [input](std::pair<sf::Keyboard::Key, InputEnum> pair){
			return pair.second == input;
		});

		if (it != this->_keyMap.end())
			this->_keyMap.erase(it);
		this->_keyMap[key] = input;
	}

	void KeyboardInput::save(std::ofstream &stream) const
	{
		std::map<Battle::InputEnum, sf::Keyboard::Key> realKeyboardMap;

		for (auto &pair : this->_keyMap)
			realKeyboardMap[pair.second] = pair.first;
		for (auto &pair : realKeyboardMap)
			stream.write(reinterpret_cast<char *>(&pair.second), sizeof(pair.second));
	}
}