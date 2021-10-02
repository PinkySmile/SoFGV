//
// Created by Gegel85 on 27/09/2021.
//

#ifndef BATTLE_REMOTEINPUT_HPP
#define BATTLE_REMOTEINPUT_HPP


#include <thread>
#include <SFML/Network.hpp>
#include <array>
#include "IInput.hpp"

namespace Battle
{
	class RemoteInput : public IInput {
	private:
		unsigned _frame = 0;
		std::array<bool, INPUT_NUMBER> _keyStates;
		std::array<int, INPUT_NUMBER> _keyDuration;
		sf::IpAddress _address;
		unsigned short _port;
		sf::UdpSocket _sock;
		//sf::TcpSocket _sock;
		//sf::TcpListener _listener;
		unsigned _recieved = 0;
		std::thread _networkThread;

		void _threadLoop();

	public:
		RemoteInput();
		~RemoteInput();
		sf::IpAddress &getAddress();
		sf::UdpSocket &getSock();
		//sf::TcpSocket &getSock();
		unsigned short &getPort();
		void host(unsigned short port);
		void connect(const sf::IpAddress &ip, unsigned short port);
		bool isPressed(InputEnum input) const override;
		InputStruct getInputs() const override;
		void receiveData();
		void update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_REMOTEINPUT_HPP
