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
		assert_exp(!this->_delayBuffer.empty());

		auto input = this->_delayBuffer.front();

		this->_delayBuffer.pop_front();
		this->_keyStates[INPUT_LEFT] = input.horizontalAxis < 0;
		this->_keyStates[INPUT_RIGHT] = input.horizontalAxis > 0;
		this->_keyStates[INPUT_UP] = input.verticalAxis > 0;
		this->_keyStates[INPUT_DOWN] = input.verticalAxis < 0;
		this->_keyStates[INPUT_N] = input.n != 0;
		this->_keyStates[INPUT_M] = input.m != 0;
		this->_keyStates[INPUT_S] = input.s != 0;
		this->_keyStates[INPUT_V] = input.v != 0;
		this->_keyStates[INPUT_A] = input.a != 0;
		this->_keyStates[INPUT_D] = input.d != 0;
		RollbackInput::update();
		if (this->_delayBuffer.empty())
			this->fillBuffer();
	}

	void DelayInput::setDelay(unsigned int delay)
	{
		this->_delay = delay;
		while (this->_delay < this->_delayBuffer.size())
			this->_delayBuffer.pop_front();
		while (this->_delay > this->_delayBuffer.size())
			this->_delayBuffer.push_back({0, 0, 0, 0, 0, 0, 0, 0, 0});
	}

	bool DelayInput::hasInputs()
	{
		return !this->_delayBuffer.empty();
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

	void DelayInput::fillBuffer()
	{
		while (this->_input->hasInputs() && this->_delayBuffer.size() <= this->_delay) {
			this->_input->update();
			this->_delayBuffer.push_back(this->_input->getInputs());
		}
	}

	unsigned DelayInput::getDelay() const
	{
		return this->_delay;
	}

	size_t DelayInput::getBufferSize() const
	{
		return this->_delayBuffer.size();
	}
}