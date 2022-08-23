//
// Created by PinkySmile on 22/08/2022.
//

#ifndef SOFGV_CLIENTCONNECTION_HPP
#define SOFGV_CLIENTCONNECTION_HPP


#include "Connection.hpp"

namespace SpiralOfFate
{
	class ClientConnection : public Connection {
	protected:
		unsigned _currentMenu = 0;
		unsigned _opCurrentMenu = 0;
		bool _playing = false;
		std::map<unsigned, unsigned> _states;
		unsigned int p1chr = 0;
		unsigned int p1pal = 0;
		unsigned int p2chr = 0;
		unsigned int p2pal = 0;
		unsigned int stage = 0;
		unsigned int platformConfig = 0;

		void _handlePacket(Remote &remote, PacketOlleh &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketRedirect &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketPunch &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketGameFrame &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketInitRequest &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketInitSuccess &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketDelayUpdate &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketMenuSwitch &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketSyncTest &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketState &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketReplay &packet, size_t size) override;
		void _handlePacket(Remote &remote, PacketGameStart &packet, size_t size) override;

	public:
		std::function<void (Remote &remote, PacketInitSuccess &packet)> onConnection;

		ClientConnection(const std::string &name);

		void reportChecksum(unsigned checksum);
	};
}


#endif //SOFGV_CLIENTCONNECTION_HPP
