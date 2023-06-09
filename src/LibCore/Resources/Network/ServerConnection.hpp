//
// Created by PinkySmile on 22/08/2022.
//

#ifndef SOFGV_SERVERCONNECTION_HPP
#define SOFGV_SERVERCONNECTION_HPP


#include "Connection.hpp"
#include "Inputs/IInput.hpp"

namespace SpiralOfFate
{
	class ServerConnection : public Connection {
	protected:
		bool _playing = false;

		void _checkOpponentState();
		void _handlePacket(Remote &remote, PacketOlleh &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketRedirect &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketPunch &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketInitRequest &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketInitSuccess &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketDelayUpdate &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketMenuSwitch &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketState &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketReplay &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketGameStart &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketGameQuit &packet, size_t size) override;

		void _handlePacket(Remote &remote, PacketError &packet, size_t size) override;

	public:
		bool spectatorEnabled = true;
		std::function<void (Remote &remote, PacketInitRequest &packet)> onConnection;

		ServerConnection(const std::string &name);
		~ServerConnection();

		void switchToChrLoadingScreen();
		void startGame(unsigned seed, unsigned p1chr, unsigned p1pal, unsigned p2chr, unsigned p2pal, unsigned stage, unsigned platformConfig);
		void switchMenu(unsigned id, bool lock = true);
		void host(unsigned short port);
		void update() override;
	};
}


#endif //SOFGV_SERVERCONNECTION_HPP
