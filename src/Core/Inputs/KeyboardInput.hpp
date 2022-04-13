//
// Created by Gegel85 on 25/09/2021.
//

#ifndef BATTLE_KEYBOARDINPUT_HPP
#define BATTLE_KEYBOARDINPUT_HPP


#include <map>
#include <array>
#include <SFML/Window/Keyboard.hpp>
#include "IInput.hpp"

namespace SpiralOfFate
{
	class KeyboardInput : public IInput {
	private:
		std::map<sf::Keyboard::Key, InputEnum> _keyMap;
		std::array<bool, INPUT_NUMBER> _keyStates;
		std::array<int, INPUT_NUMBER> _keyDuration;

	public:
		KeyboardInput(const std::map<sf::Keyboard::Key, InputEnum> &keyMap);
		bool isPressed(InputEnum input) const override;
		InputStruct getInputs() const override;
		void update() override;
		void consumeEvent(const sf::Event &event) override;
		void changeInput(InputEnum input, sf::Keyboard::Key key);
		void save(std::ofstream &stream) const;
		std::string getName() const override;
		std::vector<std::string> getKeyNames() const override;
	};
}


#endif //BATTLE_KEYBOARDINPUT_HPP
