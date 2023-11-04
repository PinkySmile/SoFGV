//
// Created by PinkySmile on 04/11/2023.
//

#ifndef SOFGV_CONTROLLERINPUT_HPP
#define SOFGV_CONTROLLERINPUT_HPP


#include <map>
#include <array>
#include <memory>
#include "Inputs/IInput.hpp"

namespace SpiralOfFate
{
	class ControllerKey {
	public:
		virtual ~ControllerKey() = default;
		virtual bool isPressed() = 0;
		virtual void consumeEvent(const sf::Event &event) = 0;
		virtual void setJoystickId(unsigned id) = 0;
		virtual std::string toString() = 0;
		virtual std::pair<bool, int> save() = 0;
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
		void setJoystickId(unsigned id) override;
		std::string toString() override;
		std::pair<bool, int> save() override;
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
		void setJoystickId(unsigned id) override;
		std::string toString() override;
		std::pair<bool, int> save() override;
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
		void setJoystickId(unsigned id);
		void changeInput(InputEnum input, ControllerKey *controller);
		void save(std::ofstream &stream) const;
		std::string getName() const override;
		std::vector<std::string> getKeyNames() const override;
	};
}


#endif //SOFGV_CONTROLLERINPUT_HPP
