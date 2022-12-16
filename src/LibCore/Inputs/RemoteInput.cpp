//
// Created by PinkySmile on 15/09/2022.
//

#include "RemoteInput.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	RemoteInput::RemoteInput(IConnection &connection) :
		_connection(connection)
	{
		this->_keyStates.reset();
		this->_keyDuration.fill(0);
		this->_connection.registerInput(this);
	}

	RemoteInput::~RemoteInput()
	{
		this->_connection.unregisterInput(this);
	}

	void RemoteInput::update()
	{
		my_assert(!this->_inputBuffer.empty());
		this->_keyStates[INPUT_LEFT] = this->_inputBuffer.front()._h < 0;
		this->_keyStates[INPUT_RIGHT] = this->_inputBuffer.front()._h > 0;
		this->_keyStates[INPUT_UP] = this->_inputBuffer.front()._v > 0;
		this->_keyStates[INPUT_DOWN] = this->_inputBuffer.front()._v < 0;
		this->_keyStates[INPUT_N] = this->_inputBuffer.front().n;
		this->_keyStates[INPUT_M] = this->_inputBuffer.front().m;
		this->_keyStates[INPUT_S] = this->_inputBuffer.front().s;
		this->_keyStates[INPUT_V] = this->_inputBuffer.front().v;
		this->_keyStates[INPUT_A] = this->_inputBuffer.front().a;
		this->_keyStates[INPUT_D] = this->_inputBuffer.front().d;
		this->_inputBuffer.pop_front();
		RollbackInput::update();
	}

	std::string RemoteInput::getName() const
	{
		return "Remote input";
	}

	bool RemoteInput::hasInputs()
	{
		return !this->_inputBuffer.empty();
	}

	void RemoteInput::refreshInputs()
	{
		for (auto &input : this->_connection.receive())
			this->_inputBuffer.push_back(input);
	}

	void RemoteInput::flush()
	{
		this->_inputBuffer.clear();
	}
}