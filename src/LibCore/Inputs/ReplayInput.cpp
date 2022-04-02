//
// Created by Gegel85 on 05/03/2022.
//

#include <cstring>
#include "ReplayInput.hpp"
#include "../Logger.hpp"

namespace SpiralOfFate
{
	ReplayInput::ReplayInput(const std::deque<Character::ReplayData> &inputs) :
		_inputs(inputs)
	{
		this->_keyStates.reset();
		this->_keyDuration.fill(0);
		this->_fillStates();
		for (auto &input : this->_inputs)
			this->_totalTime += input.time + 1;
	}

	bool ReplayInput::isPressed(InputEnum input) const
	{
		if (input == INPUT_RIGHT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) > 0;
		if (input == INPUT_LEFT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) < 0;
		return this->_keyStates[input];
	}

	InputStruct ReplayInput::getInputs() const
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
			false
		};
	}

	void ReplayInput::update()
	{
		if (!this->_inputs.empty()) {
			this->_totalTime--;
			if (this->_inputs.front().time == 0)
				this->_inputs.pop_front();
			else
				this->_inputs.front().time--;
			this->_fillStates();
		}
		for (size_t i = 0; i < this->_keyStates.size(); i++)
			if (this->_keyStates[i])
				this->_keyDuration[i]++;
			else
				this->_keyDuration[i] = 0;
	}

	void ReplayInput::consumeEvent(const sf::Event &)
	{
	}

	std::string ReplayInput::getName() const
	{
		return "Replay input";
	}

	std::vector<std::string> ReplayInput::getKeyNames() const
	{
		return std::vector<std::string>{INPUT_NUMBER};
	}

	void ReplayInput::_fillStates()
	{
		if (this->_inputs.empty()) {
			this->_keyStates.reset();
			return;
		}
		this->_keyStates[INPUT_RIGHT] = this->_inputs.front()._h > 0;
		this->_keyStates[INPUT_LEFT] = this->_inputs.front()._h < 0;
		this->_keyStates[INPUT_UP] = this->_inputs.front()._v > 0;
		this->_keyStates[INPUT_DOWN] = this->_inputs.front()._v < 0;
		this->_keyStates[INPUT_N] = this->_inputs.front().n;
		this->_keyStates[INPUT_M] = this->_inputs.front().m;
		this->_keyStates[INPUT_S] = this->_inputs.front().s;
		this->_keyStates[INPUT_V] = this->_inputs.front().v;
		this->_keyStates[INPUT_A] = this->_inputs.front().a;
		this->_keyStates[INPUT_D] = this->_inputs.front().d;
	}

	bool ReplayInput::hasData() const
	{
		return !this->_inputs.empty();
	}

	size_t ReplayInput::getRemainingTime() const
	{
		return this->_totalTime;
	}
}
