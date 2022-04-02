//
// Created by Gegel85 on 27/09/2021.
//

#ifndef SOFGV_REMOTEINPUT_HPP
#define SOFGV_REMOTEINPUT_HPP


#include <thread>
#include <SFML/Network.hpp>
#include <array>
#include <bitset>
#include "IInput.hpp"

namespace SpiralOfFate
{
	class RemoteInput : public IInput {
	public:
		std::bitset<INPUT_NUMBER - 1> _keyStates;
		std::array<int, INPUT_NUMBER - 1> _keyDuration;
		int _paused = 0;

		RemoteInput();
		bool isPressed(InputEnum input) const override;
		InputStruct getInputs() const override;
		void update() override;
		void consumeEvent(const sf::Event &event) override;
		std::string getName() const override;
		std::vector<std::string> getKeyNames() const override;
	};
}


#endif //SOFGV_REMOTEINPUT_HPP
