//
// Created by PinkySmile on 12/12/22.
//

#include <iostream>
#include <random>
#include <SFML/Network.hpp>
#include <thread>
#include <memory>
#include <mutex>

int main(int argc, char **argv)
{
	if (argc < 4) {
		std::cout << "Usage: " << argv[0] << " <bind_port> <remote_ip> <remote_port>" << std::endl;
		return EXIT_FAILURE;
	}

	sf::UdpSocket sock;
	sf::IpAddress caddr = sf::IpAddress::Any;
	sf::IpAddress haddr = (*sf::Dns::resolve(argv[2]))[0];
	unsigned short cport = 0;
	unsigned short hport = std::stoul(argv[3]);
	std::mutex f;
	size_t inSize = 0;
	size_t outSize = 0;
	float packet_lost = 0;
	std::uniform_real_distribution<float> loss_dist{0, 1};
	std::uniform_int_distribution<uint64_t> delay_dist{0, 0};
	std::mt19937_64 random_gen{static_cast<unsigned long>(time(nullptr))};
	std::thread disp{[&f, &inSize, &outSize] {
		while (true) {
			f.lock();
			size_t i = inSize * 8;
			size_t o = outSize * 8;
			inSize = 0;
			outSize = 0;
			f.unlock();

			std::cout << i / 1024.f << "kb/s|" << o / 1024.f << "kb/s\033[J\033[A" << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}};

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
	if (sock.bind(std::stoul(argv[1])) != sf::Socket::Status::Done)
		return EXIT_FAILURE;
	sock.setBlocking(false);
	while (true) {
		auto buffer = std::shared_ptr<char>((char *)malloc(1024 * 1024), free);
		size_t total = 0;
		std::optional<sf::IpAddress> addr;
		unsigned short port = 0;

		if (sock.receive(&*buffer, 1024 * 1024, total, addr, port) == sf::Socket::Status::Done) {
			auto t = delay_dist(random_gen);
			sf::IpAddress raddr{0, 0, 0, 0};
			unsigned short rport;

			if (addr == haddr && port == hport) {
				raddr = caddr;
				rport = cport;
				inSize += total;
			} else {
				raddr = haddr;
				rport = hport;
				outSize += total;
			}
			//std::cout << "C" << (addr == haddr && port == hport ? '<' : '>') << "H " << total << "bytes ";
			if ((addr != haddr || port != hport) && cport == 0) {
				caddr = *addr;
				cport = port;
			}
			if (loss_dist(random_gen) < packet_lost) {
				//std::cout << "Dropped" << std::endl;
				continue; // Oops, I lost your packet :(
			}
			//std::cout << t / 1000 << "ms" << std::endl;
			std::thread{[t, buffer, total, raddr, rport, &sock] {
				if (t != 0)
					std::this_thread::sleep_for(std::chrono::microseconds(t));
				static_cast<void>(sock.send(&*buffer, total, raddr, rport));
			}}.detach();
		}
	}
}