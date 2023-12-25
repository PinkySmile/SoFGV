//
// Created by PinkySmile on 22/08/2022.
//

#include "SpectatorConnection.hpp"
#include "Resources/version.h"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketOlleh &packet, size_t size)
	{
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_OLLEH, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (remote.connectPhase == CONNECTION_STATE_CONNECTING) {
			PacketInitRequest request{this->_names.second.c_str(), VERSION_STR, true};

			return this->_send(remote, &request, sizeof(request));
		}

		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_OLLEH, size};

		this->_send(remote, &error, sizeof(error));
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketRedirect &, size_t size)
	{
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_REDIRECT, size};

		this->_send(remote, &error, sizeof(error));
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketPunch &, size_t size)
	{
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_PUNCH, size};

		this->_send(remote, &error, sizeof(error));
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketInitRequest &, size_t size)
	{
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_PUNCH, size};

		this->_send(remote, &error, sizeof(error));
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketInitSuccess &packet, size_t size)
	{
		if (remote.connectPhase == 0) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_INIT_SUCCESS, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_INIT_SUCCESS, size};

			return this->_send(remote, &error, sizeof(error));
		}

		auto args = new SpectatorArguments();

		remote.connectPhase = CONNECTION_STATE_HOST_NODE;
		this->_opponent = &remote;
		this->_names.first = std::string(packet.player1Name, strnlen(packet.player1Name, sizeof(packet.player1Name)));
		this->_names.second = std::string(packet.player2Name, strnlen(packet.player2Name, sizeof(packet.player2Name)));
		args->connection = this;
		game->scene.switchScene("spectator_char_select", args);
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketMenuSwitch &packet, size_t size)
	{
		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_MENU_SWITCH, size};

		this->_send(remote, &error, sizeof(error));
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketState &, size_t size)
	{
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_STATE, size};

		this->_send(remote, &error, sizeof(error));
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketReplay &packet, size_t size)
	{
		if (remote.connectPhase != CONNECTION_STATE_HOST_NODE) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_REPLAY, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (size < sizeof(packet) || size != packet.getSize()) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_REPLAY, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (packet.gameId != this->_gameId || (packet.frameId > packet.lastFrameId && packet.lastFrameId)) {
			PacketError error{ERROR_INVALID_DATA, OPCODE_REPLAY, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (!this->onReplayData)
			return;
		if (packet.nbInputs != 0)
			this->onReplayData(packet);
		this->_gameEnded = packet.lastFrameId && packet.lastFrameId <= packet.frameId + packet.nbInputs;
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketGameStart &packet, size_t size)
	{
		if (remote.connectPhase != CONNECTION_STATE_HOST_NODE) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_GAME_START, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_GAME_START, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (!this->_gameEnded && game->scene.getCurrentScene().first == "spectator_in_game")
			return;

		this->_startParams.seed = packet.seed;
		this->_startParams.p1chr = packet.p1chr;
		this->_startParams.p1pal = packet.p1pal;
		this->_startParams.p2chr = packet.p2chr;
		this->_startParams.p2pal = packet.p2pal;
		this->_startParams.stage = packet.stage;
		this->_startParams.platformConfig = packet.platformConfig;

		auto args = new InGameArguments();

		args->startParams = this->_startParams;
		args->connection = this;
		args->currentScene = game->scene.getCurrentScene().second;
		game->scene.switchScene("spectator_in_game", args, true);
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketReplayList &packet, size_t size)
	{
		if (remote.connectPhase != CONNECTION_STATE_HOST_NODE) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_REPLAY_LIST, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (size < sizeof(packet) || size != packet.getSize()) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_REPLAY_LIST, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (!this->_gameEnded || packet.nbEntries == 0)
			return;

		auto old = this->_gameId;

		this->_gameId = 0;
		for (unsigned i = 0; i < packet.nbEntries; i++)
			this->_gameId = std::max(this->_gameId, packet.gameIds[i]);
		if (this->_gameId == old)
			return;
		if (game->scene.getCurrentScene().first == "spectator_in_game") {
			auto args = new SpectatorArguments();

			args->connection = this;
			game->scene.switchScene("spectator_char_select", args);
		}
		this->_gameEnded = false;
	}

	void SpectatorConnection::update()
	{
		if (this->_terminated)
			return;

		static unsigned counter = 0;

		counter++;
		Connection::update();
		if (this->_remotes.empty())
			return this->terminate();
		if (this->_remotes.size() != 1)
			return;

		auto &op = this->_remotes.back();

		if (op.connectPhase == CONNECTION_STATE_CONNECTING) {
			if (counter % 10 != 0)
				return;
			PacketHello hello{REAL_VERSION_STR, op.ip.toInteger(), op.port};

			return this->_send(this->_remotes.back(), &hello, sizeof(hello));
		}
		if (!this->_gameEnded)
			return;
		if (counter % 60 != 0)
			return;

		PacketReplayListRequest list;

		this->_send(this->_remotes.back(), &list, sizeof(list));
	}

	void SpectatorConnection::connect(sf::IpAddress ip, unsigned short port)
	{
		game->logger.info("Connecting to " + ip.toString() + " on port " + std::to_string(port));
		this->_remotes.emplace_back(*this, ip, port);

		auto &op = this->_remotes.back();

		op.connectPhase = CONNECTION_STATE_CONNECTING;
		op.onDisconnect = [this](Remote &This){
			this->onDisconnect(This);
			this->terminate();
		};
		this->_states.clear();
		this->_terminated = false;
	}

	void SpectatorConnection::requestInputs(unsigned int startFrame)
	{
		if (this->_gameEnded && startFrame == 0)
			return;

		PacketReplayRequest req{this->_gameId, startFrame, false};

		this->_send(this->_remotes.front(), &req, sizeof(req));
	}
}
