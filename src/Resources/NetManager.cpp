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
	enum Opcode {
		OPCODE_HELLO,
		OPCODE_OLLEH,
		OPCODE_ERROR,
		OPCODE_WAIT,
		OPCODE_START,
		OPCODE_QUIT,
	};

	enum ErrorCode {
		ERROR_VERSION_MISMATCH,
		ERROR_BAD_PACKET,
		ERROR_INVALID_OPCODE,
		ERROR_UNEXPECTED_OPCODE,
	};

	union Packet {
		Opcode op;
		struct {
			Opcode _;
			char versionString[32];
		};
		struct {
			Opcode __;
			ErrorCode error;
		};
		struct {
			Opcode ___;
			unsigned int currentSpecs;
			unsigned int maxSpec;
		};
	};

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

	bool NetManager::spectate(const std::string &address, unsigned short port)
	{
		//this->_initGGPO(myPort, 0);
		return false;
	}

	bool NetManager::connect(const std::string &address, unsigned short port)
	{
		Packet packet;
		size_t recvSize;
		sf::UdpSocket sock; // Sockets are temporary, GGPO is eternal
		GGPOPlayer ggpoPlayers[2];
		sf::IpAddress host = address;
		unsigned short hostPort = port;

		packet.op = OPCODE_HELLO;
		strcpy(packet.versionString, VERSION_STR);
		sock.send(&packet, sizeof(packet), host, hostPort);
		logger.info("Sent HELLO to " + host.toString() + ":" + std::to_string(hostPort));
		sock.receive(&packet, sizeof(packet), recvSize, host, hostPort);
		logger.info("Server responded with opcode " + std::to_string(packet.op));
		if (recvSize != sizeof(Packet)) {
			logger.error("Bad packet.");
			return false;
		}
		if (packet.op != OPCODE_OLLEH) {
			logger.error("Connection failed.");
			return false;
		}
		sock.setBlocking(false);
		sock.unbind();

		ggpoPlayers[0].type = GGPO_PLAYERTYPE_REMOTE;
		ggpoPlayers[0].size = sizeof(ggpoPlayers[0]);
		ggpoPlayers[0].player_num = 2;
		strcpy(ggpoPlayers[0].u.remote.ip_address, host.toString().c_str());
		ggpoPlayers[0].u.remote.port = hostPort;

		ggpoPlayers[1].type = GGPO_PLAYERTYPE_LOCAL;
		ggpoPlayers[1].size = sizeof(ggpoPlayers[1]);
		ggpoPlayers[1].player_num = 1;

		this->_initGGPO(hostPort, 0);
		logger.debug("Adding GGPO players.");
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[0], &this->_playerHandles[0]);
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[1], &this->_playerHandles[1]);
		logger.debug("All done!");
		return true;
	}

	void NetManager::host(unsigned short port, unsigned int spectators)
	{
		Packet packet;
		size_t recvSize;
		sf::UdpSocket sock; // Sockets are temporary, GGPO is eternal
		sf::IpAddress tmp = sf::IpAddress::Any;
		unsigned short tmpPort = port;
		GGPOPlayer ggpoPlayers[2 + spectators];
		sf::IpAddress player;
		unsigned short playerPort;
		sf::IpAddress spectator[spectators];
		unsigned short spectatorPort[spectators];

		logger.info("Bind socket on port " + std::to_string(port));
		sock.bind(port);
		while (true) {
			player = sf::IpAddress::Any;
			playerPort = port;
			sock.receive(&packet, sizeof(packet), recvSize, player, playerPort);
			logger.info("Received packet from " + player.toString() + ":" + std::to_string(playerPort));
			if (recvSize != sizeof(Packet)) {
				logger.error("Bad packet");
				packet.op = OPCODE_ERROR;
				packet.error = ERROR_BAD_PACKET;
				sock.send(&packet, sizeof(packet), player, playerPort);
				continue;
			}
			if (packet.op == OPCODE_HELLO) {
				if (strncmp(VERSION_STR, packet.versionString, sizeof(packet.versionString)) == 0) {
					packet.op = OPCODE_OLLEH;
					sock.send(&packet, sizeof(packet), player, playerPort);
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
		sock.setBlocking(false);
		sock.unbind();

		ggpoPlayers[0].type = GGPO_PLAYERTYPE_LOCAL;
		ggpoPlayers[0].size = sizeof(ggpoPlayers[0]);
		ggpoPlayers[0].player_num = 1;

		ggpoPlayers[1].type = GGPO_PLAYERTYPE_REMOTE;
		ggpoPlayers[1].size = sizeof(ggpoPlayers[2]);
		ggpoPlayers[1].player_num = 2;
		strcpy(ggpoPlayers[1].u.remote.ip_address, player.toString().c_str());
		ggpoPlayers[1].u.remote.port = playerPort;

		this->_initGGPO(playerPort, spectators);
		logger.debug("Adding GGPO players.");
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[0], &this->_playerHandles[0]);
		ggpo_add_player(this->_ggpoSession, &ggpoPlayers[1], &this->_playerHandles[1]);
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

				for (int i = 0; i < INPUT_NUMBER - 1; i++)
					inputs[i] = ((int *)&input)[i] != 0;
				result = ggpo_add_local_input(this->_ggpoSession, this->_playerHandles[0], &inputs, sizeof(inputs));

				if (result)
					throw GGPOError(result);
			}
			if (this->_rightRealInput) {
				this->_rightRealInput->update();

				auto input = this->_rightRealInput->getInputs();

				for (int i = 0; i < INPUT_NUMBER - 1; i++)
					inputs[i] = ((int *)&input)[i] != 0;
				result = ggpo_add_local_input(this->_ggpoSession, this->_playerHandles[1], &inputs, sizeof(inputs));

				if (result)
					throw GGPOError(result);
			}
		} catch (GGPOError &e) {
			logger.warn("Inputs are being ignored: " + std::string(e.what()));
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

		auto scene = std::make_shared<NetplayInGame>(
			new ACharacter{
				json[0]["framedata"],
				json[0]["subobjects"],
				std::pair<std::vector<Color>, std::vector<Color>>{{}, {}},
				game.networkMgr._leftInput
			},
			new ACharacter{
				json[0]["framedata"],
				json[0]["subobjects"],
				std::pair<std::vector<Color>, std::vector<Color>>{
					std::vector<Color>{Color{{0, 0, 0, 0xFF}}},
					std::vector<Color>{Color{{255, 0, 0, 0xFF}}}
				},
				game.networkMgr._rightInput
			},
			json[0],
			json[0]
		);

		this->_netScene = &*scene;
		this->_scene = scene;
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

			if (scene) {
				auto netScene = dynamic_cast<NetplayScene *>(scene);

				game.scene.reset(scene);
				assert(netScene);
				this->_netScene = netScene;
			}
			logger.debug("Frame advance");
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

		auto data = reinterpret_cast<Data *>(*buffer);

		data->isCharSelect = dynamic_cast<NetplayInGame *>(this->_netScene) != nullptr;
		data->leftInputs = this->_leftInput->_keyDuration;
		data->rightInputs = this->_rightInput->_keyDuration;
		this->_netScene->_saveState(&data[1], len);
	}

	void NetManager::load(void *buffer)
	{
		auto data = reinterpret_cast<Data *>(buffer);

		assert(data->isCharSelect == (dynamic_cast<NetplayInGame *>(this->_netScene) != nullptr));
		this->_leftInput->_keyDuration = data->leftInputs;
		this->_rightInput->_keyDuration = data->rightInputs;
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
			this->_interruptedSprite.setTextureRect({0, static_cast<int>(this->_timer) / 60 % 2 * 163, 264, 163});
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