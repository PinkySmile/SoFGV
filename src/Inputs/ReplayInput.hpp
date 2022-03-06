//
// Created by Gegel85 on 05/03/2022.
//

#ifndef BATTLE_REPLAYINPUT_HPP
#define BATTLE_REPLAYINPUT_HPP


#include <thread>
#include <SFML/Network.hpp>
#include <array>
#include <list>
#include <bitset>
#include <deque>
#include "IInput.hpp"
#include "../Resources/BattleManager.hpp"

namespace Battle
{
	class ReplayInput : public IInput {
	private:
		std::deque<ReplayData> _inputs;
		std::bitset<INPUT_NUMBER - 1> _keyStates;
		std::array<int, INPUT_NUMBER - 1> _keyDuration;

		void _fillStates();

	public:
		ReplayInput(const std::deque<ReplayData> &inputs);
		bool isPressed(InputEnum input) const override;
		InputStruct getInputs() const override;
		void update() override;
		void consumeEvent(const sf::Event &event) override;
		std::string getName() const override;
		std::vector<std::string> getKeyNames() const override;
	};
}



#endif //BATTLE_REPLAYINPUT_HPP
