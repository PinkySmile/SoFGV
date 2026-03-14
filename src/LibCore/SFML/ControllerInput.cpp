//
// Created by PinkySmile on 25/09/2021.
//

#include <map>
#include <cmath>
#include <fstream>
#include "ControllerInput.hpp"

namespace SpiralOfFate
{

	ControllerInput::ControllerInput() :
		ControllerInput({
			{ INPUT_LEFT,    new ControllerAxis(-1, sf::Joystick::Axis::X, -30) },
			{ INPUT_RIGHT,   new ControllerAxis(-1, sf::Joystick::Axis::X, 30) },
			{ INPUT_UP,      new ControllerAxis(-1, sf::Joystick::Axis::Y, -30) },
			{ INPUT_DOWN,    new ControllerAxis(-1, sf::Joystick::Axis::Y, 30) },
			{ INPUT_NEUTRAL, new ControllerButton(-1, 0) },
			{ INPUT_MATTER,  new ControllerButton(-1, 2) },
			{ INPUT_SPIRIT,  new ControllerButton(-1, 1) },
			{ INPUT_VOID,    new ControllerButton(-1, 3) },
			{ INPUT_ASCEND,  new ControllerAxis(-1, sf::Joystick::Axis::Z, 30) },
			{ INPUT_DASH,    new ControllerAxis(-1, sf::Joystick::Axis::Z, -30) },
			{ INPUT_PAUSE,   new ControllerButton(-1, 7) }
		})
	{
	}

	ControllerInput::ControllerInput(std::ifstream &stream)
	{
		std::map<InputEnum, std::pair<bool, int>> controllerMap{
			{ INPUT_LEFT,    {true,  (int)sf::Joystick::Axis::X | (256 - 30) << 3} },
			{ INPUT_RIGHT,   {true,  (int)sf::Joystick::Axis::X | 30 << 3} },
			{ INPUT_UP,      {true,  (int)sf::Joystick::Axis::Y | (256 - 30) << 3} },
			{ INPUT_DOWN,    {true,  (int)sf::Joystick::Axis::Y | 30 << 3} },
			{ INPUT_NEUTRAL, {false, 0} },
			{ INPUT_MATTER,  {false, 2} },
			{ INPUT_SPIRIT,  {false, 1} },
			{ INPUT_VOID,    {false, 3} },
			{ INPUT_ASCEND,  {true,  (int)sf::Joystick::Axis::Z | (30 << 3)} },
			{ INPUT_DASH,    {true,  (int)sf::Joystick::Axis::Z | ((256 - 30) << 3)} },
			{ INPUT_PAUSE,   {false, 7} }
		};
		std::unordered_map<InputEnum, ControllerKey *> realControllerMap;

		for (auto &[_, pair] : controllerMap)
			stream.read(reinterpret_cast<char *>(&pair), sizeof(pair));
		for (auto &[key, pair] : controllerMap)
			if (pair.first)
				realControllerMap.emplace(key, new ControllerAxis(-1, (sf::Joystick::Axis)(pair.second & 7), pair.second >> 3));
			else
				realControllerMap.emplace(key, new ControllerButton(-1, pair.second));
		for (auto [key, value] : realControllerMap)
			this->_keyMap.emplace(key, value);
		this->_keyDuration.fill(0);
	}

	ControllerInput::ControllerInput(const std::unordered_map<InputEnum, ControllerKey *> &keyMap)
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
		auto pDisconnected = event.getIf<sf::Event::JoystickDisconnected>();

		if (pDisconnected && (pDisconnected->joystickId == this->_joystickId || this->_joystickId == -1U)) {
			this->_state = false;
			return;
		}

		if (auto e = event.getIf<sf::Event::JoystickButtonPressed>()) {
			if (e->joystickId != this->_joystickId && this->_joystickId != -1U)
				return;
			if (e->button != this->_buttonId)
				return;
			this->_state = true;
		}
		if (auto e = event.getIf<sf::Event::JoystickButtonReleased>()) {
			if (e->joystickId != this->_joystickId && this->_joystickId != -1U)
				return;
			if (e->button != this->_buttonId)
				return;
			this->_state = false;
		}
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
		auto pDisconnected = event.getIf<sf::Event::JoystickDisconnected>();

		if (pDisconnected && (pDisconnected->joystickId == this->_joystickId || this->_joystickId == UINT32_MAX)) {
			this->_state = 0;
			return;
		}

		auto moved = event.getIf<sf::Event::JoystickMoved>();

		if (!moved)
			return;
		if (moved->joystickId != this->_joystickId && this->_joystickId != UINT32_MAX)
			return;
		if (moved->axis != this->_axis)
			return;
		this->_state = moved->position;
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

		return "Axis " + axis[static_cast<int>(this->_axis)] + (this->_threshHold < 0 ? "-" : "+");
	}

	std::pair<bool, int> ControllerAxis::save()
	{
		return {true, static_cast<int>(this->_axis) | ((int)(char)(this->_threshHold)) << 3};
	}
}