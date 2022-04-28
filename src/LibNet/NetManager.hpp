//
// Created by Gegel85 on 26/04/2022.
//

#ifndef SOFGV_NETMANAGER_HPP
#define SOFGV_NETMANAGER_HPP


#include <string>
#include <thread>
#include <SFML/Network.hpp>

namespace SpiralOfFate
{
	class NetManager {
	private:
		class Connection {
			sf::IpAddress addr;
			unsigned short port;
		};

		sf::UdpSocket _sock;

	public:
		void host(unsigned short port);
		void connect(const std::string &ip, unsigned short port);
		void spectate(const std::string &ip, unsigned short port);
	};
}


#endif //SOFGV_NETMANAGER_HPP
