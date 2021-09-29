//
// Created by Gegel85 on 27/09/2021.
//

#ifndef BATTLE_NETWORKINPUT_HPP
#define BATTLE_NETWORKINPUT_HPP


#include <SFML/Network/UdpSocket.hpp>
#include "IInput.hpp"
#include "RemoteInput.hpp"

namespace Battle
{
	class NetworkInput : public IInput {
	private:
		RemoteInput &_remote;
		std::unique_ptr<IInput> _real;

	public:
		NetworkInput(RemoteInput &remote, IInput *real);
		bool isPressed(InputEnum input) const override;
		InputStruct getInputs() const override;
		void sendInputs();
		void update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_NETWORKINPUT_HPP
