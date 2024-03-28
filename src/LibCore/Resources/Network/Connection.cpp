//
// Created by PinkySmile on 20/08/2022.
//

#include "Connection.hpp"
#include "Resources/version.h"
#include "Exceptions.hpp"
#include "Resources/Game.hpp"
#include "Inputs/RemoteInput.hpp"
#include "Utils.hpp"

// The size of the recv buffer. Packets bigger than this won't be able to be received.
// Additionally, the game will check if it is not attempting to send larger packets than this.
#define RECV_BUFFER_SIZE (1 * 1024 * 1024) // 1MB
#define MAX_REPLAYS_STORED 16
#define MAX_REPLAY_SEND_FRAMES RECV_BUFFER_SIZE

namespace SpiralOfFate
{
	Connection::Connection()
	{
		this->_socket.setBlocking(false);
	}

	Connection::~Connection() = default;

	void Connection::updateDelay(unsigned int delay)
	{
		PacketDelayUpdate packet{delay, 0};

		//TODO: Do this better
		this->_send(*this->_opponent, &packet, sizeof(packet));
		this->_send(*this->_opponent, &packet, sizeof(packet));
		this->_send(*this->_opponent, &packet, sizeof(packet));
		this->_send(*this->_opponent, &packet, sizeof(packet));
		this->_expectedDelay = delay;
		this->_delay = delay;
	}

	bool Connection::send(const InputStruct &inputs)
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

		if (this->_currentMenu == MENUSTATE_INGAME)
			this->_replayData[this->_gameId].local.push_back(input);
		this->_sendBuffer.emplace_back(this->_currentFrame, input);
		this->_currentFrame++;
		return true;
	}

	unsigned int Connection::getCurrentDelay()
	{
		return this->_delay;
	}

	std::list<PacketInput> Connection::receive()
	{
		std::list<PacketInput> b = this->_buffer;

		if (!this->_sendBuffer.empty()) {
			auto packet = PacketGameFrame::create(this->_sendBuffer, this->_nextExpectedFrame, this->_gameId);

			this->_send(*this->_opponent, &*packet, packet->getSize());
		}
		if (!this->_sendSyncBuffer.empty()) {
			auto packet = PacketTimeSync::create(this->_sendSyncBuffer, this->_nextExpectedDiffFrame);

			this->_send(*this->_opponent, &*packet, packet->getSize());
		}
		this->_buffer.clear();
		return b;
	}

	void Connection::_handlePacket(Remote &remote, Packet &packet, size_t size)
	{
		remote.timeSinceLastPacket.restart();
		if (packet.opcode != OPCODE_GAME_FRAME && packet.opcode != OPCODE_TIME_SYNC)
			game->logger.debug("[<" + remote.ip.toString() + ":" + std::to_string(remote.port) + "] " + packet.toString());
		else
			game->logger.verbose("[<" + remote.ip.toString() + ":" + std::to_string(remote.port) + "] " + packet.toString());
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
		case OPCODE_GAME_QUIT:
			this->_handlePacket(remote, packet.gameQuit, size);
			break;
		case OPCODE_DESYNC_DETECTED:
			this->_handlePacket(remote, packet.desyncDetected, size);
			break;
		case OPCODE_TIME_SYNC:
			this->_handlePacket(remote, packet.timeSync, size);
			break;
		case OPCODE_REPLAY_REQUEST:
			this->_handlePacket(remote, packet.replayRequest, size);
			break;
		case OPCODE_REPLAY_LIST:
			this->_handlePacket(remote, packet.replayList, size);
			break;
		case OPCODE_REPLAY_LIST_REQUEST:
			this->_handlePacket(remote, packet.replayListRequest, size);
			break;
		default:
			PacketError error{ERROR_INVALID_OPCODE, packet.opcode, size};

			this->_send(remote, &error, sizeof(error));
			break;
		}
	}

	void Connection::update()
	{
		if (this->_terminated)
			return;

		auto buffer = new char[RECV_BUFFER_SIZE];
		auto packet = (Packet *)buffer;

		while (true) {
			size_t realSize = 0;
			sf::IpAddress ip;
			unsigned short port;
			auto res = this->_socket.receive(packet, RECV_BUFFER_SIZE, realSize, ip, port);
			auto iter = this->_remotes.begin();

			while (iter != this->_remotes.end()) {
				auto t = iter->timeSinceLastPacket.getElapsedTime().asSeconds();

				if (t < 10 && iter->connectPhase != CONNECTION_STATE_DISCONNECTED) {
					iter++;
					continue;
				}
				if (this->_opponent == &*iter) {
					auto args = new TitleScreenArguments();

					args->errorMessage = "Connection lost.";
					game->scene.switchScene("title_screen", args);
					this->terminate();
					break;
				}
				if (this->onDisconnect)
					this->onDisconnect(*iter);
				this->_remotes.erase(iter);
				iter = this->_remotes.begin();
			}

			if (res == sf::Socket::NotReady)
				break;
			else if (res != sf::Socket::Done) {
				game->logger.error("[<" + ip.toString() + ":" + std::to_string(port) + "] Error receiving packet " + std::to_string(res));
				break;
			}

			auto it = std::find_if(this->_remotes.begin(), this->_remotes.end(), [&ip, &port](Remote &remote) {
				return remote.ip == ip && remote.port == port;
			});

			if (it == this->_remotes.end()) {
				this->_remotes.emplace_back(*this, ip, port);
				it = std::prev(this->_remotes.end());
			}

			try {
				// TODO: Handle split/merged packet(s)
				this->_handlePacket(*it, *packet, realSize);
			} catch (ErrorPacketException &e) {
				game->logger.error("[<" + ip.toString() + ":" + std::to_string(port) + "] Peer responded with " + e.getPacket().toString());
				if (this->onError)
					this->onError(*it, e.getPacket());
			}
		}
		delete[] buffer;
	}

	void Connection::_send(Remote &remote, void *packet, uint32_t realSize)
	{
		auto pack = reinterpret_cast<Packet *>(packet);
		//TODO: To net endianness
		auto str = pack->toString();
		auto logStr = "[>" + remote.ip.toString() + ":" + std::to_string(remote.port) + "] " + str;

		try {
			assert_exp(realSize <= RECV_BUFFER_SIZE);
		} catch (...) {
			game->logger.fatal(logStr);
			throw;
		}
		if (pack->opcode != OPCODE_GAME_FRAME && pack->opcode != OPCODE_TIME_SYNC)
			game->logger.debug(logStr);
		else
			game->logger.verbose(logStr);
		this->_socket.send(packet, realSize, remote.ip, remote.port);
	}

	void Connection::_handlePacket(Remote &remote, PacketHello &packet, size_t size)
	{
		unsigned err = -1;

		if (size != sizeof(packet))
			err = ERROR_SIZE_MISMATCH;
		else if (remote.connectPhase != CONNECTION_STATE_NOT_INITIALIZED && remote.connectPhase != CONNECTION_STATE_CONNECTING)
			err = ERROR_UNEXPECTED_OPCODE;
		else if (std::find(this->blacklist.begin(), this->blacklist.end(), remote.ip.toString()) != this->blacklist.end())
			err = ERROR_BLACKLISTED;
		else if (packet.getMagic() != PacketHello::computeMagic(REAL_VERSION_STR))
			err = ERROR_VERSION_MISMATCH;

		if (err != (unsigned)-1) {
			PacketError error{err, OPCODE_HELLO, size};

			return this->_send(remote, &error, sizeof(error));
		}
		remote.connectPhase = CONNECTION_STATE_CONNECTING;

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
			assert_exp(!remote.pingsSent.empty());
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

	void Connection::_handlePacket(Remote &remote, PacketGameFrame &packet, size_t size)
	{
		if (remote.connectPhase != CONNECTION_STATE_PLAYER) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_GAME_FRAME, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (size < sizeof(packet) || size != packet.getSize()) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_GAME_FRAME, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (packet.gameId != this->_gameId)
			return;

		for (size_t i = 0; i < packet.nbInputs; i++) {
			if (this->_nextExpectedFrame == packet.frameId + i) {
				if (packet.gameId != this->_gameId)
					return;
				this->_nextExpectedFrame++;
				this->_buffer.push_back(packet.inputs[i]);
				if (this->_currentMenu == MENUSTATE_INGAME)
					this->_replayData[this->_gameId].remote.push_back(packet.inputs[i]);
				if (!this->onInputReceived)
					continue;
				this->onInputReceived(remote, packet.frameId + i);
			}
		}
		while (!this->_sendBuffer.empty() && this->_sendBuffer.front().first < packet.lastRecvFrameId)
			this->_sendBuffer.pop_front();
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

	void Connection::_handlePacket(Connection::Remote &remote, PacketSyncTest &packet, size_t size)
	{
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_ERROR, size};

			return this->_send(remote, &error, sizeof(error));
		}

		auto it = this->_states.find(packet.frameId);

		if (it == this->_states.end())
			return;

		if (it->second != packet.stateChecksum) {
			PacketDesyncDetected desync{it->second, packet.stateChecksum, it->first};

			this->_send(remote, &desync, sizeof(desync));
			if (this->onDesync)
				this->onDesync(remote, desync.frameId, desync.computedChecksum, desync.receivedChecksum);
		}
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
		// It is supposed to be only the opcode but if they want to quit,
		// who am I to disagree.
		if (remote.connectPhase == CONNECTION_STATE_PLAYER) {
			auto args = new TitleScreenArguments();

			args->errorMessage = "Your opponent left.";
			game->scene.switchScene("title_screen", args);
			this->terminate();
			return;
		}
		remote.connectPhase = CONNECTION_STATE_DISCONNECTED;
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

	void Connection::_handlePacket(Remote &remote, PacketGameQuit &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_GAME_QUIT, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::_handlePacket(Connection::Remote &remote, PacketDesyncDetected &packet, size_t size)
	{
		if (size != sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_DESYNC_DETECTED, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (!this->onDesync)
			return;
		this->onDesync(remote, packet.frameId, packet.receivedChecksum, packet.computedChecksum);
	}

	void Connection::_handlePacket(Connection::Remote &remote, PacketTimeSync &packet, size_t size)
	{
		if (remote.connectPhase != CONNECTION_STATE_PLAYER) {
			PacketError error{ERROR_UNEXPECTED_OPCODE, OPCODE_TIME_SYNC, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (size < sizeof(packet) || size != packet.getSize()) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_TIME_SYNC, size};

			return this->_send(remote, &error, sizeof(error));
		}

		for (size_t i = 0; i < packet.nbDiff; i++) {
			if (this->_nextExpectedDiffFrame == packet.frameId + i) {
				this->_nextExpectedDiffFrame++;
				if (!this->onTimeSync)
					continue;
				this->onTimeSync(remote, packet.frameId + i, packet.timeDiff[i]);
			}
		}
		while (!this->_sendSyncBuffer.empty() && this->_sendSyncBuffer.front().first < packet.lastRecvFrameId)
			this->_sendSyncBuffer.pop_front();
	}

	void Connection::_handlePacket(Connection::Remote &remote, PacketReplayRequest &packet, size_t size)
	{
		if (size < sizeof(packet)) {
			PacketError error{ERROR_SIZE_MISMATCH, OPCODE_REPLAY_REQUEST, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (this->_replayData.find(packet.gameId) == this->_replayData.end()) {
			PacketError error{ERROR_INVALID_DATA, OPCODE_REPLAY_REQUEST, size};

			return this->_send(remote, &error, sizeof(error));
		}

		auto &array = this->_replayData[packet.gameId];
		auto total = std::min<size_t>(MAX_REPLAY_SEND_FRAMES, std::min(array.local.size(), array.remote.size()));

		if (total < packet.frame) {
			PacketError error{ERROR_INVALID_DATA, OPCODE_REPLAY_REQUEST, size};

			return this->_send(remote, &error, sizeof(error));
		}
		if (packet.frame == 0) {
			PacketGameStart gameStart{
				array.params.seed,
				array.params.p1chr,
				array.params.p1pal,
				array.params.p2chr,
				array.params.p2pal,
				array.params.stage,
				array.params.platformConfig
			};

			this->_send(remote, &gameStart, sizeof(gameStart));
		}
		if (total == packet.frame) {
			auto replay = PacketReplay::create(nullptr, 0, packet.gameId, packet.frame, packet.gameId == this->_gameId ? 0 : total, 0);

			return this->_send(remote, &*replay, replay->getSize());
		}

		auto s = total - packet.frame;
		size_t bufferSize = s * sizeof(PacketInput) * 2;
		auto buffer = new unsigned char[bufferSize];

		memcpy(buffer,                  (this->_swapSide ? array.remote : array.local).data() + packet.frame, bufferSize / 2);
		memcpy(buffer + bufferSize / 2, (this->_swapSide ? array.local : array.remote).data() + packet.frame, bufferSize / 2);

		auto replay = PacketReplay::create(buffer, bufferSize, packet.gameId, packet.frame, packet.gameId == this->_gameId ? 0 : total, s);

		delete[] buffer;
		this->_send(remote, &*replay, replay->getSize());
	}

	void Connection::_handlePacket(Connection::Remote &remote, PacketReplayList &, size_t size)
	{
		//Implemented in children classes
		PacketError error{ERROR_NOT_IMPLEMENTED, OPCODE_REPLAY_LIST, size};

		this->_send(remote, &error, sizeof(error));
	}

	void Connection::_handlePacket(Connection::Remote &remote, PacketReplayListRequest &, size_t size)
	{
		std::vector<unsigned> ids;

		ids.reserve(this->_replayData.size());
		for (auto pair : this->_replayData)
			ids.push_back(pair.first);

		auto packet = PacketReplayList::create(ids);

		this->_send(remote, &*packet, packet->getSize());
	}

	void Connection::terminate()
	{
		PacketQuit quit;

		if (this->_terminated)
			return;
		this->_terminationMutex.lock();
		this->_terminated = true;
		this->_sendBuffer.clear();
		this->_sendSyncBuffer.clear();
		this->_currentFrame = 0;
		this->_nextExpectedFrame = 0;
		for (auto &remote : this->_remotes)
			this->_send(remote, &quit, sizeof(quit));
		this->_remotes.clear();
		this->_opponent = nullptr;
		this->_terminationMutex.unlock();
	}

	const std::pair<std::string, std::string> &Connection::getNames() const
	{
		return this->_names;
	}

	bool Connection::isTerminated() const
	{
		return this->_terminated;
	}

	void Connection::reportChecksum(unsigned int checksum, unsigned int frameId)
	{
		PacketSyncTest syncTest{checksum, frameId};

		this->_states[frameId] = checksum;
		this->_send(*this->_opponent, &syncTest, sizeof(syncTest));
	}

	void Connection::nextGame()
	{
		this->_gameId++;
		this->_buffer.clear();
		this->_sendBuffer.clear();
		this->_sendSyncBuffer.clear();
		this->_currentFrame = 0;
		this->_nextExpectedFrame = 0;
		this->_nextExpectedDiffFrame = 0;
		this->_states.clear();
		for (auto input : this->_registeredInputs)
			input->flush();
	}

	void Connection::quitGame()
	{
		PacketGameQuit op;

		this->_send(*this->_opponent, &op, sizeof(op));
	}

	void Connection::notifySwitchMenu()
	{
		this->_terminationMutex.lock();
		if (this->_terminated)
			return this->_terminationMutex.unlock();
		if (this->_currentMenu > MENUSTATE_LOADING_OFFSET)
			this->_currentMenu -= MENUSTATE_LOADING_OFFSET;

		PacketMenuSwitch menuSwitch{this->_currentMenu, this->_opCurrentMenu};

		this->_send(*this->_opponent, &menuSwitch, sizeof(menuSwitch));
		this->_terminationMutex.unlock();
	}

	void Connection::waitForOpponent()
	{
		int i = 0;
		PacketMenuSwitch menuSwitch{this->_currentMenu, this->_opCurrentMenu};

		while (this->_opCurrentMenu != this->_currentMenu) {
			if (i % 20 == 0) {
				menuSwitch.opMenuId = this->_opCurrentMenu;
				this->_terminationMutex.lock();
				if (this->_terminated)
					return this->_terminationMutex.unlock();
				this->_send(*this->_opponent, &menuSwitch, sizeof(menuSwitch));
				this->_terminationMutex.unlock();
			}
			i++;
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}

	void Connection::timeSync(long long int time, unsigned frame)
	{
		game->logger.verbose("Adding timesync data for frame " + std::to_string(frame));
		this->_sendSyncBuffer.emplace_back(frame, time);
	}

	void Connection::Remote::_pingLoop()
	{
		PacketPing ping(0);

		while (this->connectPhase != CONNECTION_STATE_DISCONNECTED) {
			this->base._send(*this, &ping, sizeof(ping));
			for (int i = 0; i < 20 && this->connectPhase != CONNECTION_STATE_DISCONNECTED; i++)
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			ping.seqId++;
		}
	}

	Connection::Remote::~Remote()
	{
		if (this->connectPhase != CONNECTION_STATE_DISCONNECTED) {
			PacketQuit quit;

			this->base._send(*this, &quit, sizeof(quit));
			this->connectPhase = CONNECTION_STATE_DISCONNECTED;
		}
		if (this->pingThread.joinable())
			this->pingThread.join();
	}

	Connection::Remote::Remote(Connection &base, const sf::IpAddress &ip, unsigned short port) :
		base(base),
		ip(ip),
		port(port)/*,
		pingThread{&Remote::_pingLoop, this}*/
	{

	}
}