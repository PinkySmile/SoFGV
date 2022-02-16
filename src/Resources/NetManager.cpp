//
// Created by Gegel85 on 07/02/2022.
//

#include <winsock.h>
#include "NetManager.hpp"
#include "Game.hpp"
#include "../Scenes/NetplayInGame.hpp"
#include "../Scenes/TitleScreen.hpp"
#include "../Logger.hpp"

#define VERSION_STR "stickman fighter alpha 0.0.0"

namespace Battle
{
	NetManager::NetManager()
	{
		this->_interruptedLogo.loadFromFile("assets/icons/netplay/disconnected.png");
		this->_interruptedSprite.setTexture(this->_interruptedLogo);
		this->_interruptedSprite.setPosition(550, -250);
		this->_leftInput = std::make_shared<RemoteInput>();
		this->_rightInput = std::make_shared<RemoteInput>();
	}

	void NetManager::_initGGPO(unsigned short port, unsigned int spectators)
	{
		WSADATA wd;
		GGPOErrorCode result;
		GGPOSessionCallbacks cb;

		logger.debug("Starting GGPO for " + std::to_string(spectators + 2) + " players.");
		WSAStartup(MAKEWORD(2, 2), &wd);
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
		result = ggpo_start_session(&this->_ggpoSession, &cb, "stickman_fighter_v0.0.1", 2 + spectators, sizeof(RemoteInput::_keyStates), port);
		if (result)
			throw GGPOError(result);
		ggpo_set_disconnect_timeout(this->_ggpoSession, 4000);
		ggpo_set_disconnect_notify_start(this->_ggpoSession, 1000);
	}

	void NetManager::_initGGPOSyncTest()
	{
		WSADATA wd;
		GGPOErrorCode result;
		GGPOSessionCallbacks cb;

		logger.debug("Starting GGPO sync test.");
		WSAStartup(MAKEWORD(2, 2), &wd);
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
		result = ggpo_start_synctest(&this->_ggpoSession, &cb, "stickman_fighter_v0.0.1", 2, sizeof(RemoteInput::_keyStates), 1);
		if (result)
			throw GGPOError(result);
		ggpo_set_disconnect_timeout(this->_ggpoSession, 4000);
		ggpo_set_disconnect_notify_start(this->_ggpoSession, 1000);
	}

	void NetManager::host(
		unsigned short port,
		unsigned int spectators,
		const std::function<void (const sf::IpAddress &, unsigned short)> &onDisconnect,
		const std::function<void (const sf::IpAddress &, unsigned short)> &onConnect,
		const std::function<void (unsigned)> &pingUpdate
	)
	{
		bool received = false;
		Packet packet;
		size_t recvSize;
		sf::Clock clock;
		sf::Clock clock2;
		sf::UdpSocket sock; // Sockets are temporary, GGPO is eternal
		sf::IpAddress tmp = sf::IpAddress::Any;
		unsigned short tmpPort = port;
		auto ggpoPlayers = new GGPOPlayer[2 + spectators];
		sf::IpAddress player;
		unsigned short playerPort;
		unsigned pingId = 0;
		unsigned lastPingId = 0;
		auto spectator = new sf::IpAddress[spectators];
		auto spectatorPort = new unsigned short[spectators];

		this->_host = true;
		logger.info("Bind socket on port " + std::to_string(port));
		sock.bind(port);
	start:
		sock.setBlocking(false);
		while (true) {
			sf::Socket::Status status;

			do {
				player = sf::IpAddress::Any;
				playerPort = port;
				status = sock.receive(&packet, sizeof(packet), recvSize, player, playerPort);
				if (!this->_host) {
					delete[] ggpoPlayers;
					delete[] spectator;
					delete[] spectatorPort;
					return;
				}
				if (status == sf::Socket::NotReady)
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
			} while (status == sf::Socket::NotReady);
			logger.info("Received packet from " + player.toString() + ":" + std::to_string(playerPort));
			if (recvSize != sizeof(Packet)) {
				logger.error("Bad packet");
				memset(&packet, 0, sizeof(packet));
				packet.op = OPCODE_ERROR;
				packet.error = ERROR_BAD_PACKET;
				sock.send(&packet, sizeof(packet), player, playerPort);
				continue;
			}
			if (packet.op == OPCODE_HELLO) {
				if (strncmp(VERSION_STR, packet.versionString, sizeof(packet.versionString)) == 0) {
					packet.op = OPCODE_OLLEH;
					packet.spectator = false;
					sock.send(&packet, sizeof(packet), player, playerPort);
					onConnect(player, playerPort);
					break;
				} else {
					logger.error("Version mismatch (expected " VERSION_STR " but got " + std::string(packet.versionString));
					packet.op = OPCODE_ERROR;
					packet.error = ERROR_VERSION_MISMATCH;
					sock.send(&packet, sizeof(packet), player, playerPort);
					continue;
				}
			} else {
				logger.error("Bad opcode (" + std::to_string(packet.op) + ")");
				packet.op = OPCODE_ERROR;
				packet.error = ERROR_UNEXPECTED_OPCODE;
				sock.send(&packet, sizeof(packet), player, playerPort);
				continue;
			}
		}
		this->_delay = 0;
		clock.restart();
		while (this->_delay == 0) {
			if (received) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000 - clock2.getElapsedTime().asMilliseconds()));
				lastPingId++;
				packet.op = OPCODE_PING;
				packet.pingId = lastPingId;
				sock.send(&packet, sizeof(packet), player, playerPort);
				received = false;
				clock2.restart();
				clock.restart();
			} else if (clock2.getElapsedTime().asSeconds() >= 1) {
				packet.op = OPCODE_PING;
				packet.pingId = lastPingId;
				sock.send(&packet, sizeof(packet), player, playerPort);
				clock2.restart();
			}
			tmp = player;
			tmpPort = playerPort;

			auto status = sock.receive(&packet, sizeof(packet), recvSize, tmp, tmpPort);

			if (!this->_host) {
				delete[] ggpoPlayers;
				delete[] spectator;
				delete[] spectatorPort;
				return;
			}
			if (status == sf::Socket::NotReady) {
				continue;
			}
			if (recvSize != sizeof(Packet)) {
				logger.error("Bad packet");
				memset(&packet, 0, sizeof(packet));
				packet.op = OPCODE_ERROR;
				packet.error = ERROR_BAD_PACKET;
				sock.send(&packet, sizeof(packet), player, playerPort);
				continue;
			}
			if (packet.op == OPCODE_QUIT) {
				logger.error("Player left");
				onDisconnect(player, playerPort);
				goto start;
			}
			if (packet.op != OPCODE_PONG) {
				logger.error("Bad packet");
				packet.op = OPCODE_ERROR;
				packet.error = ERROR_UNEXPECTED_OPCODE;
				sock.send(&packet, sizeof(packet), player, playerPort);
				continue;
			}
			if (packet.pingId >= pingId) {
				if (packet.pingId == pingId) {
					pingUpdate(clock.getElapsedTime().asMilliseconds());
					received = true;
				}
				pingId = lastPingId;
			}
			if (pingId - lastPingId > 20) {
				onDisconnect(player, playerPort);
				goto start;
			}
		}
		this->_delay--;
		logger.debug("Starting game with " + std::to_string(this->_delay) + " frames of delay !");
		packet.op = OPCODE_GAME_START;
		packet.delay = this->_delay;
		sock.send(&packet, sizeof(packet), player, playerPort);

		sock.unbind();
		ggpoPlayers[0].type = GGPO_PLAYERTYPE_LOCAL;
		ggpoPlayers[0].size = sizeof(ggpoPlayers[0]);
		ggpoPlayers[0].player_num = 1;

		ggpoPlayers[1].type = GGPO_PLAYERTYPE_REMOTE;
		ggpoPlayers[1].size = sizeof(ggpoPlayers[1]);
		ggpoPlayers[1].player_num = 2;
		ggpoPlayers[1].u.remote.port = 10800;
		strcpy(ggpoPlayers[1].u.remote.ip_address, player.toString().c_str());

		this->_initGGPO(port, spectators);
		logger.debug("Adding GGPO players.");
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[0], &this->_playerHandles[0]);
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[1], &this->_playerHandles[1]);
		ggpo_set_frame_delay(this->_ggpoSession, this->_playerHandles[0], this->_delay);
		logger.debug("All done!");
		delete[] ggpoPlayers;
		delete[] spectator;
		delete[] spectatorPort;
	}

	bool NetManager::spectate(const std::string &address, unsigned short port, const std::function<void (const sf::IpAddress &, unsigned short)> &onConnect, const std::function<void (unsigned, unsigned)> &spectatorUpdate)
	{
		//this->_initGGPO(myPort, 0);
		return this->connect(address, port, onConnect, spectatorUpdate);
	}

	bool NetManager::connect(const std::string &address, unsigned short port, const std::function<void (const sf::IpAddress &, unsigned short)> &onConnect, const std::function<void (unsigned, unsigned)> &spectatorUpdate)
	{
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
			logger.info("Sent HELLO to " + host.toString() + ":" + std::to_string(hostPort));
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
		logger.info("Server responded with opcode " + std::to_string(packet.op));
		try {
			_checkPacket(packet, recvSize);
		} catch (std::exception &e) {
			logger.error(e.what());
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
		onConnect(host, hostPort);
		while (true) {
			status = sock.receive(&packet, sizeof(packet), recvSize, host, hostPort);
			if (!this->_connect) {
				packet.op = OPCODE_QUIT;
				sock.send(&packet, sizeof(packet), host, hostPort);
				return false;
			}
			if (status == sf::Socket::NotReady) {
				if (ctr == 10) {
					this->_connect = false;
					throw std::invalid_argument("Timed out");
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				ctr++;
				continue;
			}
			ctr = 0;
			try {
				_checkPacket(packet, recvSize);
			} catch (std::exception &e) {
				logger.error(e.what());
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

		logger.debug("Host selected " + std::to_string(this->_delay) + " frames of delay");
		ggpoPlayers[0].type = GGPO_PLAYERTYPE_REMOTE;
		ggpoPlayers[0].size = sizeof(ggpoPlayers[0]);
		ggpoPlayers[0].player_num = 1;
		strcpy(ggpoPlayers[0].u.remote.ip_address, host.toString().c_str());
		ggpoPlayers[0].u.remote.port = port;

		ggpoPlayers[1].type = GGPO_PLAYERTYPE_LOCAL;
		ggpoPlayers[1].size = sizeof(ggpoPlayers[1]);
		ggpoPlayers[1].player_num = 2;

		this->_initGGPO(10800, 0);
		logger.debug("Adding GGPO players.");
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[0], &this->_playerHandles[0]);
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[1], &this->_playerHandles[1]);
		ggpo_set_frame_delay(this->_ggpoSession, this->_playerHandles[1], this->_delay);
		logger.debug("All done!");
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
		logger.debug("Adding GGPO players.");
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[0], &this->_playerHandles[0]);
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[1], &this->_playerHandles[1]);
		ggpo_set_frame_delay(this->_ggpoSession, this->_playerHandles[0], 4);
		ggpo_set_frame_delay(this->_ggpoSession, this->_playerHandles[1], 4);
		logger.debug("All done!");
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

				logger.debug("Adding left inputs (" + str + ")");
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

				logger.debug("Adding right inputs (" + str + ")");
#endif
				result = ggpo_add_local_input(this->_ggpoSession, this->_playerHandles[1], &inputs, sizeof(inputs));

				if (result)
					throw GGPOError(result);
			}
		} catch (GGPOError &e) {
			logger.warn("Inputs are being ignored: " + std::string(e.what()));
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
		WSACleanup();
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

			logger.debug("Frame advance (" + str + ")");
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
			logger.warn("Frame skipped: " + std::string(e.what()));
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
		logger.debug("Received opcode " + std::to_string(packet.op));
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
}