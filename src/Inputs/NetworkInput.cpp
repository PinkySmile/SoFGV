//
// Created by Gegel85 on 27/09/2021.
//

#include "NetworkInput.hpp"

namespace Battle
{
	NetworkInput::NetworkInput(RemoteInput &remote, std::shared_ptr<IInput> real) :
		_remote(remote),
		_real(real)
	{
	}

	bool NetworkInput::isPressed(InputEnum input) const
	{
		return this->_real->isPressed(input);
	}

	InputStruct NetworkInput::getInputs() const
	{
		return this->_real->getInputs();
	}

	void NetworkInput::update()
	{
		this->_real->update();
		this->sendInputs();
	}

	void NetworkInput::consumeEvent(const sf::Event &event)
	{
		this->_real->consumeEvent(event);
	}

	void NetworkInput::sendInputs()
	{
		struct {
			int horizontalAxis : 2;
			int verticalAxis : 2;
			bool n : 1;
			bool m : 1;
			bool s : 1;
			bool v : 1;
			bool a : 1;
		} packet;
		auto inputs = this->_real->getInputs();

		packet.horizontalAxis = (inputs.horizontalAxis > 0) - (inputs.horizontalAxis < 0);
		packet.verticalAxis = (inputs.verticalAxis > 0) - (inputs.verticalAxis < 0);
		packet.n = inputs.n != 0;
		packet.m = inputs.m != 0;
		packet.s = inputs.s != 0;
		packet.v = inputs.v != 0;
		packet.a = inputs.a != 0;
		//this->_remote.getSock().send(&packet, sizeof(packet));
		this->_remote.getSock().send(&packet, sizeof(packet), this->_remote.getAddress(), this->_remote.getPort());
	}

	std::string NetworkInput::getName() const
	{
		return "NetworkInput: " + this->_real->getName();
	}

	std::vector<std::string> NetworkInput::getKeyNames() const
	{
		return this->_real->getKeyNames();
	}
}