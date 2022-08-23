//
// Created by PinkySmile on 22/08/2022.
//

#ifndef SOFGV_SERVERCONNECTION_HPP
#define SOFGV_SERVERCONNECTION_HPP


#include "Connection.hpp"

namespace SpiralOfFate
{
	class ServerConnection : public Connection {
	protected:
		unsigned _currentMenu = 0;
		unsigned _opCurrentMenu = 0;
		bool _playing = false;
		std::map<unsigned, unsigned> _states;
		unsigned int seed;
		unsigned int p1chr;
		unsigned int p1pal;
		unsigned int p2chr;
		unsigned int p2pal;
		unsigned int stage;
		unsigned int platformConfig;

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
		bool spectatorEnabled = true;
		std::function<void (Remote &remote, PacketInitRequest &packet)> onConnection;

		ServerConnection(const std::string &name);

		bool send(InputStruct &inputs) override;
		void startGame(unsigned seed, unsigned p1chr, unsigned p1pal, unsigned p2chr, unsigned p2pal, unsigned stage, unsigned platformConfig);
		void reportChecksum(unsigned checksum);
		void switchMenu(unsigned id);
		void host(unsigned short port);
	};
}


#endif //SOFGV_SERVERCONNECTION_HPP
