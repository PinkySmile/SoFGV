//
// Created by PinkySmile on 22/08/2022.
//

#include "ServerConnection.hpp"
#include "Resources/version.h"
#include "Resources/Game.hpp"
#include "Scenes/Network/ServerCharacterSelect.hpp"

namespace SpiralOfFate
{
	ServerConnection::ServerConnection(const std::string &name, std::shared_ptr<IInput> localInput) :
		_localInput(std::move(localInput))
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

		this->_playing = true;
		this->_names.second = std::string(packet.playerName, strnlen(packet.playerName, sizeof(packet.playerName)));
		if (this->onConnection)
			this->onConnection(remote, packet);
		if (!packet.spectator)
			this->_opponent = &remote;
		remote.connectPhase = 1 + packet.spectator;

		PacketInitSuccess result{this->_names.first.c_str(), VERSION_STR};

		this->_send(remote, &result, sizeof(result));
		game->sceneMutex.lock();
		game->scene.reset(new ServerCharacterSelect(this->_localInput));
		game->sceneMutex.unlock();
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

	void ServerConnection::switchMenu(unsigned int id)
	{
		this->_currentMenu = id;
		this->_sendBuffer.clear();
		this->_states.clear();
		if (id == 2) {
			game->sceneMutex.lock();
			game->scene.reset(new ServerCharacterSelect(this->_localInput));
			game->sceneMutex.unlock();
		}
	}

	void ServerConnection::reportChecksum(unsigned int checksum)
	{
		PacketSyncTest syncTest{checksum, this->_sendBuffer.back().first};

		this->_states[this->_sendBuffer.back().first] = checksum;
		this->_send(*this->_opponent, &syncTest, sizeof(syncTest));
	}

	void ServerConnection::startGame(unsigned int _seed, unsigned int _p1chr, unsigned int _p1pal, unsigned int _p2chr, unsigned int _p2pal, unsigned int _stage, unsigned int _platformConfig)
	{
		this->seed = _seed;
		this->p1chr = _p1chr;
		this->p1pal = _p1pal;
		this->p2chr = _p2chr;
		this->p2pal = _p2pal;
		this->stage = _stage;
		this->platformConfig = _platformConfig;
		this->switchMenu(1);
	}

	bool ServerConnection::send(InputStruct &inputs)
	{
		if (this->_currentMenu != this->_opCurrentMenu) {
			if (this->_currentMenu == 1) {
				PacketGameStart gameStart{this->seed, this->p1chr, this->p1pal, this->p2chr, this->p2pal, this->stage, this->platformConfig};

				this->_send(*this->_opponent, &gameStart, sizeof(gameStart));
			} else {
				PacketMenuSwitch menuSwitch{this->_currentMenu};

				this->_send(*this->_opponent, &menuSwitch, sizeof(menuSwitch));
			}
			return false;
		}
		return Connection::send(inputs);
	}

	void ServerConnection::host(unsigned short port)
	{
		game->logger.info("Hosting on port " + std::to_string(port));
		this->_endThread = false;
		this->_states.clear();
		this->_socket.bind(port);
		this->_netThread = std::thread{&ServerConnection::_threadLoop, this};
	}
}