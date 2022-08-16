//
// Created by PinkySmile on 05/03/2022.
//

#ifndef SOFGV_REPLAYINPUT_HPP
#define SOFGV_REPLAYINPUT_HPP


#include <thread>
#include <SFML/Network.hpp>
#include <array>
#include <list>
#include <bitset>
#include <deque>
#include "IInput.hpp"
#include "Resources/BattleManager.hpp"

namespace SpiralOfFate
{
	class ReplayInput : public IInput {
	private:
		size_t _totalTime = 0;
		std::deque<Character::ReplayData> _inputs;
		std::bitset<INPUT_NUMBER - 1> _keyStates;
		std::array<int, INPUT_NUMBER - 1> _keyDuration;

		void _fillStates();

	public:
		ReplayInput(const std::deque<Character::ReplayData> &inputs);
		bool isPressed(InputEnum input) const override;
		InputStruct getInputs() const override;
		void update() override;
		void consumeEvent(const sf::Event &event) override;
		std::string getName() const override;
		std::vector<std::string> getKeyNames() const override;
		bool hasData() const;
		size_t getRemainingTime() const;
	};
}



#endif //SOFGV_REPLAYINPUT_HPP
