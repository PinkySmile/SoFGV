//
// Created by Gegel85 on 27/09/2021.
//

#include <cstring>
#include "RemoteInput.hpp"
#include "../Logger.hpp"

const unsigned char handshake[] = {0xA0, 0x98, 0x45, 0x12, 0x90, 0x04, 0xCD, 0xF4};

namespace Battle
{
	RemoteInput::RemoteInput()
	{
		this->_keyStates.fill(false);
		this->_keyDuration.fill(0);
	}

	RemoteInput::~RemoteInput()
	{

	}

	bool RemoteInput::isPressed(InputEnum input) const
	{
		if (input == INPUT_RIGHT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) > 0;
		if (input == INPUT_LEFT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) > 0;
		return this->_keyStates[input];
	}

	InputStruct RemoteInput::getInputs() const
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
		};
	}

	void RemoteInput::update()
	{
		this->receiveData();
		for (int i = 0; i < INPUT_NUMBER; i++)
			if (this->_keyStates[i])
				this->_keyDuration[i]++;
			else
				this->_keyDuration[i] = 0;
	}

	void RemoteInput::consumeEvent(const sf::Event &)
	{
	}

	sf::IpAddress &RemoteInput::getAddress()
	{
		return this->_address;
	}

	unsigned short &RemoteInput::getPort()
	{
		return this->_port;
	}

	void RemoteInput::host(unsigned short port)
	{
		unsigned char buffer[8];
		size_t received;

		logger.debug("Hosting on port " + std::to_string(port));
		//this->_listener.listen(port);
		//this->_listener.accept(this->_sock);
		this->_port = port;
		this->_address = sf::IpAddress::Any;
		//this->_sock.receive(buffer, sizeof(buffer), received);
		//logger.debug("Connection from " + this->_sock.getRemoteAddress().toString() + ":" + std::to_string(this->_sock.getRemotePort()));
		this->_sock.bind(port);
		this->_sock.receive(buffer, sizeof(buffer), received, this->_address, this->_port);
		logger.debug("Connection from " + this->_address.toString() + ":" + std::to_string(this->_port));
		if (received != 8)
			// TODO: Make custom exceptions
			throw std::invalid_argument("Invalid handshake");
		if (std::memcmp(handshake, buffer, sizeof(buffer)) != 0)
			// TODO: Make custom exceptions
			throw std::invalid_argument("Invalid handshake");
		this->_sock.send(buffer, sizeof(buffer), this->_address, this->_port);
		//this->_sock.send(buffer, sizeof(buffer));
		//this->_networkThread = std::thread();
	}

	void RemoteInput::connect(const sf::IpAddress &ip, unsigned short port)
	{
		unsigned char buffer[8];
		size_t received;

		this->_address = ip;
		this->_port = port;
		logger.debug("Connecting to " + this->_address.toString() + ":" + std::to_string(this->_port));
		//this->_sock.connect(ip, port);
		//this->_sock.send(handshake, sizeof(handshake));
		//this->_sock.receive(buffer, sizeof(buffer), received);
		this->_sock.send(handshake, sizeof(handshake), this->_address, this->_port);
		this->_sock.receive(buffer, sizeof(buffer), received, this->_address, this->_port);
		logger.debug("Response from " + this->_address.toString() + ":" + std::to_string(this->_port));
		if (received != 8)
			// TODO: Make custom exceptions
			throw std::invalid_argument("Invalid handshake");
		if (std::memcmp(handshake, buffer, sizeof(buffer)) != 0)
			// TODO: Make custom exceptions
			throw std::invalid_argument("Invalid handshake");
	}

	void RemoteInput::receiveData()
	{
		struct {
		//	unsigned frame;
			int horizontalAxis : 2;
			int verticalAxis : 2;
			bool n : 1;
			bool m : 1;
			bool s : 1;
			bool v : 1;
			bool a : 1;
		} packet;
		size_t received;

		this->_sock.receive(&packet, sizeof(packet), received, this->_address, this->_port);
		//this->_sock.receive(&packet, sizeof(packet), received);
		this->_keyStates[INPUT_LEFT] = packet.horizontalAxis < 0;
		this->_keyStates[INPUT_RIGHT] = packet.horizontalAxis > 0;
		this->_keyStates[INPUT_UP] = packet.verticalAxis > 0;
		this->_keyStates[INPUT_DOWN] = packet.verticalAxis < 0;
		this->_keyStates[INPUT_NEUTRAL] = packet.n;
		this->_keyStates[INPUT_MATTER] = packet.m;
		this->_keyStates[INPUT_SPIRIT] = packet.s;
		this->_keyStates[INPUT_VOID] = packet.v;
		this->_keyStates[INPUT_ASCEND] = packet.a;
	}

	void RemoteInput::_threadLoop()
	{

	}

	sf::UdpSocket &RemoteInput::getSock()
	//sf::TcpSocket &RemoteInput::getSock()
	{
		return this->_sock;
	}

	std::string RemoteInput::getName() const
	{
		return "Remote input at " + this->_address.toString();
	}

	std::vector<std::string> RemoteInput::getKeyNames() const
	{
		return std::vector<std::string>{INPUT_NUMBER};
	}
}
