//
// Created by PinkySmile on 22/08/2022.
//

#include "ServerConnection.hpp"
#include "Resources/version.h"
#include "Resources/Game.hpp"

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
		else if (remote.connectPhase != CONNECTION_STATE_CONNECTING)
			err = ERROR_UNEXPECTED_OPCODE;
		else if (packet.spectator && !this->_playing)
			err = ERROR_GAME_NOT_STARTED;
		else if (packet.spectator && !this->spectatorEnabled)
			err = ERROR_SPECTATORS_DISABLED;
		else if (!packet.spectator && this->_playing)
			err = ERROR_GAME_ALREADY_STARTED;

		if (err != -1) {
			PacketError error{static_cast<unsigned int>(err), OPCODE_INIT_REQUEST, size};

			return this->_send(remote, &error, sizeof(error));
		}

		this->_send(remote, &result, sizeof(result));
		this->_playing = true;
		this->_names.second = std::string(packet.playerName, strnlen(packet.playerName, sizeof(packet.playerName)));
		if (remote.connectPhase == CONNECTION_STATE_CONNECTING) {
			if (this->onConnection)
				this->onConnection(remote, packet);
			remote.connectPhase = packet.spectator ? CONNECTION_STATE_SPECTATOR : CONNECTION_STATE_PLAYER;
			if (packet.spectator)
				return;
			this->_opponent = &remote;
			this->_currentMenu = MENUSTATE_LOADING_CHARSELECT;
			this->nextGame();

			auto args = new CharSelectArguments();

			args->restore = false;
			args->connection = this;
			game->scene.switchScene("server_char_select", args);
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
		if (remote.connectPhase != CONNECTION_STATE_PLAYER) {
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

	void ServerConnection::_handlePacket(Connection::Remote &remote, PacketError &packet, size_t size)
	{
		if (
			remote.connectPhase == CONNECTION_STATE_PLAYER &&
			packet.code == ERROR_UNEXPECTED_OPCODE &&
			packet.offendingPacket == OPCODE_MENU_SWITCH &&
			packet.offendingPacketSize == sizeof(PacketMenuSwitch)
		) {
			// In this case, the init success packet has probably been dropped, so we send it again
			PacketInitSuccess result{this->_names.first.c_str(), VERSION_STR};

			this->_send(remote, &result, sizeof(result));
			return;
		}
		Connection::_handlePacket(remote, packet, size);
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
		if (remote.connectPhase != CONNECTION_STATE_PLAYER) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_MENU_SWITCH, size};

			this->_send(remote, &error, sizeof(error));
		}
		if (this->_currentMenu == MENUSTATE_INGAME)
			this->switchMenu(MENUSTATE_CHARSELECT);
	}

	void ServerConnection::switchToChrLoadingScreen()
	{
		this->switchMenu(MENUSTATE_CHARSELECT, false);
		this->nextGame();

		auto args = new CharSelectArguments();

		args->restore = true;
		args->connection = this;
		args->startParams = this->_startParams;
		game->scene.switchScene("server_char_select", args);
	}

	void ServerConnection::switchMenu(unsigned int id, bool lock)
	{
		this->_sendSyncBuffer.clear();
		this->_sendBuffer.clear();
		this->_currentFrame = 0;
		this->_nextExpectedFrame = 0;
		this->_nextExpectedDiffFrame = 0;
		this->_states.clear();
		if (id == MENUSTATE_CHARSELECT && lock) {
			this->nextGame();

			auto args = new CharSelectArguments();

			args->restore = false;
			args->connection = this;
			game->scene.switchScene("server_char_select", args);
		} else if (id == MENUSTATE_INGAME) {
			auto args = new InGameArguments();

			this->nextGame();
			args->startParams = this->_startParams;
			args->connection = this;
			args->currentScene = game->scene.getCurrentScene().second;
			game->scene.switchScene("server_in_game", args);
		}
		this->_currentMenu = id + MENUSTATE_LOADING_OFFSET;

		PacketMenuSwitch menuSwitch{this->_currentMenu, this->_opCurrentMenu};

		this->_send(*this->_opponent, &menuSwitch, sizeof(menuSwitch));
		this->_send(*this->_opponent, &menuSwitch, sizeof(menuSwitch));
		this->_send(*this->_opponent, &menuSwitch, sizeof(menuSwitch));
	}

	void ServerConnection::startGame(unsigned int _seed, unsigned int _p1chr, unsigned int _p1pal, unsigned int _p2chr, unsigned int _p2pal, unsigned int _stage, unsigned int _platformConfig)
	{
		PacketGameStart gameStart{_seed, _p1chr, _p1pal, _p2chr, _p2pal, _stage, _platformConfig};

		this->_startParams.seed = _seed;
		this->_startParams.p1chr = _p1chr;
		this->_startParams.p1pal = _p1pal;
		this->_startParams.p2chr = _p2chr;
		this->_startParams.p2pal = _p2pal;
		this->_startParams.stage = _stage;
		this->_startParams.platformConfig = _platformConfig;
		this->_send(*this->_opponent, &gameStart, sizeof(gameStart));
		this->switchMenu(MENUSTATE_INGAME, false);
	}

	void ServerConnection::update()
	{
		if (this->_terminated)
			return;
		this->_checkOpponentState();
		Connection::update();
	}

	void ServerConnection::host(unsigned short port)
	{
		game->logger.info("Hosting on port " + std::to_string(port));
		this->_states.clear();
		if (this->_socket.bind(port) != sf::Socket::Done) {
			game->logger.error("Failed to bind port.");
			return;
		}
		this->_terminated = false;
	}

	void ServerConnection::_checkOpponentState()
	{
		auto menu = this->_currentMenu;

		if (menu > MENUSTATE_LOADING_OFFSET)
			menu -= MENUSTATE_LOADING_OFFSET;
		if (this->_currentMenu != this->_opCurrentMenu) {
			if (menu == MENUSTATE_INGAME && this->_opCurrentMenu != MENUSTATE_INGAME && this->_opCurrentMenu != MENUSTATE_LOADING_INGAME) {
				PacketGameStart gameStart{
					this->_startParams.seed,
					this->_startParams.p1chr,
					this->_startParams.p1pal,
					this->_startParams.p2chr,
					this->_startParams.p2pal,
					this->_startParams.stage,
					this->_startParams.platformConfig
				};

				this->_send(*this->_opponent, &gameStart, sizeof(gameStart));
			} else {
				PacketMenuSwitch menuSwitch{this->_currentMenu, this->_opCurrentMenu};

				this->_send(*this->_opponent, &menuSwitch, sizeof(menuSwitch));
			}
		}
	}
}