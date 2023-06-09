//
// Created by PinkySmile on 22/08/2022.
//

#ifndef SOFGV_CLIENTCONNECTION_HPP
#define SOFGV_CLIENTCONNECTION_HPP


#include "Resources/SceneArgument.hpp"
#include "Connection.hpp"

namespace SpiralOfFate
{
	class ClientConnection : public Connection {
	protected:
		bool _playing = false;

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

	public:
		std::function<void (Remote &remote, PacketInitSuccess &packet)> onConnection;
		std::function<void (PacketGameStart &packet)> onGameStart;

		ClientConnection(const std::string &name);

		void connect(sf::IpAddress ip, unsigned short port);
		void update() override;
	};
}


#endif //SOFGV_CLIENTCONNECTION_HPP
