//
// Created by PinkySmile on 20/08/2022.
//

#ifndef SOFGV_CONNECTION_HPP
#define SOFGV_CONNECTION_HPP


#include <thread>
#include <SFML/Network.hpp>
#include <vector>
#include "Packet.hpp"
#include "Inputs/InputEnum.hpp"

namespace SpiralOfFate
{
	class Connection {
	protected:
		struct Remote {
			sf::IpAddress ip;
			unsigned short port;
			int connectPhase = 0;
			std::vector<std::pair<unsigned, time_t>> pingsSent;
			unsigned pingTimeLast = 0;
			unsigned pingTimePeak = 0;
			std::vector<time_t> pingsReceived;

			Remote(const sf::IpAddress &ip, unsigned short port) : ip(ip), port(port) {};
		};

		bool _endThread = false;
		sf::Thread _netThread{&Connection::_threadLoop, this};
		unsigned _delay;
		unsigned _expectedDelay;
		unsigned _currentFrame;
		std::vector<InputStruct> _buffer;
		std::list<std::pair<unsigned, PacketInput>> _sendBuffer;
		sf::UdpSocket _socket;
		std::vector<Remote> _remotes;

		void _send(Remote &remote, void *packet, size_t size);

		virtual void _handlePacket(Remote &remote, PacketHello &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketOlleh &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketRedirect &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketPunch &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketPing &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketPong &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketGameFrame &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketInitRequest &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketInitSuccess &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketError &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketDelayUpdate &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketMenuSwitch &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketSyncTest &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketState &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketReplay &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketQuit &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketGameStart &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketGameStarted &packet, size_t size);
		virtual void _handlePacket(Remote &remote, Packet &packet, size_t size);
		void _threadLoop();

	public:
		std::vector<std::string> blacklist;
		std::function<void (Remote &remote, const PacketError &e)> onError;

		Connection();
		virtual ~Connection() = default;
		void updateDelay(unsigned int delay);
		void send(InputStruct &inputs);
		unsigned int getCurrentDelay();
		std::vector<InputStruct> receive();
		void terminate();
	};
}


#endif //SOFGV_CONNECTION_HPP
