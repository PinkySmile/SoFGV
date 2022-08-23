//
// Created by PinkySmile on 20/08/2022.
//

#include "Connection.hpp"
#include "Resources/version.h"
#include "Exceptions.hpp"

namespace SpiralOfFate
{
	Connection::Connection()
	{
		this->_socket.setBlocking(false);
	}

	void Connection::updateDelay(unsigned int delay)
	{
		this->_expectedDelay = delay;
	}

	bool Connection::send(InputStruct &inputs)
	{
		PacketInput input;

		input.d = !!inputs.d;
		input.a = !!inputs.a;
		input.n = !!inputs.n;
		input.v = !!inputs.v;
		input.m = !!inputs.m;
		input.s = !!inputs.s;

		if (inputs.horizontalAxis < 0)
			input._h = -1;
		else if (inputs.horizontalAxis == 0)
			input._h = 0;
		else
			input._h = 1;

		if (inputs.verticalAxis < 0)
			input._v = -1;
		else if (inputs.verticalAxis == 0)
			input._v = 0;
		else
			input._v = 1;

		this->_mutex.lock();
		this->_sendBuffer.emplace_back(this->_currentFrame, input);
		this->_currentFrame++;

		auto packet = PacketGameFrame::create(this->_sendBuffer);

		this->_send(*this->_opponent, &*packet, packet->nbInputs * sizeof(*PacketGameFrame::inputs) + sizeof(PacketGameFrame));
		this->_mutex.unlock();
		return true;
	}

	unsigned int Connection::getCurrentDelay()
	{
		return this->_delay;
	}

	std::vector<PacketInput> Connection::receive()
	{
		std::vector<PacketInput> b = this->_buffer;

		this->_buffer.clear();
		return b;
	}

	void Connection::_handlePacket(Remote &remote, Packet &packet, size_t size)
	{
		game->logger.debug("[<" + remote.ip.toString() + "] " + packet.toString());
		this->_mutex.lock();
		try {
			switch (packet.opcode) {
			case OPCODE_HELLO:
				this->_handlePacket(remote, packet.hello, size);
				break;
			case OPCODE_OLLEH:
				this->_handlePacket(remote, packet.olleh, size);
				break;
			case OPCODE_REDIRECT:
				this->_handlePacket(remote, packet.redirect, size);
				break;
			case OPCODE_PUNCH:
				this->_handlePacket(remote, packet.punch, size);
				break;
			case OPCODE_PING:
				this->_handlePacket(remote, packet.ping, size);
				break;
			case OPCODE_PONG:
				this->_handlePacket(remote, packet.pong, size);
				break;
			case OPCODE_GAME_START:
				this->_handlePacket(remote, packet.gameStart, size);
				break;
			case OPCODE_GAME_FRAME:
				this->_handlePacket(remote, packet.gameFrame, size);
				break;
			case OPCODE_INIT_REQUEST:
				this->_handlePacket(remote, packet.initRequest, size);
				break;
			case OPCODE_INIT_SUCCESS:
				this->_handlePacket(remote, packet.initSuccess, size);
				break;
			case OPCODE_ERROR:
				this->_handlePacket(remote, packet.error, size);
				break;
			case OPCODE_DELAY_UPDATE:
				this->_handlePacket(remote, packet.delayUpdate, size);
				break;
			case OPCODE_MENU_SWITCH:
				this->_handlePacket(remote, packet.menuSwitch, size);
				break;
			case OPCODE_SYNC_TEST:
				this->_handlePacket(remote, packet.syncTest, size);
				break;
			case OPCODE_STATE:
				this->_handlePacket(remote, packet.state, size);
				break;
			case OPCODE_REPLAY:
				this->_handlePacket(remote, packet.replay, size);
				break;
			case OPCODE_QUIT:
				this->_handlePacket(remote, packet.quit, size);
				break;
			default:
				PacketError error{ERROR_INVALID_OPCODE, packet.opcode, size};

				this->_send(remote, &error, sizeof(error));
				break;
			}
		} catch (...) {
			this->_mutex.unlock();
			throw;
		}
		this->_mutex.unlock();
	}

	void Connection::_threadLoop()
	{
		Packet *packet = nullptr;
		size_t allocSize = 0;

		while (!this->_endThread) {
			uint32_t size;
			size_t realSize;
			sf::IpAddress ip = sf::IpAddress::Any;
			unsigned short port = 0;
			auto res = this->_socket.receive(&size, sizeof(size), realSize, ip, port);
			auto iter = this->_remotes.begin();
			auto olditer = iter;

			while (iter != this->_remotes.end()) {
				if (iter->connectPhase >= 0) {
					olditer = iter;
					iter++;
					continue;
				}
				if (this->onDisconnect)
					this->onDisconnect(*iter);
				this->_remotes.erase(iter);
				if (olditer == iter)
					olditer = this->_remotes.begin();
				else
					olditer = iter;
			}

			if (res == sf::Socket::NotReady) {
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				continue;
			} else if (res != sf::Socket::Done) {
				game->logger.error("Error receiving packet " + std::to_string(res));
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				continue;
			} else if (realSize != sizeof(size)) {
				game->logger.error("Invalid packet header size " + std::to_string(realSize) + " != " + std::to_string(sizeof(size)));
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				continue;
			}

			auto it = std::find_if(this->_remotes.begin(), this->_remotes.end(), [&ip, &port](Remote &remote) {
				return remote.ip == ip && remote.port == port;
			});

			if (it == this->_remotes.end()) {
				this->_remotes.emplace_back(*this, ip, port);
				it = std::prev(this->_remotes.end());
			}
			if (allocSize < size) {
				auto old = (Packet *)realloc(packet, size);

				if (!old) {
					game->logger.error("Error allocating packet of size " + std::to_string(size));
					this->terminate();
					return;
				}
				packet = old;
				allocSize = size;
			}

			res = this->_socket.receive(packet, size, realSize, ip, port);
			if (res == sf::Socket::NotReady) {
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				continue;
			} else if (res != sf::Socket::Done) {
				game->logger.error("Error receiving packet " + std::to_string(res));
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				continue;
			} else if (realSize != size) {
				game->logger.error("Invalid packet size " + std::to_string(realSize) + " != " + std::to_string(size));
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				continue;
			}

			try {
				this->_handlePacket(*it, *packet, realSize);
			} catch (ErrorPacketException &e) {
				game->logger.error("Peer responded with " + e.getPacket().toString());
				if (this->onError)
					this->onError(*it, e.getPacket());
			}
		}
	}

	void Connection::_send(Remote &remote, void *packet, size_t size)
	{
		game->logger.debug("[>" + remote.ip.toString() + "] " + reinterpret_cast<Packet *>(packet)->toString());
		this->_socket.send(&size, sizeof(size), remote.ip, remote.port);
		this->_socket.send(packet, size, remote.ip, remote.port);
	}

	void Connection::_handlePacket(Remote &remote, PacketHello &packet, size_t size)
	{
		unsigned err = -1;

		if (size != sizeof(packet))
			err = ERROR_SIZE_MISMATCH;
		else if (std::find(this->blacklist.begin(), this->blacklist.end(), remote.ip.toString()) != this->blacklist.end())
			err = ERROR_BLACKLISTED;
		else if (packet.getMagic() != PacketHello::computeMagic(REAL_VERSION_STR))
			err = ERROR_VERSION_MISMATCH;

		if (err != (unsigned)-1) {
			PacketError error{err, OPCODE_HELLO, size};

			return this->_send(remote, &error, sizeof(error));
		}
		PacketOlleh olleh;

		this->_send(remote, &olleh, sizeof(olleh));
	}

	void Connection::_handlePacket(Remote &remote, PacketOlleh &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_OLLEH, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::_handlePacket(Remote &remote, PacketRedirect &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_REDIRECT, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::_handlePacket(Remote &remote, PacketPunch &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_PUNCH, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::_handlePacket(Remote &remote, PacketPing &packet, size_t size)
	{
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_PING, size};

			return this->_send(remote, &error, sizeof(error));
		}

		PacketPong pong{packet.seqId};

		this->_send(remote, &pong, sizeof(pong));
	}

	void Connection::_handlePacket(Remote &remote, PacketPong &packet, size_t size)
	{
		if (remote.pingsSent.empty() || remote.pingsSent.front().sequence > packet.seqId)
			return;
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_PONG, size};

			return this->_send(remote, &error, sizeof(error));
		}
		while (remote.pingsSent.front().sequence != packet.seqId) {
			remote.pingsSent.pop_front();
			remote.pingLost++;
			my_assert(!remote.pingsSent.empty());
		}

		auto time = remote.pingsSent.front().clock.getElapsedTime();

		remote.pingsSent.pop_front();
		remote.pingTimeLast = time.asMilliseconds();
		remote.pingTimePeak = std::max(remote.pingTimePeak, remote.pingTimeLast);
		remote.pingsReceived.push_back(remote.pingTimeLast);
		if (remote.pingsReceived.size() >= 120) {
			remote.pingTimeSum -= remote.pingsReceived.front();
			remote.pingsReceived.pop_front();
		}
	}

	void Connection::_handlePacket(Remote &remote, PacketGameFrame &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_GAME_FRAME, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::_handlePacket(Remote &remote, PacketInitRequest &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_INIT_REQUEST, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::_handlePacket(Remote &remote, PacketInitSuccess &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_INIT_SUCCESS, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::_handlePacket(Remote &remote, PacketError &packet, size_t size)
	{
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_ERROR, size};

			return this->_send(remote, &error, sizeof(error));
		}
		throw ErrorPacketException(packet);
	}

	void Connection::_handlePacket(Remote &remote, PacketDelayUpdate &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_DELAY_UPDATE, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::_handlePacket(Remote &remote, PacketMenuSwitch &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_MENU_SWITCH, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::_handlePacket(Remote &remote, PacketSyncTest &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_SYNC_TEST, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::_handlePacket(Remote &remote, PacketState &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_STATE, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::_handlePacket(Remote &remote, PacketReplay &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_REPLAY, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::_handlePacket(Remote &remote, PacketQuit &packet, size_t size)
	{
		// We disconnect them even if the packet was malformed.
		// It is supposed to be only the opcode and if they want to quit,
		// who am I to make them stay.
		remote.connectPhase = -1;
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_QUIT, size};

			return this->_send(remote, &error, sizeof(error));
		}
	}

	void Connection::_handlePacket(Remote &remote, PacketGameStart &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_GAME_START, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::terminate()
	{
		this->_sendBuffer.clear();
		this->_endThread = true;
		this->_remotes.clear();
		this->_opponent = nullptr;
	}

	const std::pair<std::string, std::string> &Connection::getNames() const
	{
		return this->_names;
	}

	bool Connection::isTerminated() const
	{
		return this->_endThread;
	}

	void Connection::Remote::_pingLoop()
	{
		PacketPing ping(0);

		while (this->connectPhase >= 0) {
			this->base._send(*this, &ping, sizeof(ping));
			for (int i = 0; i < 100 && this->connectPhase >= 0; i++)
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			ping.seqId++;
		}
	}

	Connection::Remote::~Remote()
	{
		if (this->connectPhase >= 0) {
			PacketQuit quit;

			this->base._send(*this, &quit, sizeof(quit));
			this->connectPhase = -1;
		}
	}
}