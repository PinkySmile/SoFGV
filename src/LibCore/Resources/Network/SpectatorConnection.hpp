//
// Created by PinkySmile on 22/08/2022.
//

#ifndef SOFGV_SPECTATORCONNECTION_HPP
#define SOFGV_SPECTATORCONNECTION_HPP


#include "Resources/SceneArgument.hpp"
#include "Connection.hpp"

namespace SpiralOfFate
{
	class SpectatorConnection : public Connection {
	protected:
		void _handlePacket(Remote &remote, PacketOlleh &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketRedirect &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketPunch &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketInitRequest &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketInitSuccess &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketMenuSwitch &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketState &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketReplay &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketGameStart &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketReplayList &packet, size_t size) override;

	public:
		std::function<void (Remote &remote, PacketInitSuccess &packet)> onConnection;
		std::function<void (PacketGameStart &packet)> onGameStart;
		std::function<void (PacketReplay &replay)> onReplayData;

		SpectatorConnection(const std::string &name);

		void connect(sf::IpAddress ip, unsigned short port);
		void update() override;
		void requestInputs(unsigned startFrame);
	};
}


#endif //SOFGV_SPECTATORCONNECTION_HPP
