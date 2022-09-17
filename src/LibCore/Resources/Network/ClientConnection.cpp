//
// Created by PinkySmile on 22/08/2022.
//

#include "Scenes/Network/ClientCharacterSelect.hpp"
#include "ClientConnection.hpp"
#include "Resources/version.h"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	ClientConnection::ClientConnection(const std::string &name, std::shared_ptr<IInput> localInput) :
		_localInput(localInput)
	{
		this->_names.second = name;
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketOlleh &packet, size_t size)
	{
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_OLLEH, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (remote.connectPhase != 0) {
			PacketInitRequest request{this->_names.second.c_str(), VERSION_STR, false};

			this->_send(remote, &request, sizeof(request));
			return;
		}

		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_OLLEH, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketRedirect &, size_t size)
	{
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_REDIRECT, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketPunch &, size_t size)
	{
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_PUNCH, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketInitRequest &packet, size_t size)
	{
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_PUNCH, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketInitSuccess &packet, size_t size)
	{
		if (remote.connectPhase == 0) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_INIT_SUCCESS, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_INIT_SUCCESS, size};

			return this->_send(remote, &error, sizeof(error));
		}

		PacketMenuSwitch menuSwitch{2};

		if (this->_currentMenu != 2) {
			remote.connectPhase = 1;
			this->_opponent = &remote;
			this->_currentMenu = 2;
			this->_opCurrentMenu = 2;
			this->_names.first = std::string(packet.playerName, strnlen(packet.playerName, sizeof(packet.playerName)));
			game->sceneMutex.lock();
			game->scene.reset(new ClientCharacterSelect(this->_localInput));
			game->sceneMutex.unlock();
			this->_sendMutex.lock();
			this->_sendBuffer.clear();
			this->_sendMutex.unlock();
		}
		this->_send(remote, &menuSwitch, sizeof(menuSwitch));
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketDelayUpdate &, size_t size)
	{
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_DELAY_UPDATE, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketMenuSwitch &packet, size_t size)
	{
		if (remote.connectPhase != 1) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_MENU_SWITCH, size};

			this->_send(remote, &error, sizeof(error));
		}
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_MENU_SWITCH, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (packet.menuId == 1)
			return;
		this->_opCurrentMenu = packet.menuId;
		//TODO: Switch scene
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketSyncTest &packet, size_t size)
	{
		auto it = this->_states.find(packet.frameId);

		if (it == this->_states.end())
			return;

		if (it->second != packet.stateChecksum) {
			PacketError error{ERROR_DESYNC_DETECTED, OPCODE_SYNC_TEST, size};

			this->_send(remote, &error, sizeof(error));
		}
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketState &, size_t size)
	{
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_STATE, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketReplay &, size_t size)
	{
		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_REPLAY, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketGameStart &packet, size_t size)
	{
		if (remote.connectPhase != 1) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_MENU_SWITCH, size};

			this->_send(remote, &error, sizeof(error));
		}

		PacketMenuSwitch menuSwitch{1};

		this->_send(remote, &menuSwitch, sizeof(menuSwitch));
		this->_opCurrentMenu = 1;
		//TODO: Switch scene
	}

	void ClientConnection::reportChecksum(unsigned int checksum)
	{
		PacketSyncTest syncTest{checksum, this->_sendBuffer.back().first};

		this->_states[this->_sendBuffer.back().first] = checksum;
		this->_send(*this->_opponent, &syncTest, sizeof(syncTest));
	}

	void ClientConnection::connect(sf::IpAddress ip, unsigned short port)
	{
		game->logger.info("Connecting to " + ip.toString() + " on port " + std::to_string(port));

		PacketHello hello{REAL_VERSION_STR, ip.toInteger(), port};

		this->_remotes.emplace_back(*this, ip, port);
		this->_remotes.back().connectPhase = 3;
		this->_send(this->_remotes.back(), &hello, sizeof(hello));
		this->_send(this->_remotes.back(), &hello, sizeof(hello));
		this->_send(this->_remotes.back(), &hello, sizeof(hello));
		this->_send(this->_remotes.back(), &hello, sizeof(hello));
		this->_send(this->_remotes.back(), &hello, sizeof(hello));
		this->_endThread = false;
		this->_states.clear();
		this->_netThread = std::thread{&ClientConnection::_threadLoop, this};
	}
}