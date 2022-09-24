//
// Created by PinkySmile on 15/09/2022.
//

#include "NetworkInput.hpp"

namespace SpiralOfFate
{
	NetworkInput::NetworkInput(IInput &input, Connection &connection) :
		_input(input),
		_connection(connection)
	{
	}

	bool NetworkInput::isPressed(InputEnum input) const
	{
		return this->_input.isPressed(input);
	}

	InputStruct NetworkInput::getInputs() const
	{
		return this->_input.getInputs();
	}

	void NetworkInput::update()
	{
		this->_input.update();
		this->_connection.send(this->getInputs());
	}

	void NetworkInput::consumeEvent(const sf::Event &event)
	{
		this->_input.consumeEvent(event);
	}

	std::string NetworkInput::getName() const
	{
		return "Network Input bundling " + this->_input.getName();
	}

	std::vector<std::string> NetworkInput::getKeyNames() const
	{
		return this->_input.getKeyNames();
	}
}