//
// Created by PinkySmile on 15/09/2022.
//

#include "DelayInput.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	DelayInput::DelayInput(std::shared_ptr<IInput> input) :
		_input(std::move(input))
	{
	}

	void DelayInput::update()
	{
		while (this->_input->hasInputs() && this->_delayBuffer.size() <= this->_delay) {
			this->_input->update();
			this->_delayBuffer.push_back(this->_input->getInputs());
		}
		my_assert(!this->_delayBuffer.empty());
		this->_keyStates[INPUT_LEFT] = this->_delayBuffer.front().horizontalAxis < 0;
		this->_keyStates[INPUT_RIGHT] = this->_delayBuffer.front().horizontalAxis > 0;
		this->_keyStates[INPUT_UP] = this->_delayBuffer.front().verticalAxis > 0;
		this->_keyStates[INPUT_DOWN] = this->_delayBuffer.front().verticalAxis < 0;
		this->_keyStates[INPUT_N] = this->_delayBuffer.front().n != 0;
		this->_keyStates[INPUT_M] = this->_delayBuffer.front().m != 0;
		this->_keyStates[INPUT_S] = this->_delayBuffer.front().s != 0;
		this->_keyStates[INPUT_V] = this->_delayBuffer.front().v != 0;
		this->_keyStates[INPUT_A] = this->_delayBuffer.front().a != 0;
		this->_keyStates[INPUT_D] = this->_delayBuffer.front().d != 0;
		this->_delayBuffer.pop_front();
		RollbackInput::update();
	}

	void DelayInput::setDelay(unsigned int delay)
	{
		this->_delay = delay;
		while (delay < this->_delayBuffer.size())
			this->_delayBuffer.pop_front();
		while (delay > this->_delayBuffer.size())
			this->_delayBuffer.push_back({0, 0, 0, 0, 0, 0, 0, 0, 0});
	}

	bool DelayInput::hasInputs()
	{
		return this->_input->hasInputs() || !this->_delayBuffer.empty();
	}

	void DelayInput::consumeEvent(const sf::Event &event)
	{
		this->_input->consumeEvent(event);
	}

	void DelayInput::flush(unsigned delay)
	{
		this->_delay = delay;
		this->_delayBuffer.clear();
		while (this->_delay > this->_delayBuffer.size())
			this->_delayBuffer.push_back({0, 0, 0, 0, 0, 0, 0, 0, 0});
	}
}