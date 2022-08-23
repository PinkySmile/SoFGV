//
// Created by PinkySmile on 20/08/2022.
//

#ifndef SOFGV_CONNECTION_HPP
#define SOFGV_CONNECTION_HPP


#include <thread>
#include <SFML/Network.hpp>
#include <vector>
#include <mutex>
#include "Packet.hpp"
#include "Inputs/InputEnum.hpp"
#include "IConnection.hpp"

namespace SpiralOfFate
{
	class Connection : public IConnection {
	protected:
		class Remote {
		private:
			struct Ping {
				unsigned sequence = 0;
				sf::Clock clock;
			};

			Connection &base;

			void _pingLoop();

		public:
			sf::IpAddress ip;
			unsigned short port;
			volatile int connectPhase = 0;
			std::list<Ping> pingsSent;
			unsigned pingTimeLast = 0;
			unsigned pingTimePeak = 0;
			unsigned pingTimeSum = 0;
			unsigned pingLost = 0;
			std::list<unsigned> pingsReceived;
			sf::Thread pingThread{&Remote::_pingLoop, this};

			Remote(Connection &base, const sf::IpAddress &ip, unsigned short port) : base(base), ip(ip), port(port) {};
			~Remote();
		};

		std::mutex _mutex;
		bool _endThread = true;
		sf::Thread _netThread{&Connection::_threadLoop, this};
		unsigned _delay;
		unsigned _expectedDelay;
		unsigned _currentFrame = 0;
		Remote *_opponent = nullptr;
		std::vector<PacketInput> _buffer;
		std::list<std::pair<unsigned, PacketInput>> _sendBuffer;
		sf::UdpSocket _socket;
		std::list<Remote> _remotes;
		std::pair<std::string, std::string> _names;

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
		virtual void _handlePacket(Remote &remote, Packet &packet, size_t size);
		void _threadLoop();

	public:
		std::vector<std::string> blacklist;
		std::function<void (Remote &remote)> onDisconnect;
		std::function<void (Remote &remote, const PacketError &e)> onError;

		Connection();
		virtual ~Connection() = default;
		void updateDelay(unsigned int delay);
		virtual bool send(InputStruct &inputs);
		unsigned int getCurrentDelay();
		std::vector<PacketInput> receive();
		void terminate();
		bool isTerminated() const;
		const std::pair<std::string, std::string> &getNames() const;
	};
}


#endif //SOFGV_CONNECTION_HPP
