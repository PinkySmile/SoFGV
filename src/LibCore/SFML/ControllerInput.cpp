//
// Created by PinkySmile on 25/09/2021.
//

#include <map>
#include <cmath>
#include <fstream>
#include "ControllerInput.hpp"

namespace SpiralOfFate
{
#define DEFAULT_THRESHOLD 30

	static std::vector<std::string> axis{
		"X", "Y",
		"Z", "R",
		"U", "V",
		"PadX", "PadY"
	};

	ControllerInput::ControllerInput() :
		ControllerInput({
			{ INPUT_LEFT,    new ControllerDoubleAxis(-1, sf::Joystick::Axis::X, sf::Joystick::Axis::PovX, -DEFAULT_THRESHOLD) },
			{ INPUT_RIGHT,   new ControllerDoubleAxis(-1, sf::Joystick::Axis::X, sf::Joystick::Axis::PovX, DEFAULT_THRESHOLD) },
			{ INPUT_UP,      new ControllerDoubleAxis(-1, sf::Joystick::Axis::Y, sf::Joystick::Axis::PovY, -DEFAULT_THRESHOLD) },
			{ INPUT_DOWN,    new ControllerDoubleAxis(-1, sf::Joystick::Axis::Y, sf::Joystick::Axis::PovY, DEFAULT_THRESHOLD) },
			{ INPUT_NEUTRAL, new ControllerButton(-1, 0) },
			{ INPUT_MATTER,  new ControllerButton(-1, 2) },
			{ INPUT_SPIRIT,  new ControllerButton(-1, 1) },
			{ INPUT_VOID,    new ControllerButton(-1, 3) },
			{ INPUT_ASCEND,  new ControllerAxis(-1, sf::Joystick::Axis::Z, DEFAULT_THRESHOLD) },
		#ifdef _WIN32
			{ INPUT_DASH,    new ControllerAxis(-1, sf::Joystick::Axis::Z, -DEFAULT_THRESHOLD) },
		#else
			{ INPUT_DASH,    new ControllerAxis(-1, sf::Joystick::Axis::R, DEFAULT_THRESHOLD) },
		#endif
			{ INPUT_PAUSE,   new ControllerButton(-1, 7) }
		})
	{
	}

	ControllerInput::ControllerInput(std::ifstream &stream)
	{
		std::map<InputEnum, std::pair<char, int>> controllerMap{
			{ INPUT_LEFT,    {2, (int)sf::Joystick::Axis::X | (int)sf::Joystick::Axis::PovX << 3 | (256 - DEFAULT_THRESHOLD) << 6} },
			{ INPUT_RIGHT,   {2, (int)sf::Joystick::Axis::X | (int)sf::Joystick::Axis::PovX << 3 | DEFAULT_THRESHOLD << 6} },
			{ INPUT_UP,      {2, (int)sf::Joystick::Axis::Y | (int)sf::Joystick::Axis::PovY << 3 | (256 - DEFAULT_THRESHOLD) << 6} },
			{ INPUT_DOWN,    {2, (int)sf::Joystick::Axis::Y | (int)sf::Joystick::Axis::PovY << 3 | DEFAULT_THRESHOLD << 6} },
			{ INPUT_NEUTRAL, {0, 0} },
			{ INPUT_MATTER,  {0, 2} },
			{ INPUT_SPIRIT,  {0, 1} },
			{ INPUT_VOID,    {0, 3} },
			{ INPUT_ASCEND,  {1, (int)sf::Joystick::Axis::Z | (DEFAULT_THRESHOLD << 3)} },
		#ifdef _WIN32
			{ INPUT_DASH,    {1, (int)sf::Joystick::Axis::Z | ((256 - DEFAULT_THRESHOLD) << 3)} },
		#else
			{ INPUT_DASH,    {1, (int)sf::Joystick::Axis::R | (DEFAULT_THRESHOLD << 3)} },
		#endif
			{ INPUT_PAUSE,   {0, 7} }
		};
		std::unordered_map<InputEnum, ControllerKey *> realControllerMap;

		for (auto &[_, pair] : controllerMap) {
			stream.read(reinterpret_cast<char *>(&pair.first),  sizeof(pair.first));
			stream.read(reinterpret_cast<char *>(&pair.second), sizeof(pair.second));
		}
		for (auto &[key, pair] : controllerMap)
			if (pair.first == 2)
				realControllerMap.emplace(key, new ControllerDoubleAxis(
					-1,
					static_cast<sf::Joystick::Axis>(pair.second & 7),
					static_cast<sf::Joystick::Axis>((pair.second >> 3) & 7),
					pair.second >> 6
				));
			else if (pair.first == 1)
				realControllerMap.emplace(key, new ControllerAxis(
					-1,
					static_cast<sf::Joystick::Axis>(pair.second & 7),
					pair.second >> 3
				));
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
		std::map<SpiralOfFate::InputEnum, std::pair<char, int>> controllerMap;

		for (auto &pair : this->_keyMap)
			controllerMap[pair.first] = pair.second->save();
		for (auto &pair : controllerMap) {
			stream.write(reinterpret_cast<char *>(&pair.second.first),  sizeof(pair.second.first));
			stream.write(reinterpret_cast<char *>(&pair.second.second), sizeof(pair.second.second));
		}
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

	std::pair<char, int> ControllerButton::save()
	{
		return {0, this->_buttonId};
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
		return "Axis " + axis[static_cast<int>(this->_axis)] + (this->_threshHold < 0 ? "-" : "+");
	}

	std::pair<char, int> ControllerAxis::save()
	{
		return {1, static_cast<int>(this->_axis) | ((int)(char)(this->_threshHold)) << 3};
	}

	ControllerDoubleAxis::ControllerDoubleAxis(unsigned int joystickId, sf::Joystick::Axis axis1, sf::Joystick::Axis axis2, float threshHold) :
		_joystickId(joystickId),
		_threshHold(threshHold),
		axis1(axis1),
		axis2(axis2)
	{
	}

	bool ControllerDoubleAxis::isPressed()
	{
		return std::copysign(1, this->_state) == std::copysign(1, this->_threshHold) &&
		       std::abs(this->_state) >= std::abs(this->_threshHold);
	}

	void ControllerDoubleAxis::consumeEvent(const sf::Event &event)
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
		if (moved->axis != this->axis1 && moved->axis != this->axis2)
			return;
		this->_state = moved->position;
	}

	void ControllerDoubleAxis::setJoystickId(unsigned int id)
	{
		this->_joystickId = id;
	}

	std::string ControllerDoubleAxis::toString()
	{
		return "Axis " + axis[static_cast<int>(this->axis1)] + "/" + axis[static_cast<int>(this->axis2)] + (this->_threshHold < 0 ? "-" : "+");
	}

	std::pair<char, int> ControllerDoubleAxis::save()
	{
		return {2, static_cast<int>(this->axis1) | static_cast<int>(this->axis2) << 3 | ((int)(char)(this->_threshHold)) << 6};
	}
}