//
// Created by PinkySmile on 22/08/2022.
//

#include "SpectatorConnection.hpp"
#include "Resources/version.h"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	SpectatorConnection::SpectatorConnection(const std::string &name)
	{
		this->_names.second = name;
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketOlleh &packet, size_t size)
	{
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_OLLEH, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (remote.connectPhase == 3) {
			PacketInitRequest request{this->_names.second.c_str(), VERSION_STR, false};

			this->_send(remote, &request, sizeof(request));
			return;
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

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketInitRequest &packet, size_t size)
	{
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_PUNCH, size};

		(void)packet;
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

		remote.connectPhase = CONNECTION_STATE_HOST_NODE;
		this->_opponent = &remote;
		this->_names.first = std::string(packet.player1Name, strnlen(packet.player1Name, sizeof(packet.player1Name)));
		this->_names.second = std::string(packet.player2Name, strnlen(packet.player2Name, sizeof(packet.player2Name)));
		game->scene.switchScene("spectator_char_select");
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketMenuSwitch &packet, size_t size)
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

			this->_send(remote, &menuSwitch, sizeof(menuSwitch));
		}
		this->_opCurrentMenu = packet.menuId;
		if (packet.menuId == MENUSTATE_INGAME || packet.menuId == MENUSTATE_LOADING_INGAME)
			return;

		auto menu = this->_currentMenu;

		if (menu > MENUSTATE_LOADING_OFFSET)
			menu -= MENUSTATE_LOADING_OFFSET;
		if (menu == packet.menuId || menu + MENUSTATE_LOADING_OFFSET == packet.menuId)
			return;

		PacketMenuSwitch pack{packet.menuId + MENUSTATE_LOADING_OFFSET, this->_opCurrentMenu};

		this->_send(remote, &pack, sizeof(pack));
		if (packet.menuId == MENUSTATE_CHARSELECT || packet.menuId == MENUSTATE_LOADING_CHARSELECT) {
			auto restore = this->_currentMenu == MENUSTATE_INGAME || this->_currentMenu == MENUSTATE_LOADING_INGAME;
			auto args = new CharSelectArguments();

			this->_currentMenu = MENUSTATE_LOADING_CHARSELECT;
			this->nextGame();
			args->restore = restore;
			args->startParams = this->_startParams;
			args->connection = this;
			game->scene.switchScene("client_char_select", args);
		}
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketState &, size_t size)
	{
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_STATE, size};

		this->_send(remote, &error, sizeof(error));
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketReplay &, size_t size)
	{
		PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_REPLAY, size};

		this->_send(remote, &error, sizeof(error));
	}

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketGameStart &packet, size_t size)
	{
		if (remote.connectPhase != 1) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_MENU_SWITCH, size};

			this->_send(remote, &error, sizeof(error));
		}

		this->_opCurrentMenu = MENUSTATE_LOADING_INGAME;
		if (this->_currentMenu == MENUSTATE_LOADING_INGAME || this->_currentMenu == MENUSTATE_INGAME) {
			PacketMenuSwitch menuSwitch{this->_currentMenu, this->_opCurrentMenu};

			this->_send(remote, &menuSwitch, sizeof(menuSwitch));
			return;
		}
		this->_currentMenu = MENUSTATE_LOADING_INGAME;
		this->_startParams.seed = packet.seed;
		this->_startParams.p1chr = packet.p1chr;
		this->_startParams.p1pal = packet.p1pal;
		this->_startParams.p2chr = packet.p2chr;
		this->_startParams.p2pal = packet.p2pal;
		this->_startParams.stage = packet.stage;
		this->_startParams.platformConfig = packet.platformConfig;

		PacketMenuSwitch menuSwitch{this->_currentMenu, this->_opCurrentMenu};
		auto args = new InGameArguments();

		args->startParams = this->_startParams;
		args->connection = this;
		args->currentScene = game->scene.getCurrentScene().second;

		this->_send(remote, &menuSwitch, sizeof(menuSwitch));
		this->nextGame();
		game->scene.switchScene("client_in_game", args);
	}

	void SpectatorConnection::update()
	{
		if (this->_terminated)
			return;

		Connection::update();
		if (this->_remotes.empty())
			return this->terminate();
		if (this->_remotes.size() != 1)
			return;

		auto &op = this->_remotes.back();

		if (op.connectPhase == CONNECTION_STATE_CONNECTING) {
			PacketHello hello{REAL_VERSION_STR, op.ip.toInteger(), op.port};

			return this->_send(this->_remotes.back(), &hello, sizeof(hello));
		}
		if (this->_gameId)
			return;
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

	void SpectatorConnection::_handlePacket(Connection::Remote &remote, PacketReplayList &packet, size_t size)
	{
	}

	void SpectatorConnection::requestInputs(unsigned int startFrame)
	{
		PacketReplayRequest req{this->_gameId, startFrame, false};

		this->_send(this->_remotes.front(), &req, sizeof(req));
	}
}
