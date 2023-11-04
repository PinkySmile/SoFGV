//
// Created by PinkySmile on 04/11/2023.
//

#include <cmath>
#include <fstream>
#include "ControllerInput.hpp"

namespace SpiralOfFate
{
	ControllerInput::ControllerInput(const std::map<InputEnum, ControllerKey *> &keyMap)
	{
		for (auto [key, value] : keyMap)
			this->_keyMap.emplace(key, value);
		this->_keyDuration.fill(0);
	}

	bool ControllerInput::isPressed(InputEnum input) const
	{
		if (input == INPUT_RIGHT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) > 0;
		if (input == INPUT_LEFT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) < 0;
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
			this->_keyDuration[INPUT_PAUSE],
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
		std::vector<std::string> result;

		result.resize(INPUT_NUMBER, "Not mapped");
		for (auto &pair : this->_keyMap)
			result[pair.first] = pair.second->toString();
		return result;
	}

	void ControllerInput::changeInput(InputEnum input, ControllerKey *controller)
	{
		this->_keyMap[input].reset(controller);
	}

	void ControllerInput::save(std::ofstream &stream) const
	{
		std::map<SpiralOfFate::InputEnum, std::pair<bool, int>> controllerMap;

		for (auto &pair : this->_keyMap)
			controllerMap[pair.first] = pair.second->save();
		for (auto &pair : controllerMap)
			stream.write(reinterpret_cast<char *>(&pair.second), sizeof(pair.second));
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
		if (event.type == sf::Event::JoystickDisconnected && (
			event.joystickConnect.joystickId == this->_joystickId || this->_joystickId == (unsigned)-1
		)) {
			this->_state = false;
			return;
		}
		if (event.type != sf::Event::JoystickButtonPressed && event.type != sf::Event::JoystickButtonReleased)
			return;
		if (event.joystickButton.joystickId != this->_joystickId && this->_joystickId != (unsigned)-1)
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

	std::pair<bool, int> ControllerButton::save()
	{
		return {false, this->_buttonId};
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
		if (event.type == sf::Event::JoystickDisconnected && (
			event.joystickConnect.joystickId == this->_joystickId || this->_joystickId == (unsigned)-1
		)) {
			this->_state = 0;
			return;
		}
		if (event.type != sf::Event::JoystickMoved)
			return;
		if (event.joystickMove.joystickId != this->_joystickId && this->_joystickId != (unsigned)-1)
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

	std::pair<bool, int> ControllerAxis::save()
	{
		return {true, (this->_axis) | ((int)(char)(this->_threshHold)) << 3};
	}
}