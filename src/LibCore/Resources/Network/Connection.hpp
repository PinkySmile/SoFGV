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
#include "Resources/SceneArgument.hpp"

enum MenuStates {
	MENUSTATE_NOMENU,
	MENUSTATE_CHARSELECT,
	MENUSTATE_INGAME,
	MENUSTATE_LOADING_CHARSELECT,
	MENUSTATE_LOADING_INGAME,
};
#define MENUSTATE_LOADING_OFFSET (MENUSTATE_LOADING_CHARSELECT - MENUSTATE_CHARSELECT)

namespace SpiralOfFate
{
	class Connection : public IConnection {
	public:
		enum ConnectionState {
			CONNECTION_STATE_DISCONNECTED = -1,
			CONNECTION_STATE_NOT_INITIALIZED,
			CONNECTION_STATE_PLAYER,
			CONNECTION_STATE_SPECTATOR,
			CONNECTION_STATE_CONNECTING
		};

		struct GameStartParams {
			unsigned seed;
			unsigned p1chr;
			unsigned p1pal;
			unsigned p2chr;
			unsigned p2pal;
			unsigned stage;
			unsigned platformConfig;
		};

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
			volatile ConnectionState connectPhase = CONNECTION_STATE_NOT_INITIALIZED;
			std::list<Ping> pingsSent;
			unsigned pingTimeLast = 0;
			unsigned pingTimePeak = 0;
			unsigned pingTimeSum = 0;
			unsigned pingLost = 0;
			std::list<unsigned> pingsReceived;
			sf::Clock timeSinceLastPacket;
			std::thread pingThread;//{&Remote::_pingLoop, this};
			std::function<void (Remote &remote)> onDisconnect;

			Remote(Connection &base, const sf::IpAddress &ip, unsigned short port) : base(base), ip(ip), port(port) {};
			~Remote();
		};

	protected:
		GameStartParams _startParams;
		bool _terminated = true;
		unsigned _currentMenu = 0;
		unsigned _opCurrentMenu = 0;
		unsigned _delay = 0;
		unsigned _expectedDelay = 0;
		unsigned _currentFrame = 0;
		unsigned _nextExpectedFrame = 0;
		unsigned _lastOpRecvFrame = 0;
		unsigned _gameId = 0;
		std::map<unsigned, unsigned> _states;
		Remote *_opponent = nullptr;
		std::list<PacketInput> _buffer;
		std::list<std::pair<unsigned, PacketInput>> _sendBuffer;
		sf::UdpSocket _socket;
		std::list<Remote> _remotes;
		std::mutex _terminationMutex;
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
		virtual void _handlePacket(Remote &remote, PacketGameQuit &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketDesyncDetected &packet, size_t size);
		virtual void _handlePacket(Remote &remote, PacketTimeSync &packet, size_t size);
		virtual void _handlePacket(Remote &remote, Packet &packet, size_t size);

	public:
		struct TitleScreenArguments : public SceneArguments {
			std::string errorMessage;
		};
		struct InGameArguments : public SceneArguments {
			Connection *connection;
			GameStartParams startParams;
			class IScene *currentScene;
		};
		struct CharSelectArguments : public SceneArguments {
			Connection *connection;
			GameStartParams startParams;
			bool restore;
		};

		std::vector<std::string> blacklist;
		std::function<void (Remote &remote, unsigned frameId, unsigned cpuSum, unsigned recvSum)> onDesync;
		std::function<void (Remote &remote)> onDisconnect;
		std::function<void (unsigned newDelay)> onDelayUpdate;
		std::function<void (Remote &remote, const PacketError &e)> onError;

		Connection();
		~Connection() override;
		void waitForOpponent();
		void notifySwitchMenu();
		void quitGame();
		void updateDelay(unsigned int delay);
		virtual bool send(const InputStruct &inputs);
		unsigned int getCurrentDelay();
		std::list<PacketInput> receive();
		void terminate();
		bool isTerminated() const;
		void nextGame();
		virtual void update();
		const std::pair<std::string, std::string> &getNames() const;
		void reportChecksum(unsigned checksum, unsigned frameId);

#ifdef _DEBUG
		friend class NetworkInGame;
#endif
	};
}


#endif //SOFGV_CONNECTION_HPP
