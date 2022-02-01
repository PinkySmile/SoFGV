//
// Created by Gegel85 on 25/09/2021.
//

#include <cmath>
#include "ControllerInput.hpp"

namespace Battle
{
	ControllerInput::ControllerInput(const std::map<InputEnum, ControllerKey *> &keyMap)
	{
		for (auto [key, value] : keyMap)
			this->_keyMap.emplace(key, value);
		this->_keyDuration.fill(0);
	}

	bool ControllerInput::isPressed(InputEnum input) const
	{
		return this->_keyMap.at(input)->isPressed();
	}

	InputStruct ControllerInput::getInputs() const
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

	void ControllerInput::update()
	{
		for (auto &[enumVal, key] : this->_keyMap)
			if (key->isPressed())
				this->_keyDuration[enumVal]++;
			else
				this->_keyDuration[enumVal] = 0;
	}

	void ControllerInput::consumeEvent(const sf::Event &event)
	{
		for (auto &[_, key] : this->_keyMap)
			key->consumeEvent(event);
	}

	void ControllerInput::setJoystickId(unsigned int id)
	{
		for (auto &keys : this->_keyMap)
			keys.second->setJoystickId(id);
	}

	std::string ControllerInput::getName() const
	{
		return "Controller";
	}

	std::vector<std::string> ControllerInput::getKeyNames() const
	{
		std::vector<std::string> result{INPUT_NUMBER};

		for (auto &pair : this->_keyMap)
			result[pair.first] = pair.second->toString();
		return result;
	}

	ControllerButton::ControllerButton(unsigned int joystickId, unsigned int buttonId) :
		_joystickId(joystickId),
		_buttonId(buttonId)
	{
	}

	bool ControllerButton::isPressed()
	{
		return this->_state;
	}

	void ControllerButton::consumeEvent(const sf::Event &event)
	{
		if (event.type == sf::Event::JoystickDisconnected && event.joystickConnect.joystickId == this->_joystickId) {
			this->_state = false;
			return;
		}
		if (event.type != sf::Event::JoystickButtonPressed && event.type != sf::Event::JoystickButtonReleased)
			return;
		if (event.joystickButton.joystickId != this->_joystickId)
			return;
		if (event.joystickButton.button != this->_buttonId)
			return;
		this->_state = event.type == sf::Event::JoystickButtonPressed;
	}

	void ControllerButton::setJoystickId(unsigned int id)
	{
		this->_joystickId = id;
	}

	std::string ControllerButton::toString()
	{
		return "Button " + std::to_string(this->_buttonId);
	}

	ControllerAxis::ControllerAxis(unsigned int joystickId, sf::Joystick::Axis axis, float threshHold) :
		_joystickId(joystickId),
		_threshHold(threshHold),
		_axis(axis)
	{
	}

	bool ControllerAxis::isPressed()
	{
		return std::copysign(1, this->_state) == std::copysign(1, this->_threshHold) &&
		       std::abs(this->_state) >= std::abs(this->_threshHold);
	}

	void ControllerAxis::consumeEvent(const sf::Event &event)
	{
		if (event.type == sf::Event::JoystickDisconnected && event.joystickConnect.joystickId == this->_joystickId) {
			this->_state = 0;
			return;
		}
		if (event.type != sf::Event::JoystickMoved)
			return;
		if (event.joystickButton.joystickId != this->_joystickId)
			return;
		if (event.joystickMove.axis != this->_axis)
			return;
		this->_state = event.joystickMove.position;
	}

	void ControllerAxis::setJoystickId(unsigned int id)
	{
		this->_joystickId = id;
	}

	std::string ControllerAxis::toString()
	{
		std::vector<std::string> axis{
			"X",
			"Y",
			"Z",
			"R",
			"U",
			"V",
			"PadX",
			"PadY"
		};

		return "Axis " + axis[this->_axis] + (this->_threshHold < 0 ? "-" : "+");
	}
}