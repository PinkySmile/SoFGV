//
// Created by PinkySmile on 27/09/2021.
//

#ifndef SOFGV_ROLLBACKINPUT_HPP
#define SOFGV_ROLLBACKINPUT_HPP


#include <thread>
#include <SFML/Network.hpp>
#include <array>
#include <bitset>
#include "IInput.hpp"

namespace SpiralOfFate
{
	class RollbackInput : public IInput {
	public:
		std::bitset<INPUT_NUMBER - 1> _keyStates;
		std::array<int, INPUT_NUMBER - 1> _keyDuration;
		int _paused = 0;

		RollbackInput();
		bool isPressed(InputEnum input) const override;
		InputStruct getInputs() const override;
		void update() override;
		void consumeEvent(const sf::Event &event) override;
		std::string getName() const override;
		std::vector<std::string> getKeyNames() const override;
	};
}


#endif //SOFGV_ROLLBACKINPUT_HPP