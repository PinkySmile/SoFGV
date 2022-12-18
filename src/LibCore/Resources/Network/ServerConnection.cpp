//
// Created by PinkySmile on 22/08/2022.
//

#include "ServerConnection.hpp"
#include "Resources/version.h"
#include "Resources/Game.hpp"
#include "Scenes/Network/ServerCharacterSelect.hpp"
#include "Scenes/Network/ServerInGame.hpp"

namespace SpiralOfFate
{
	ServerConnection::ServerConnection(const std::string &name)
	{
		this->_names.first = name;
	}

	ServerConnection::~ServerConnection()
	{
		this->_socket.unbind();
	}

	void ServerConnection::_handlePacket(Connection::Remote &remote, PacketOlleh &, size_t size)
	{
		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_OLLEH, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ServerConnection::_handlePacket(Connection::Remote &remote, PacketRedirect &, size_t size)
	{
		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_REDIRECT, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ServerConnection::_handlePacket(Connection::Remote &remote, PacketPunch &, size_t size)
	{
		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_PUNCH, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ServerConnection::_handlePacket(Connection::Remote &remote, PacketInitRequest &packet, size_t size)
	{
		int err = -1;
		PacketInitSuccess result{this->_names.first.c_str(), VERSION_STR};

		if (size != sizeof(packet))
			err = ERROR_SIZE_MISMATCH;
		else if (packet.spectator && !this->_playing)
			err = ERROR_GAME_NOT_STARTED;
		else if (packet.spectator && !this->spectatorEnabled)
			err = ERROR_SPECTATORS_DISABLED;
		else if (!packet.spectator && this->_playing && remote.connectPhase != 1)
			err = ERROR_GAME_ALREADY_STARTED;

		if (err != -1) {
			PacketError error{static_cast<unsigned int>(err), OPCODE_INIT_REQUEST, size};

			return this->_send(remote, &error, sizeof(error));
		}

		this->_send(remote, &result, sizeof(result));
		this->_playing = true;
		this->_names.second = std::string(packet.playerName, strnlen(packet.playerName, sizeof(packet.playerName)));
		if (remote.connectPhase == 0) {
			if (this->onConnection)
				this->onConnection(remote, packet);
			if (packet.spectator)
				return;
			this->_opponent = &remote;
			this->_currentMenu = MENUSTATE_LOADING_CHARSELECT;
			remote.connectPhase = 1 + packet.spectator;
			game->connection->nextGame();
			game->scene.reset(new LoadingScene([this](LoadingScene *me){
				me->setStatus("Loading assets...");
				auto result = new ServerCharacterSelect();

				this->_currentMenu = MENUSTATE_CHARSELECT;
				me->setStatus("Waiting for opponent to finish loading...");
				while (this->_opCurrentMenu != MENUSTATE_CHARSELECT)
					std::this_thread::sleep_for(std::chrono::milliseconds(5));
				return result;
			}));
		}
	}

	void ServerConnection::_handlePacket(Connection::Remote &remote, PacketInitSuccess &, size_t size)
	{
		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_INIT_SUCCESS, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ServerConnection::_handlePacket(Connection::Remote &remote, PacketDelayUpdate &, size_t size)
	{
		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_DELAY_UPDATE, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ServerConnection::_handlePacket(Connection::Remote &remote, PacketMenuSwitch &packet, size_t size)
	{
		if (remote.connectPhase != 1) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_MENU_SWITCH, size};

			this->_send(remote, &error, sizeof(error));
		}
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_MENU_SWITCH, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (packet.opMenuId != this->_currentMenu) {
			PacketMenuSwitch menuSwitch{this->_currentMenu, this->_opCurrentMenu};

			this->_send(*this->_opponent, &menuSwitch, sizeof(menuSwitch));
		}
		this->_opCurrentMenu = packet.menuId;
	}

	void ServerConnection::_handlePacket(Connection::Remote &remote, PacketSyncTest &packet, size_t size)
	{
		auto it = this->_states.find(packet.frameId);

		if (it == this->_states.end())
			return;

		if (it->second != packet.stateChecksum) {
			PacketError error{ERROR_DESYNC_DETECTED, OPCODE_SYNC_TEST, size};

			this->_send(remote, &error, sizeof(error));
		}
	}

	void ServerConnection::_handlePacket(Connection::Remote &remote, PacketState &, size_t size)
	{
		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_STATE, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ServerConnection::_handlePacket(Connection::Remote &remote, PacketReplay &, size_t size)
	{
		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_REPLAY, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ServerConnection::_handlePacket(Connection::Remote &remote, PacketGameStart &, size_t size)
	{
		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_GAME_START, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ServerConnection::_handlePacket(Connection::Remote &remote, PacketGameQuit &packet, size_t size)
	{
		if (remote.connectPhase != 1) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_MENU_SWITCH, size};

			this->_send(remote, &error, sizeof(error));
		}
		if (this->_currentMenu == MENUSTATE_INGAME)
			this->switchMenu(MENUSTATE_CHARSELECT);
	}

	LoadingScene *ServerConnection::getChrLoadingScreen()
	{
		game->connection->nextGame();
		return new LoadingScene([this](LoadingScene *me){
			me->setStatus("Loading assets...");
			auto result = new ServerCharacterSelect(this->p1chr, this->p2chr, this->p1pal, this->p2pal, this->stage, this->platformConfig);

			this->_currentMenu = MENUSTATE_CHARSELECT;
			me->setStatus("Waiting for opponent to finish loading...");
			while (this->_opCurrentMenu != MENUSTATE_CHARSELECT)
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			return result;
		});
	}

	void ServerConnection::switchMenu(unsigned int id, bool lock)
	{
		this->_sendBuffer.clear();
		this->_currentFrame = 0;
		this->_lastOpRecvFrame = 0;
		this->_nextExpectedFrame = 0;
		this->_states.clear();
		if (id == MENUSTATE_CHARSELECT && lock) {
			game->connection->nextGame();
			game->scene.reset(new LoadingScene([this](LoadingScene *me) {
				me->setStatus("Loading assets...");
				auto result = new ServerCharacterSelect();

				this->_currentMenu = MENUSTATE_CHARSELECT;
				me->setStatus("Waiting for opponent to finish loading...");
				while (this->_opCurrentMenu != MENUSTATE_CHARSELECT)
					std::this_thread::sleep_for(std::chrono::milliseconds(5));
				return result;
			}));
		} else if (id == MENUSTATE_INGAME) {
			auto tmp = game->scene;

			game->connection->nextGame();
			game->scene.reset(new LoadingScene([tmp, this](LoadingScene *me) {
				auto *scene = reinterpret_cast<ServerCharacterSelect *>(&*tmp);
				me->setStatus("Loading P1's character (" + scene->_entries[this->p1chr].name + ")");
				auto lchr = scene->_createCharacter(this->p1chr, this->p1pal, scene->_leftInput);
				me->setStatus("Loading P2's character (" + scene->_entries[this->p2chr].name + ")");
				auto rchr = scene->_createCharacter(this->p2chr, this->p2pal, scene->_rightInput);
				auto &lentry = scene->_entries[this->p1chr];
				auto &rentry = scene->_entries[this->p2chr];
				auto &licon = lentry.icon[this->p1pal];
				auto &ricon = rentry.icon[this->p2pal];
				auto &stageObj = scene->_stages[this->stage];

				scene->_localInput->flush(HARDCODED_CURRENT_DELAY);
				scene->_remoteInput->flush(HARDCODED_CURRENT_DELAY);
				game->battleRandom.seed(this->seed);
				me->setStatus("Creating scene...");
				auto result = new ServerInGame(
					scene->_remoteRealInput,
					{static_cast<unsigned>(this->stage), 0, static_cast<unsigned>(this->platformConfig)},
					stageObj.platforms[scene->_platform],
					stageObj,
					lchr,
					rchr,
					licon.textureHandle,
					ricon.textureHandle,
					lentry.entry,
					rentry.entry
				);

				this->_currentMenu = MENUSTATE_INGAME;
				me->setStatus("Waiting for opponent to finish loading...");
				while (this->_opCurrentMenu != MENUSTATE_INGAME)
					std::this_thread::sleep_for(std::chrono::milliseconds(5));
				return result;
			}));
		}
		this->_currentMenu = id + MENUSTATE_LOADING_OFFSET;

		PacketMenuSwitch menuSwitch{this->_currentMenu, this->_opCurrentMenu};

		this->_send(*this->_opponent, &menuSwitch, sizeof(menuSwitch));
		this->_send(*this->_opponent, &menuSwitch, sizeof(menuSwitch));
		this->_send(*this->_opponent, &menuSwitch, sizeof(menuSwitch));
	}

	void ServerConnection::reportChecksum(unsigned int checksum)
	{
		PacketSyncTest syncTest{checksum, this->_sendBuffer.back().first};

		this->_states[this->_sendBuffer.back().first] = checksum;
		this->_send(*this->_opponent, &syncTest, sizeof(syncTest));
	}

	void ServerConnection::startGame(unsigned int _seed, unsigned int _p1chr, unsigned int _p1pal, unsigned int _p2chr, unsigned int _p2pal, unsigned int _stage, unsigned int _platformConfig)
	{
		PacketGameStart gameStart{_seed, _p1chr, _p1pal, _p2chr, _p2pal, _stage, _platformConfig};

		this->seed = _seed;
		this->p1chr = _p1chr;
		this->p1pal = _p1pal;
		this->p2chr = _p2chr;
		this->p2pal = _p2pal;
		this->stage = _stage;
		this->platformConfig = _platformConfig;
		this->_send(*this->_opponent, &gameStart, sizeof(gameStart));
		this->switchMenu(MENUSTATE_INGAME, false);
	}

	void ServerConnection::update()
	{
		this->_checkOpponentState();
		Connection::update();
	}

	void ServerConnection::host(unsigned short port)
	{
		game->logger.info("Hosting on port " + std::to_string(port));
		this->_states.clear();
		this->_socket.bind(port);
		this->_terminated = false;
	}

	void ServerConnection::_checkOpponentState()
	{
		auto menu = this->_currentMenu;

		if (menu > MENUSTATE_LOADING_OFFSET)
			menu -= MENUSTATE_LOADING_OFFSET;
		if (this->_currentMenu != this->_opCurrentMenu) {
			if (menu == MENUSTATE_INGAME && this->_opCurrentMenu != MENUSTATE_INGAME && this->_opCurrentMenu != MENUSTATE_LOADING_INGAME) {
				PacketGameStart gameStart{this->seed, this->p1chr, this->p1pal, this->p2chr, this->p2pal, this->stage, this->platformConfig};

				this->_send(*this->_opponent, &gameStart, sizeof(gameStart));
			} else {
				PacketMenuSwitch menuSwitch{this->_currentMenu, this->_opCurrentMenu};

				this->_send(*this->_opponent, &menuSwitch, sizeof(menuSwitch));
			}
		}
	}
}