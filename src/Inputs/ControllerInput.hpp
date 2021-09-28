//
// Created by Gegel85 on 25/09/2021.
//

#ifndef BATTLE_CONTROLLERINPUT_HPP
#define BATTLE_CONTROLLERINPUT_HPP


#include <map>
#include <array>
#include "IInput.hpp"

namespace Battle
{
	class ControllerKey {
	public:
		virtual bool isPressed() = 0;
		virtual void consumeEvent(const sf::Event &event) = 0;
	};

	class ControllerButton : public ControllerKey {
	private:
		unsigned _joystickId;
		unsigned _buttonId;
		bool _state = false;

	public:
		ControllerButton(unsigned joystickId, unsigned buttonId);
		bool isPressed() override;
		void consumeEvent(const sf::Event &event) override;
	};

	class ControllerAxis : public ControllerKey {
	private:
		unsigned _joystickId;
		float _threshHold;
		float _state = 0;
		sf::Joystick::Axis _axis;

	public:
		ControllerAxis(unsigned joystickId, sf::Joystick::Axis axis, float threshHold);
		bool isPressed() override;
		void consumeEvent(const sf::Event &event) override;
	};

	class ControllerInput : public IInput {
	private:
		std::map<InputEnum, std::unique_ptr<ControllerKey>> _keyMap;
		std::array<int, INPUT_NUMBER> _keyDuration;

	public:
		ControllerInput(const std::map<InputEnum, ControllerKey *> &keyMap);
		bool isPressed(InputEnum input) const override;
		InputStruct getInputs() const override;
		void update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_CONTROLLERINPUT_HPP
