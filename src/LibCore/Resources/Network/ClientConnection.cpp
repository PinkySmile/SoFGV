//
// Created by PinkySmile on 22/08/2022.
//

#include "ClientConnection.hpp"
#include "Resources/version.h"

namespace SpiralOfFate
{
	ClientConnection::ClientConnection(const std::string &name)
	{
		this->_names.second = name;
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketOlleh &, size_t size)
	{
		if (remote.connectPhase == 3) {
			PacketInitRequest request{this->_names.second.c_str(), VERSION_STR, false};

			this->_send(remote, &request, sizeof(request));
			return;
		}

		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_PUNCH, size};

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

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketGameFrame &packet, size_t size)
	{
		if (remote.connectPhase != 1) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_GAME_FRAME, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (size < sizeof(packet) || size != packet.nbInputs * sizeof(*packet.inputs) + sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_GAME_FRAME, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (this->_sendBuffer.front().first != packet.frameId)
			return;
		for (size_t i = 0; i < packet.nbInputs; i++) {
			this->_buffer.push_back(packet.inputs[i]);
			this->_sendBuffer.pop_front();
		}
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketInitRequest &packet, size_t size)
	{
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_PUNCH, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketInitSuccess &, size_t size)
	{
		PacketMenuSwitch menuSwitch{2};

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

	void ClientConnection::_handlePacket(Connection::Remote &remote, PacketGameStart &, size_t size)
	{
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_GAME_START, size};

		this->_send(remote, &error, sizeof(error));
	}

	void ClientConnection::reportChecksum(unsigned int checksum)
	{
		PacketSyncTest syncTest{checksum, this->_sendBuffer.back().first};

		this->_states[this->_sendBuffer.back().first] = checksum;
		this->_send(*this->_opponent, &syncTest, sizeof(syncTest));
	}
}