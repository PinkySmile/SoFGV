//
// Created by PinkySmile on 25/12/23
//

#include "SpectatorInput.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	SpectatorInput::SpectatorInput(std::list<PacketInput> &list, unsigned *currentFrame) :
		_inputList(list),
		_currentFrame(currentFrame)
	{
	}

	void SpectatorInput::update()
	{
		my_assert(!this->_inputList.empty());

		auto &elem = this->_inputList.front();

		this->_inputList.pop_front();
		this->_keyStates[INPUT_LEFT] = elem._h < 0;
		this->_keyStates[INPUT_RIGHT] = elem._h > 0;
		this->_keyStates[INPUT_UP] = elem._v > 0;
		this->_keyStates[INPUT_DOWN] = elem._v < 0;
		this->_keyStates[INPUT_NEUTRAL] = elem.n;
		this->_keyStates[INPUT_SPIRIT] = elem.s;
		this->_keyStates[INPUT_MATTER] = elem.m;
		this->_keyStates[INPUT_VOID] = elem.v;
		this->_keyStates[INPUT_ASCEND] = elem.a;
		this->_keyStates[INPUT_DASH] = elem.d;
		RollbackInput::update();
		if (this->_currentFrame)
			(*this->_currentFrame)++;
	}
}
