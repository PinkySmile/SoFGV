//
// Created by PinkySmile on 20/08/2022.
//

#ifndef SOFGV_CONNECTION_HPP
#define SOFGV_CONNECTION_HPP


#include <thread>
#include <SFML/Network.hpp>
#include <vector>
#include <mutex>
#include <functional>
#include "Packet.hpp"
#include "Inputs/InputEnum.hpp"
#include "IConnection.hpp"

namespace SpiralOfFate
{
	class Connection : public IConnection {
	public:
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
			std::thread pingThread;//{&Remote::_pingLoop, this}

			Remote(Connection &base, const sf::IpAddress &ip, unsigned short port) : base(base), ip(ip), port(port) {};
			~Remote();
		};

	protected:
		std::mutex _sendMutex;
		bool _endThread = true;
		std::thread _netThread;
		unsigned _delay;
		unsigned _expectedDelay;
		unsigned _currentFrame = 0;
		unsigned _nextExpectedFrame = 0;
		unsigned _lastOpRecvFrame = 0;
		Remote *_opponent = nullptr;
		std::list<PacketInput> _buffer;
		std::list<std::pair<unsigned, PacketInput>> _sendBuffer;
		sf::UdpSocket _socket;
		std::list<Remote> _remotes;
		std::pair<std::string, std::string> _names;

		void _send(Remote &remote, void *packet, uint32_t size);

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
		~Connection();
		void updateDelay(unsigned int delay);
		virtual bool send(const InputStruct &inputs);
		unsigned int getCurrentDelay();
		std::list<PacketInput> receive();
		void terminate();
		bool isTerminated() const;
		const std::pair<std::string, std::string> &getNames() const;
		virtual void reportChecksum(unsigned checksum);
	};
}


#endif //SOFGV_CONNECTION_HPP
