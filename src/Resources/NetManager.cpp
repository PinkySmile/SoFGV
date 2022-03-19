//
// Created by Gegel85 on 07/02/2022.
//

#ifdef _WIN32
#include <winsock.h>
#endif
#include "version.h"
#include "NetManager.hpp"
#include "Game.hpp"
#include "../Scenes/NetplayInGame.hpp"
#include "../Scenes/TitleScreen.hpp"
#include "../Logger.hpp"

namespace Battle
{
#ifdef _WIN32
	NetManager::NetManager()
	{
		this->_interruptedLogo.loadFromFile("assets/icons/netplay/disconnected.png");
		this->_interruptedSprite.setTexture(this->_interruptedLogo);
		this->_interruptedSprite.setPosition(550, -250);
		this->_leftInput = std::make_shared<RemoteInput>();
		this->_rightInput = std::make_shared<RemoteInput>();
	}

	void NetManager::_initGGPO(unsigned short port, unsigned int spectators, bool spectator, const std::string &host, unsigned short hostPort)
	{
#ifdef _WIN32
		WSADATA wd;
		WSAStartup(MAKEWORD(2, 2), &wd);
#endif
		GGPOErrorCode result;
		GGPOSessionCallbacks cb;

		game.logger.debug("Starting GGPO for " + std::to_string(spectators + 2) + " players.");
		assert(!this->_ggpoSession);
		/* fill in all callback functions */
		cb.begin_game = GGPONetplay::startGame;
		cb.save_game_state = GGPONetplay::saveState;
		cb.load_game_state = GGPONetplay::loadState;
		cb.log_game_state = GGPONetplay::logState;
		cb.free_buffer = GGPONetplay::freeBuffer;
		cb.advance_frame = GGPONetplay::updateGame;
		cb.on_event = GGPONetplay::onEvent;

		/* Start a new session */
		if (spectator)
			result = ggpo_start_spectating(&this->_ggpoSession, &cb, VERSION_STR, 2, sizeof(RemoteInput::_keyStates), port, host.c_str(), hostPort);
		else
			result = ggpo_start_session(&this->_ggpoSession, &cb, VERSION_STR, 2 + spectators, sizeof(RemoteInput::_keyStates), port);
		if (result)
			throw GGPOError(result);
		ggpo_set_disconnect_timeout(this->_ggpoSession, 4000);
		ggpo_set_disconnect_notify_start(this->_ggpoSession, 1000);
	}

	void NetManager::_initGGPOSyncTest()
	{
#ifdef _WIN32
		WSADATA wd;
#endif
		GGPOErrorCode result;
		GGPOSessionCallbacks cb;

		game.logger.debug("Starting GGPO sync test.");
#ifdef _WIN32
		WSAStartup(MAKEWORD(2, 2), &wd);
#endif
		assert(!this->_ggpoSession);
		/* fill in all callback functions */
		cb.begin_game = GGPONetplay::startGame;
		cb.save_game_state = GGPONetplay::saveState;
		cb.load_game_state = GGPONetplay::loadState;
		cb.log_game_state = GGPONetplay::logState;
		cb.free_buffer = GGPONetplay::freeBuffer;
		cb.advance_frame = GGPONetplay::updateGame;
		cb.on_event = GGPONetplay::onEvent;

		/* Start a new session */
		result = ggpo_start_synctest(&this->_ggpoSession, &cb, VERSION_STR, 2, sizeof(RemoteInput::_keyStates), 1);
		if (result)
			throw GGPOError(result);
		ggpo_set_disconnect_timeout(this->_ggpoSession, 4000);
		ggpo_set_disconnect_notify_start(this->_ggpoSession, 1000);
	}

	bool NetManager::_tickClient(
		Client &client,
		size_t index,
		sf::UdpSocket &sock,
		const std::function<void (const sf::IpAddress &, unsigned short)> &onDisconnect,
		const std::function<void (Client &, size_t, unsigned)> &pingUpdate
	)
	{
		size_t recvSize;
		Packet packet;

		while (true) {
			auto ip = client.addr;
			unsigned short port = client.port;
			auto status = sf::Socket::Done;

			for (size_t i = 0; i < this->_packetQueue.size(); i++) {
				auto &pack = this->_packetQueue[i];

				if (std::get<0>(pack) == ip && std::get<1>(pack)) {
					packet = std::get<2>(pack);
					recvSize = std::get<3>(pack);
					this->_packetQueue.erase(this->_packetQueue.begin() + i);
					goto found;
				}
			}
			status = sock.receive(&packet, sizeof(packet), recvSize, ip, port);

		found:
			if (!this->_host) {
				game.logger.info("Host canceled");
				return false;
			}
			if (client.pingClock2.getElapsedTime().asSeconds() >= 1) {
				packet.op = OPCODE_PING;
				packet.pingId = client.lastPingId++;
				game.logger.debug("Send PING to " + client.addr.toString() + ":" + std::to_string(client.port));
				sock.send(&packet, sizeof(packet), client.addr, client.port);
				client.pingClock2.restart();
				client.pingClock.restart();
			}
			if (status == sf::Socket::NotReady) {
				if (client.lastPingId - client.pingId > 5) {
					game.logger.debug("Too many unanswered pings: " + std::to_string(client.lastPingId) + " - " + std::to_string(client.pingId) + " > 5");
					onDisconnect(client.addr, client.port);
					return false;
				}
				return true;
			}
			if (recvSize != sizeof(Packet)) {
				game.logger.error("Bad packet");
				memset(&packet, 0, sizeof(packet));
				packet.op = OPCODE_ERROR;
				packet.error = ERROR_BAD_PACKET;
				sock.send(&packet, sizeof(packet), client.addr, client.port);
				continue;
			}
			if (packet.op == OPCODE_QUIT) {
				game.logger.error("Player left");
				onDisconnect(client.addr, client.port);
				return false;
			}
			if (packet.op == OPCODE_HELLO)
				continue;
			if (packet.op != OPCODE_PONG) {
				game.logger.error("Bad opcode (" + std::to_string(packet.op) + ")");
				packet.op = OPCODE_ERROR;
				packet.error = ERROR_UNEXPECTED_OPCODE;
				sock.send(&packet, sizeof(packet), client.addr, client.port);
				continue;
			}
			if (packet.pingId >= client.pingId) {
				if (packet.pingId == client.pingId) {
					auto ms = client.pingClock.restart().asMilliseconds();
					pingUpdate(client, index, ms);
				}
				client.pingId = client.lastPingId;
			}
			game.logger.debug("PONG! " + std::to_string(packet.pingId));
		}
		return true;
	}

	void NetManager::_tickClients(
		std::vector<std::shared_ptr<Client>> &otherClients,
		sf::UdpSocket &sock,
		const std::function<void (const sf::IpAddress &, unsigned short)> &onDisconnect,
		const std::function<void (Client &, size_t, unsigned)> &pingUpdate
	)
	{
		for (size_t i = 0; i < otherClients.size(); i++) {
			auto client = otherClients[i];

			if (!this->_tickClient(*client, i, sock, onDisconnect, pingUpdate)) {
				if (!this->_host)
					return;
				game.logger.debug("Disconnected.");
				otherClients.erase(otherClients.begin() + i);
				onDisconnect(client->addr, client->port);
				i--;
			}
		}
	}

	bool NetManager::_acceptClientConnection(
		std::shared_ptr<Client> &client,
		std::vector<std::shared_ptr<Client>> &otherClients,
		unsigned short port,
		bool spectator,
		sf::UdpSocket &sock,
		const std::function<void (const sf::IpAddress &, unsigned short)> &onDisconnect,
		const std::function<void (const sf::IpAddress &, unsigned short)> &onConnect,
		const std::function<void (Client &, size_t, unsigned)> &pingUpdate
	)
	{
		Packet packet;
		size_t recvSize;

		client = std::make_shared<Client>();
		sock.setBlocking(false);
		while (true) {
			sf::Socket::Status status;

			do {
				client->addr = sf::IpAddress::Any;
				client->port = port;
				status = sock.receive(&packet, sizeof(packet), recvSize, client->addr, client->port);
				if (!this->_host)
					return false;
				for (auto &cli : otherClients) {
					if (cli->addr == client->addr && cli->port == client->port) {
						this->_packetQueue.emplace_back(cli->addr, cli->port, packet, recvSize);
						status = sf::Socket::NotReady;
						goto found;
					}
				}
			found:
				this->_tickClients(otherClients, sock, onDisconnect, pingUpdate);
			} while (status == sf::Socket::NotReady);
			game.logger.info("Received packet from " + client->addr.toString() + ":" + std::to_string(client->port));
			if (recvSize != sizeof(Packet)) {
				game.logger.error("Bad packet");
				memset(&packet, 0, sizeof(packet));
				packet.op = OPCODE_ERROR;
				packet.error = ERROR_BAD_PACKET;
				sock.send(&packet, sizeof(packet), client->addr, client->port);
				continue;
			}
			if (packet.op != OPCODE_HELLO) {
				game.logger.error("Bad opcode (" + std::to_string(packet.op) + ")");
				packet.op = OPCODE_ERROR;
				packet.error = ERROR_UNEXPECTED_OPCODE;
				sock.send(&packet, sizeof(packet), client->addr, client->port);
				continue;
			}
			if (strncmp(VERSION_STR, packet.versionString, sizeof(packet.versionString)) != 0) {
				game.logger.error("Version mismatch (expected \"" VERSION_STR "\" but got \"" + std::string(packet.versionString) + "\")");
				packet.op = OPCODE_ERROR;
				packet.error = ERROR_VERSION_MISMATCH;
				sock.send(&packet, sizeof(packet), client->addr, client->port);
				continue;
			}
			packet.op = OPCODE_OLLEH;
			packet.spectator = spectator;
			sock.send(&packet, sizeof(packet), client->addr, client->port);
			onConnect(client->addr, client->port);
			break;
		}
		//sock.unbind();
		return true;
	}

	bool NetManager::_acceptSpectators(
		std::vector<std::shared_ptr<Client>> &clients,
		unsigned short port,
		sf::UdpSocket &sock,
		const std::function<void (const sf::IpAddress &, unsigned short)> &onDisconnect,
		const std::function<void (const sf::IpAddress &, unsigned short)> &onConnect,
		const std::function<void (Client &, size_t, unsigned)> &pingUpdate,
		unsigned spectatorCount
	)
	{
		std::shared_ptr<Client> tmp;
		bool restart = false;

		game.logger.debug("Waiting for " + std::to_string(spectatorCount) + " (" + std::to_string(clients.size()) + ")");
		while (clients.size() <= spectatorCount) {
			this->_acceptClientConnection(tmp, clients, port, false, sock, [&onDisconnect, &clients, &restart](const sf::IpAddress &addr, unsigned short port){
				onDisconnect(addr, port);
				if (clients[0]->addr == addr && clients[0]->port == port)
					restart = false;
			}, onConnect, pingUpdate);
			if (restart)
				return false;
			if (!this->_host)
				return false;
			clients.push_back(tmp);
		}
		return true;
	}

	void NetManager::host(
		unsigned short port,
		unsigned int spectators,
		const std::function<void (const sf::IpAddress &, unsigned short)> &onDisconnect,
		const std::function<void (const sf::IpAddress &, unsigned short)> &onConnect,
		const std::function<void (unsigned, unsigned)> &spectatorUpdate,
		const std::function<void (unsigned)> &pingUpdate
	)
	{
		Packet packet;
		bool restart;
		sf::UdpSocket sock;
		std::shared_ptr<Client> tmp;
		std::vector<std::shared_ptr<Client>> clients;
		std::vector<GGPOPlayer> ggpoPlayers{2 + spectators};

		this->_host = true;
		game.logger.info("Bind socket on port " + std::to_string(port));
		sock.bind(port);
	start:
		restart = false;
		do {
			if (!this->_host)
				return;
			this->_acceptClientConnection(tmp, clients, port, false, sock, onDisconnect, onConnect, [&pingUpdate](Client &, size_t index, unsigned ping){
				if (!index)
					pingUpdate(ping);
			});
			if (!this->_host)
				return;
			clients.push_back(tmp);
			spectatorUpdate(clients.size() - 1, spectators);
			packet.op = OPCODE_WAIT;
			packet.currentSpecs = clients.size() - 1;
			packet.maxSpec = spectators;
			for (auto &client : clients)
				sock.send(&packet, sizeof(packet), client->addr, client->port);
		} while (!this->_acceptSpectators(clients, port, sock, [&onDisconnect, &spectatorUpdate, &clients, spectators, &packet, &sock](const sf::IpAddress &ip, unsigned short port){
			spectatorUpdate(clients.size() - 1, spectators);
			packet.op = OPCODE_WAIT;
			packet.currentSpecs = clients.size() - 1;
			packet.maxSpec = spectators;
			for (auto &client : clients)
				sock.send(&packet, sizeof(packet), client->addr, client->port);
			onDisconnect(ip, port);
		}, [&spectatorUpdate, &clients, spectators, &packet, &sock](const sf::IpAddress &, unsigned short){
			spectatorUpdate(clients.size() - 1, spectators);
			packet.op = OPCODE_WAIT;
			packet.currentSpecs = clients.size();
			packet.maxSpec = spectators;
			for (auto &client : clients)
				sock.send(&packet, sizeof(packet), client->addr, client->port);
		}, [&pingUpdate](Client &, size_t index, unsigned ping){
			if (!index)
				pingUpdate(ping);
		}, spectators));

		this->_delay = 0;
		while (this->_delay == 0) {
			this->_tickClients(clients, sock, [&onDisconnect, &clients, &restart](const sf::IpAddress &addr, unsigned short port){
				onDisconnect(addr, port);
				if (clients[0]->addr == addr && clients[0]->port == port)
					restart = true;
			}, [&pingUpdate](Client &, size_t index, unsigned ping) {
				game.logger.debug(std::to_string(index) + " ping is " + std::to_string(ping) + "ms");
				if (!index)
					pingUpdate(ping);
			});
			if (!this->_host)
				return;
			if (restart) {
				packet.op = OPCODE_QUIT;
				for (auto &client : clients)
					sock.send(&packet, sizeof(packet), client->addr, client->port);
				clients.clear();
				goto start;
			}
		}
		this->_delay--;
		game.logger.debug("Starting game with " + std::to_string(this->_delay) + " frames of delay !");
		packet.op = OPCODE_GAME_START;
		packet.delay = this->_delay;
		for (auto &client : clients)
			sock.send(&packet, sizeof(packet), client->addr, client->port);

		sock.unbind();
		this->_initGGPO(port, spectators, false);
		game.logger.debug("Adding GGPO players.");
		ggpoPlayers[0].type = GGPO_PLAYERTYPE_LOCAL;
		ggpoPlayers[0].size = sizeof(ggpoPlayers[0]);
		ggpoPlayers[0].player_num = 1;
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[0], &this->_playerHandles[0]);
		ggpo_set_frame_delay(this->_ggpoSession, this->_playerHandles[0], this->_delay);

		ggpoPlayers[1].type = GGPO_PLAYERTYPE_REMOTE;
		ggpoPlayers[1].size = sizeof(ggpoPlayers[1]);
		ggpoPlayers[1].player_num = 2;
		ggpoPlayers[1].u.remote.port = 10800;
		game.logger.info("Fighting " + clients[0]->addr.toString());
		strcpy(ggpoPlayers[1].u.remote.ip_address, clients[0]->addr.toString().c_str());
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[1], &this->_playerHandles[1]);

		for (size_t i = 1; i < clients.size(); i++) {
			GGPOPlayerHandle trashCan;

			game.logger.info("Adding spectator " + clients[i]->addr.toString());
			ggpoPlayers[i + 1].type = GGPO_PLAYERTYPE_SPECTATOR;
			ggpoPlayers[i + 1].size = sizeof(ggpoPlayers[1 + i]);
			ggpoPlayers[i + 1].u.remote.port = 10800;
			strcpy(ggpoPlayers[i + 1].u.remote.ip_address, clients[i]->addr.toString().c_str());
			ggpo_add_player(this->_ggpoSession, &ggpoPlayers[1 + i], &trashCan);
		}

		game.logger.debug("All done!");
	}

	bool NetManager::spectate(const std::string &address, unsigned short port, const std::function<void (const sf::IpAddress &, unsigned short)> &onConnect, const std::function<void (unsigned, unsigned)> &spectatorUpdate)
	{
		//this->_initGGPO(myPort, 0);
		return this->connect(address, port, onConnect, spectatorUpdate);
	}

	bool NetManager::connect(const std::string &address, unsigned short port, const std::function<void (const sf::IpAddress &, unsigned short)> &onConnect, const std::function<void (unsigned, unsigned)> &spectatorUpdate)
	{
		bool spectator;
		Packet packet;
		size_t recvSize;
		sf::UdpSocket sock; // Sockets are temporary, GGPO is eternal
		GGPOPlayer ggpoPlayers[2];
		sf::IpAddress host;
		unsigned short hostPort;
		unsigned ctr = 0;
		auto attempts = 0;
		sf::Socket::Status status;

		this->_connect = true;
		sock.setBlocking(false);
		do {
			host = address;
			hostPort = port;
			packet.op = OPCODE_HELLO;
			strcpy(packet.versionString, VERSION_STR);
			sock.send(&packet, sizeof(packet), host, hostPort);
			game.logger.info("Sent HELLO to " + host.toString() + ":" + std::to_string(hostPort));
			if (!this->_connect)
				return false;
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			status = sock.receive(&packet, sizeof(packet), recvSize, host, hostPort);
			if (status != sf::Socket::NotReady)
				break;
			attempts++;
			if (attempts > 50)
				return false;
		} while (true);
		game.logger.info("Server responded with opcode " + std::to_string(packet.op));
		try {
			_checkPacket(packet, recvSize);
		} catch (std::exception &e) {
			game.logger.error(e.what());
			this->_connect = false;
			memset(&packet, 0, sizeof(packet));
			packet.op = OPCODE_QUIT;
			sock.send(&packet, sizeof(packet), host, hostPort);
			throw;
		}
		if (packet.op != OPCODE_OLLEH) {
			this->_connect = false;
			throw std::invalid_argument("Invalid opcode received (Received opcode " + std::to_string(packet.op) + " but expected opcode " + std::to_string(OPCODE_OLLEH) + ").");
		}
		spectator = packet.spectator;
		if (spectator)
			this->setInputs(nullptr, nullptr);
		onConnect(host, hostPort);
		while (true) {
			status = sock.receive(&packet, sizeof(packet), recvSize, host, hostPort);
			if (!this->_connect) {
				packet.op = OPCODE_QUIT;
				sock.send(&packet, sizeof(packet), host, hostPort);
				return false;
			}
			if (status == sf::Socket::NotReady) {
				if (ctr == 3000) {
					this->_connect = false;
					throw std::invalid_argument("Timed out");
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				ctr++;
				continue;
			}
			ctr = 0;
			try {
				_checkPacket(packet, recvSize);
			} catch (std::exception &e) {
				game.logger.error(e.what());
				this->_connect = false;
				packet.op = OPCODE_QUIT;
				sock.send(&packet, sizeof(packet), host, hostPort);
				throw;
			}
			if (packet.op == OPCODE_WAIT) {
				spectatorUpdate(packet.spectator, packet.maxSpec);
				continue;
			}
			if (packet.op == OPCODE_GAME_START) {
				this->_delay = packet.delay;
				break;
			}
			if (packet.op != OPCODE_PING)
				continue;
			packet.op = OPCODE_PONG;
			sock.send(&packet, sizeof(packet), host, hostPort);
		}

		if (!spectator) {
			game.logger.debug("Host selected " + std::to_string(this->_delay) + " frames of delay");
			ggpoPlayers[0].type = GGPO_PLAYERTYPE_REMOTE;
			ggpoPlayers[0].size = sizeof(ggpoPlayers[0]);
			ggpoPlayers[0].player_num = 1;
			strcpy(ggpoPlayers[0].u.remote.ip_address, host.toString().c_str());
			ggpoPlayers[0].u.remote.port = port;

			ggpoPlayers[1].type = GGPO_PLAYERTYPE_LOCAL;
			ggpoPlayers[1].size = sizeof(ggpoPlayers[1]);
			ggpoPlayers[1].player_num = 2;
			ggpo_set_frame_delay(this->_ggpoSession, this->_playerHandles[1], this->_delay);

			game.logger.info("Fighting " + host.toString());
			this->_initGGPO(10800, 0, false);
			game.logger.debug("Adding GGPO players.");
			ggpo_add_player(this->_ggpoSession, &ggpoPlayers[0], &this->_playerHandles[0]);
			ggpo_add_player(this->_ggpoSession, &ggpoPlayers[1], &this->_playerHandles[1]);
			game.logger.debug("All done!");
		} else
			this->_initGGPO(10800, 0, true, host.toString(), port);
		this->_connect = false;
		return true;
	}

	void NetManager::startSyncTest()
	{
		GGPOPlayer ggpoPlayers[2];

		ggpoPlayers[0].type = GGPO_PLAYERTYPE_LOCAL;
		ggpoPlayers[0].size = sizeof(ggpoPlayers[0]);
		ggpoPlayers[0].player_num = 1;

		ggpoPlayers[1].type = GGPO_PLAYERTYPE_LOCAL;
		ggpoPlayers[1].size = sizeof(ggpoPlayers[1]);
		ggpoPlayers[1].player_num = 2;

		this->_initGGPOSyncTest();
		game.logger.debug("Adding GGPO players.");
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[0], &this->_playerHandles[0]);
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[1], &this->_playerHandles[1]);
		ggpo_set_frame_delay(this->_ggpoSession, this->_playerHandles[0], 4);
		ggpo_set_frame_delay(this->_ggpoSession, this->_playerHandles[1], 4);
		game.logger.debug("All done!");
	}

	void NetManager::nextFrame()
	{
		std::bitset<INPUT_NUMBER - 1> inputs;
		GGPOErrorCode result;

		this->_timer++;
		try {
			if (this->_leftRealInput) {
				this->_leftRealInput->update();

				auto input = this->_leftRealInput->getInputs();

				inputs[INPUT_UP] = input.verticalAxis > 0;
				inputs[INPUT_DOWN] = input.verticalAxis < 0;
				inputs[INPUT_LEFT] = input.horizontalAxis < 0;
				inputs[INPUT_RIGHT] = input.horizontalAxis > 0;
				for (int i = 4; i < INPUT_NUMBER - 1; i++)
					inputs[i] = ((int *)&input)[i - 2] != 0;
#ifdef _DEBUG
				std::string str;

				str.reserve(INPUT_NUMBER - 1);
				for (int i = 0; i < INPUT_NUMBER - 1; i++)
					str += inputs[i] ? "1" : "0";

				game.logger.debug("Adding left inputs (" + str + ")");
#endif
				result = ggpo_add_local_input(this->_ggpoSession, this->_playerHandles[0], &inputs, sizeof(inputs));

				if (result)
					throw GGPOError(result);
			}
			if (this->_rightRealInput) {
				this->_rightRealInput->update();

				auto input = this->_rightRealInput->getInputs();

				inputs[INPUT_UP] = input.verticalAxis > 0;
				inputs[INPUT_DOWN] = input.verticalAxis < 0;
				inputs[INPUT_LEFT] = input.horizontalAxis < 0;
				inputs[INPUT_RIGHT] = input.horizontalAxis > 0;
				for (int i = 4; i < INPUT_NUMBER - 1; i++)
					inputs[i] = ((int *)&input)[i - 2] != 0;
#ifdef _DEBUG
				std::string str;

				str.reserve(INPUT_NUMBER - 1);
				for (int i = 0; i < INPUT_NUMBER - 1; i++)
					str += inputs[i] ? "1" : "0";

				game.logger.debug("Adding right inputs (" + str + ")");
#endif
				result = ggpo_add_local_input(this->_ggpoSession, this->_playerHandles[1], &inputs, sizeof(inputs));

				if (result)
					throw GGPOError(result);
			}
		} catch (GGPOError &e) {
			game.logger.warn("Inputs are being ignored: " + std::string(e.what()));
			ggpo_idle(this->_ggpoSession, 1000 / 60);
			return;
		}

		this->advanceState();
	}

	void NetManager::setInputs(std::shared_ptr<IInput> left, std::shared_ptr<IInput> right)
	{
		this->_leftRealInput = std::move(left);
		this->_rightRealInput = std::move(right);
	}

	void NetManager::endSession()
	{
		ggpo_close_session(this->_ggpoSession);
		this->_ggpoSession = nullptr;
#ifdef _WIN32
		WSACleanup();
#endif
	}

	void NetManager::beginSession()
	{
		std::ifstream stream{"assets/characters/list.json"};
		nlohmann::json json;

		stream >> json;

		auto scene = std::make_shared<NetplayCharacterSelect>();

		this->_netScene = &*scene;
		this->_characterSelect = scene;
		this->_scene = game.scene;
		game.scene = scene;
	}

	void NetManager::advanceState()
	{
		int flags;
		std::bitset<INPUT_NUMBER - 1> inputs[2];
		GGPOErrorCode result;

		try {
			result = ggpo_synchronize_input(this->_ggpoSession, &inputs, sizeof(inputs), &flags);
			if (result)
				throw GGPOError(result);
			if (flags) {
				auto screen = new TitleScreen(game.P1, game.P2);

				this->endSession();
				game.scene.reset(screen);
			}
			this->_leftInput->_keyStates = inputs[0];
			this->_rightInput->_keyStates = inputs[1];

			auto scene = this->_netScene->_realUpdate();
#ifdef _DEBUG
			std::string str;

			str.reserve(INPUT_NUMBER * 2);
			for (int i = 0; i < INPUT_NUMBER - 1; i++)
				str += this->_leftInput->_keyStates[i] ? "1" : "0";
			str += ", ";
			for (int i = 0; i < INPUT_NUMBER - 1; i++)
				str += this->_rightInput->_keyStates[i] ? "1" : "0";

			game.logger.debug("Frame advance (" + str + ")");
#endif

			if (scene) {
				auto netScene = dynamic_cast<NetplayScene *>(scene);
				auto characterSelect = dynamic_cast<NetplayCharacterSelect *>(scene);
				auto inGame = dynamic_cast<NetplayInGame *>(scene);

				assert(netScene);
				assert(characterSelect || inGame);
				if (characterSelect) {
					this->_characterSelect.reset(characterSelect);
					game.scene = this->_characterSelect;
				} else {
					this->_inGame.reset(inGame);
					game.scene = this->_inGame;
				}
				this->_netScene = netScene;
			}
			ggpo_advance_frame(this->_ggpoSession);
		} catch (GGPOError &e) {
			game.logger.warn("Frame skipped: " + std::string(e.what()));
		}
		ggpo_idle(this->_ggpoSession, 1000 / 60);
	}

	bool NetManager::isConnected() const
	{
		return this->_ggpoSession;
	}

	void NetManager::save(void **buffer, int *len)
	{
		this->_netScene->_saveState(nullptr, len);
		*len += sizeof(Data);
		*buffer = new char[*len];
		memset(*buffer, 0, *len);

		auto data = reinterpret_cast<Data *>(*buffer);

#ifdef _DEBUG
		game.logger.debug("Saving NetManager (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)data));
#endif
		data->battleRandom = game.battleRandom;
		data->isCharSelect = &*this->_characterSelect == this->_netScene;
		for (size_t i = 0; i < this->_leftInput->_keyDuration.size(); i++)
			data->leftInputs[i] = this->_leftInput->_keyDuration[i];
		for (size_t i = 0; i < this->_rightInput->_keyDuration.size(); i++)
			data->rightInputs[i] = this->_rightInput->_keyDuration[i];
		this->_netScene->_saveState(&data[1], nullptr);
	}

	void NetManager::load(void *buffer)
	{
		auto data = reinterpret_cast<Data *>(buffer);

		game.battleRandom = data->battleRandom;
		if (data->isCharSelect) {
			this->_netScene = &*this->_characterSelect;
			game.scene = this->_characterSelect;
		} else {
			this->_netScene = &*this->_inGame;
			game.scene = this->_inGame;
		}
		for (size_t i = 0; i < this->_leftInput->_keyDuration.size(); i++)
			this->_leftInput->_keyDuration[i] = data->leftInputs[i];
		for (size_t i = 0; i < this->_rightInput->_keyDuration.size(); i++)
			this->_rightInput->_keyDuration[i] = data->rightInputs[i];
		this->_netScene->_loadState(&data[1]);
	}

	void NetManager::free(void *buffer)
	{
		delete[] reinterpret_cast<char *>(buffer);
	}

	void NetManager::update()
	{
		this->_scene = game.scene;
	}

	void NetManager::consumeEvent(GGPOEvent *event)
	{
		switch (event->code) {
		case GGPO_EVENTCODE_CONNECTED_TO_PEER:
			break;
		case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
			break;
		case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
			break;
		case GGPO_EVENTCODE_RUNNING:
			break;
		case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
			this->endSession();
			break;
		case GGPO_EVENTCODE_TIMESYNC:
			break;
		case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
			this->_interrupted = true;
			break;
		case GGPO_EVENTCODE_CONNECTION_RESUMED:
			this->_interrupted = false;
			break;
		}
	}

	void NetManager::renderHUD()
	{
		if (!this->isConnected())
			return;
	}

	void NetManager::postRender()
	{
		sf::RectangleShape shape;

		if (!this->isConnected())
			return;
		shape.setSize({1100, 700});
		shape.setPosition(-50, -600);
		shape.setFillColor(sf::Color{0x00, 0x00, 0x00, 0x80});
		shape.setOutlineColor(sf::Color::Transparent);
		if (this->_interrupted) {
			this->_interruptedSprite.setTextureRect({0, static_cast<int>(this->_timer) / 60 % 2 * 164, 264, 164});
			game.screen->draw(shape);
			game.screen->draw(this->_interruptedSprite);
		}
	}

	void NetManager::consumeEvent(const sf::Event &event)
	{
		if (this->_leftRealInput)
			this->_leftRealInput->consumeEvent(event);
		if (this->_rightRealInput)
			this->_rightRealInput->consumeEvent(event);
	}

	void NetManager::cancelHost()
	{
		this->_host = false;
		this->_connect = false;
	}

	bool NetManager::isHosting()
	{
		return this->_host;
	}

	bool NetManager::isConnecting()
	{
		return this->_connect;
	}

	void NetManager::setDelay(unsigned int delay)
	{
		this->_delay = delay;
	}

	void NetManager::_checkPacket(const Packet &packet, size_t size)
	{
		if (size != sizeof(Packet))
			throw std::invalid_argument("Bad packet (Received " + std::to_string(size) + " bytes instead of " + std::to_string(sizeof(Packet)) + " bytes)");
		if (packet.op == OPCODE_ERROR) {
			const char * const errs[] = {
				"ERROR_VERSION_MISMATCH",
				"ERROR_BAD_PACKET",
				"ERROR_INVALID_OPCODE",
				"ERROR_UNEXPECTED_OPCODE"
			};

			throw std::invalid_argument("Server responded with error " + std::to_string(packet.error) + " (" + (packet.error <= ERROR_UNEXPECTED_OPCODE ? errs[packet.error] : "ERROR_UNKNOWN") + ").");
		}
#ifdef _DEBUG
		game.logger.debug("Received opcode " + std::to_string(packet.op));
#endif
	}

	GGPOError::GGPOError(GGPOErrorCode code) :
		_code(code)
	{
	}

	const char *GGPOError::what() const noexcept
	{
		switch (this->_code) {
		case GGPO_ERRORCODE_SUCCESS:
			return "Success";
		case GGPO_ERRORCODE_INVALID_SESSION:
			return "Invalid session";
		case GGPO_ERRORCODE_INVALID_PLAYER_HANDLE:
			return "Invalid player handle";
		case GGPO_ERRORCODE_PLAYER_OUT_OF_RANGE:
			return "Player out of range";
		case GGPO_ERRORCODE_PREDICTION_THRESHOLD:
			return "Prediction threshold hit";
		case GGPO_ERRORCODE_UNSUPPORTED:
			return "Operation not supported";
		case GGPO_ERRORCODE_NOT_SYNCHRONIZED:
			return "Not synchronized";
		case GGPO_ERRORCODE_IN_ROLLBACK:
			return "In rollback";
		case GGPO_ERRORCODE_INPUT_DROPPED:
			return "Input dropped";
		case GGPO_ERRORCODE_PLAYER_DISCONNECTED:
			return "Player disconnected";
		case GGPO_ERRORCODE_TOO_MANY_SPECTATORS:
			return "Too many spectator";
		case GGPO_ERRORCODE_INVALID_REQUEST:
			return "Invalid request";
		case GGPO_ERRORCODE_GENERAL_FAILURE:
		default:
			return "Unknown error";
		}
	}
#else
	void NetManager::_initGGPO(unsigned short, unsigned int, bool, const std::string &, unsigned short) {}
	void NetManager::_initGGPOSyncTest() {}
	void NetManager::_checkPacket(const NetManager::Packet &, size_t) {}
	NetManager::NetManager() {}
	void NetManager::startSyncTest() {}
	void NetManager::consumeEvent(const sf::Event &) {}
	void NetManager::consumeEvent(GGPOEvent *) {}
	bool NetManager::isConnected() const { return false; }
	void NetManager::beginSession() {}
	void NetManager::setInputs(std::shared_ptr<IInput>, std::shared_ptr<IInput>) {}
	bool NetManager::spectate(
		const std::string &,
		unsigned short,
		const std::function<void(const sf::IpAddress &, unsigned short)> &,
		const std::function<void(unsigned int, unsigned int)> &)
	{ return false; }
	bool NetManager::connect(
		const std::string &,
		unsigned short,
		const std::function<void(const sf::IpAddress &, unsigned short)> &,
		const std::function<void(unsigned int,unsigned int)> &)
	{ return false; }
	void NetManager::host(
		unsigned short,
		unsigned int,
		const std::function<void(const sf::IpAddress &, unsigned short)> &,
		const std::function<void(const sf::IpAddress &, unsigned short)> &,
		const std::function<void (unsigned, unsigned)> &,
		const std::function<void(unsigned int)> &)
	{}
	void NetManager::advanceState() {}
	void NetManager::nextFrame() {}
	void NetManager::endSession() {}
	void NetManager::setDelay(unsigned int) {}
	void NetManager::save(void **, int *) {}
	void NetManager::load(void *) {}
	void NetManager::update() {}
	void NetManager::renderHUD() {}
	void NetManager::postRender() {}
	void NetManager::cancelHost() {}
	bool NetManager::isHosting() { return false; }
	bool NetManager::isConnecting() { return false; }
	void NetManager::free(void *) {}
#endif
}
