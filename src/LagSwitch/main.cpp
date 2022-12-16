//
// Created by PinkySmile on 12/12/22.
//

#include <iostream>
#include <random>
#include <SFML/Network.hpp>
#include <thread>
#include <memory>

int main(int argc, char **argv)
{
	if (argc < 4) {
		std::cout << "Usage: " << argv[0] << " <bind_port> <remote_ip> <remote_port>" << std::endl;
		return EXIT_FAILURE;
	}

	sf::UdpSocket sock;
	sf::IpAddress caddr = sf::IpAddress::Any;
	sf::IpAddress haddr{argv[2]};
	unsigned short cport = 0;
	unsigned short hport = std::stoul(argv[3]);
	float packet_lost = 0;
	std::uniform_real_distribution<float> loss_dist{0, 1};
	std::uniform_int_distribution<uint64_t> delay_dist{0, 0};
	std::mt19937_64 random_gen{static_cast<unsigned long>(time(nullptr))};

	std::thread{[&delay_dist, &packet_lost]{
		std::string s;

		std::cout << "> ";
		while (std::getline(std::cin, s)) {
			size_t pos1 = s.find(':');
			size_t pos2 = s.find('/');

			try {
				if (pos1 == std::string::npos || pos2 == std::string::npos || pos1 > pos2)
					throw std::exception();

				std::string pl = s.substr(0, pos1);
				std::string nd = s.substr(pos1 + 1, pos2 - pos1 - 1);
				std::string xd = s.substr(pos2 + 1);
				auto ndf = std::stoul(nd);
				auto xdf = std::stoul(xd);

				if (ndf > xdf)
					throw std::exception();
				packet_lost = std::stof(pl);
				delay_dist = std::uniform_int_distribution<uint64_t>{ndf * 1000, xdf * 1000};
				std::cout << "Updated settings to " << packet_lost * 100 << "% packet loss, delay between " << ndf << "ms and " << xdf << "ms" << std::endl;
			} catch (...) {
				std::cout << "Invalid format. Expected <packetloss>:<mindelay>/<maxdelay>" << std::endl;
			}
			std::cout << "> ";
		}
		exit(EXIT_SUCCESS);
	}}.detach();
	if (sock.bind(std::stoul(argv[1])) != sf::Socket::Done)
		return EXIT_FAILURE;
	sock.setBlocking(false);
	while (true) {
		auto buffer = std::shared_ptr<char>((char *)malloc(1024 * 1024), free);
		size_t total = 0;
		sf::IpAddress addr = sf::IpAddress::Any;
		unsigned short port = 0;

		if (sock.receive(&*buffer, 1024 * 1024, total, addr, port) == sf::Socket::Done) {
			auto t = delay_dist(random_gen);
			sf::IpAddress raddr = addr == haddr && port == hport ? caddr : haddr;
			unsigned short rport = addr == haddr && port == hport ? cport : hport;

			//std::cout << "C" << (addr == haddr && port == hport ? '<' : '>') << "H " << total << "bytes ";
			if ((addr != haddr || port != hport) && cport == 0) {
				caddr = addr;
				cport = port;
			}
			if (loss_dist(random_gen) < packet_lost) {
				//std::cout << "Dropped" << std::endl;
				continue; // Oops, I lost your packet :(
			}
			//std::cout << t / 1000 << "ms" << std::endl;
			std::thread{[t, buffer, total, raddr, rport, &sock] {
				std::this_thread::sleep_for(std::chrono::microseconds(t));
				sock.send(&*buffer, total, raddr, rport);
			}}.detach();
		}
	}
}