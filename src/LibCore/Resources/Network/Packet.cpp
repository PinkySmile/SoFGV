//
// Created by PinkySmile on 17/08/2022.
//

#include <memory>
#include <cstdlib>
#include <string>
#include <cstring>
#include <vector>
#include "Resources/Game.hpp"
#include "Packet.hpp"
#include "Utils.hpp"


namespace SpiralOfFate
{
	void deleteArray(void *a) noexcept
	{
		delete[] (char *)a;
	}

	unsigned PacketHello::computeMagic(const char *version)
	{
		unsigned magic = magicStart;

		for (int i = 0; version[i]; i++) {
			magic <<= 1;
			magic += version[i];
		}
		return magic;
	}

	PacketHello::PacketHello(const char *version, unsigned targetIp, unsigned targetPort) :
		opcode(OPCODE_HELLO),
		magic(computeMagic(version)),
		targetIp(targetIp),
		targetPort(targetPort)
	{
	}

	unsigned int PacketHello::getMagic() const
	{
		return this->magic;
	}

	std::string PacketHello::toString() const
	{
		return "Packet HELLO: magic " + std::to_string(this->magic) +
			" targetIp " + std::to_string(((unsigned char *)&this->targetIp)[0]) +
			"." + std::to_string(((unsigned char *)&this->targetIp)[1]) +
			"." + std::to_string(((unsigned char *)&this->targetIp)[2]) +
			"." + std::to_string(((unsigned char *)&this->targetIp)[3]) +
			" targetPort " + std::to_string(this->targetPort);
	}

	PacketOlleh::PacketOlleh() :
		opcode(OPCODE_OLLEH)
	{
	}

	std::string PacketOlleh::toString() const
	{
		return "Packet OLLEH";
	}

	PacketGameStart::PacketGameStart(
		unsigned seed,
		unsigned p1chr,
		unsigned p1pal,
		unsigned p2chr,
		unsigned p2pal,
		unsigned stage,
		unsigned platformConfig
	) :
		opcode(OPCODE_GAME_START),
		seed(seed),
		p1chr(p1chr),
		p1pal(p1pal),
		p2chr(p2chr),
		p2pal(p2pal),
		stage(stage),
		platformConfig(platformConfig)
	{
	}

	std::string PacketGameStart::toString() const
	{
		return "Packet GAMESTART"
		       " seed:" + std::to_string(this->seed) +
		       " p1chr:" + std::to_string(this->p1chr) +
		       " p1pal:" + std::to_string(this->p1pal) +
		       " p2chr:" + std::to_string(this->p2chr) +
		       " p2pal:" + std::to_string(this->p2pal) +
		       " stage:" + std::to_string(this->stage) +
		       " platformConfig:" + std::to_string(this->platformConfig);
	}

	PacketRedirect::PacketRedirect(unsigned targetIp, unsigned targetPort) :
		opcode(OPCODE_REDIRECT),
		targetIp(targetIp),
		targetPort(targetPort)
	{
	}

	std::string PacketRedirect::toString() const
	{
		return "Packet REDIRECT: targetIp " + std::to_string(((char *)&this->targetIp)[0]) + "." + std::to_string(((char *)&this->targetIp)[1]) + "." + std::to_string(((char *)&this->targetIp)[2]) + "." + std::to_string(((char *)&this->targetIp)[3]) + " targetPort " + std::to_string(this->targetPort);
	}

	PacketPunch::PacketPunch(unsigned targetIp, unsigned targetPort) :
		opcode(OPCODE_PUNCH),
		targetIp(targetIp),
		targetPort(targetPort)
	{
	}

	std::string PacketPunch::toString() const
	{
		return "Packet PUNCH: targetIp " +
			std::to_string(((unsigned char *)&this->targetIp)[0]) + "." +
			std::to_string(((unsigned char *)&this->targetIp)[1]) + "." +
			std::to_string(((unsigned char *)&this->targetIp)[2]) + "." +
			std::to_string(((unsigned char *)&this->targetIp)[3]) + " targetPort " + std::to_string(this->targetPort);
	}

	PacketPing::PacketPing(unsigned seqId) :
		opcode(OPCODE_PING),
		seqId(seqId)
	{
	}

	std::string PacketPing::toString() const
	{
		return "Packet PING: seqId " + std::to_string(this->seqId);
	}

	PacketPong::PacketPong(unsigned seqId) :
		opcode(OPCODE_PONG),
		seqId(seqId)
	{
	}

	std::string PacketPong::toString() const
	{
		return "Packet PONG: seqId " + std::to_string(this->seqId);
	}

	PacketGameFrame::PacketGameFrame() :
		opcode(OPCODE_GAME_FRAME)
	{
	}

	std::shared_ptr<PacketGameFrame> PacketGameFrame::create(std::list<std::pair<unsigned, PacketInput>> &inputs, uint32_t lastRecvFrameId, uint32_t gameId)
	{
		char *buffer = new char[inputs.size() * sizeof(*PacketGameFrame::inputs) + sizeof(PacketGameFrame)];
		auto *packet = new(buffer) PacketGameFrame();
		size_t i = 0;

		my_assert(!inputs.empty());
		my_assert((void *)buffer == (void *)packet);
		packet->lastRecvFrameId = lastRecvFrameId;
		packet->frameId = inputs.front().first;
		packet->nbInputs = inputs.size();
		packet->gameId = gameId;
		for (auto &input : inputs) {
			packet->inputs[i] = input.second;
			i++;
		}
		my_assert(i == inputs.size() && i == packet->nbInputs);
		return {packet, deleteArray};
	}

	std::string PacketGameFrame::toString() const
	{
		return "Packet GAMEFRAME: " + std::to_string(this->nbInputs) + " inputs in game " + std::to_string(this->gameId) + " from frame " + std::to_string(this->frameId) + " expecting " + std::to_string(this->lastRecvFrameId);
	}

	size_t PacketGameFrame::getSize()
	{
		return this->nbInputs * sizeof(*this->inputs) + sizeof(*this);
	}

	PacketInitRequest::PacketInitRequest(const char *name, const char *version, bool spectator) :
		opcode(OPCODE_INIT_REQUEST),
		spectator(spectator)
	{
		my_assert(strlen(name) < sizeof(this->playerName));
		my_assert(strlen(version) < sizeof(this->gameVersion));
		strncpy(this->playerName, name, sizeof(this->playerName));
		strncpy(this->gameVersion, version, sizeof(this->gameVersion));
	}

	std::string PacketInitRequest::toString() const
	{
		char buffer[130];

		memset(buffer, 0, sizeof(buffer));
		strcpy(buffer, "Packet INITREQUEST: As ");
		strcat(buffer, this->spectator ? "spectator" : "player");
		strcat(buffer, " from player ");
		strncat(buffer, this->playerName, sizeof(this->playerName));
		strcat(buffer, " using game version ");
		strncat(buffer, this->gameVersion, sizeof(this->gameVersion));
		return buffer;
	}

	PacketInitSuccess::PacketInitSuccess(const char *name1, const char *name2, const char *version) :
		opcode(OPCODE_INIT_SUCCESS)
	{
		strncpy(this->player1Name, name1, sizeof(this->player1Name));
		if (name2)
			strncpy(this->player2Name, name2, sizeof(this->player2Name));
		strncpy(this->gameVersion, version, sizeof(this->gameVersion));
	}

	std::string PacketInitSuccess::toString() const
	{
		char buffer[180];

		memset(buffer, 0, sizeof(buffer));
		strcpy(buffer, "Packet INITSUCCESS: '");
		strncat(buffer, this->player1Name, sizeof(this->player1Name));
		strcat(buffer, "' vs '");
		strncat(buffer, this->player2Name, sizeof(this->player2Name));
		strcat(buffer, "' using game version ");
		strncat(buffer, this->gameVersion, sizeof(this->gameVersion));
		return buffer;
	}

	PacketError::PacketError(unsigned code, Opcode offendingPacket, size_t offendingPacketSize) :
		opcode(OPCODE_ERROR),
		code(code),
		offendingPacket(offendingPacket),
		offendingPacketSize(offendingPacketSize)
	{}

	std::string PacketError::toString() const
	{
		return "Packet ERROR: code " + std::to_string(this->code) + " (" + (this->code < errorStrings.size() ? errorStrings[this->code] : "Unknown code") + "). "
		       "Faulting packet: " + std::to_string(this->offendingPacket) + " (" + (this->offendingPacket < opcodeStrings.size() ? opcodeStrings[this->offendingPacket] : "Unknown opcode") + ") of size " + std::to_string(this->offendingPacketSize) + " bytes.";
	}

	PacketDelayUpdate::PacketDelayUpdate(unsigned newDelay, unsigned frameId) :
		opcode(OPCODE_DELAY_UPDATE),
		frameId(frameId),
		newDelay(newDelay)
	{
	}

	std::string PacketDelayUpdate::toString() const
	{
		return "Packet DELAYUPDATE: delay " + std::to_string(this->newDelay) + " frameId " + std::to_string(this->frameId);
	}

	PacketMenuSwitch::PacketMenuSwitch(unsigned menuId, unsigned op) :
		opcode(OPCODE_MENU_SWITCH),
		menuId(menuId),
		opMenuId(op)
	{
	}

	std::string PacketMenuSwitch::toString() const
	{
		return "Packet MENUSWITCH: menuId " + std::to_string(this->menuId) + " opmenuId " + std::to_string(this->opMenuId);
	}

	PacketSyncTest::PacketSyncTest(unsigned stateChecksum, unsigned frameId) :
		opcode(OPCODE_SYNC_TEST),
		frameId(frameId),
		stateChecksum(stateChecksum)
	{
	}

	std::string PacketSyncTest::toString() const
	{
		return "Packet SYNCTEST: stateChecksum " + std::to_string(this->stateChecksum) + " frameId " + std::to_string(this->frameId);
	}

	PacketState::PacketState() :
		opcode(OPCODE_STATE)
	{
	}

	std::shared_ptr<PacketState> PacketState::create(void *data, size_t size)
	{
		void *buffer = new char[size + 1];
		auto *packet = new(buffer) PacketState();

		//TODO: Compress the data
		my_assert((void *)buffer == (void *)packet);
		return {packet, deleteArray};
	}

	std::string PacketState::toString() const
	{
		return "Packet STATE: " + std::to_string(this->compressedSize) + " compressed bytes";
	}

	PacketReplay::PacketReplay() :
		opcode(OPCODE_REPLAY)
	{
	}

	std::shared_ptr<PacketReplay> PacketReplay::create(void *data, size_t size, unsigned gameId, unsigned frameId, unsigned lastFrameId, unsigned nbInputs)
	{
		std::vector<unsigned char> out;

		if (size)
			Utils::Z::compress((unsigned char *)data, size, out, -1);

		void *buffer = new char[out.size() + sizeof(PacketReplay)];
		auto *packet = new(buffer) PacketReplay();

		my_assert((void *)buffer == (void *)packet);
		packet->compressedSize = out.size();
		packet->gameId = gameId;
		packet->frameId = frameId;
		packet->lastFrameId = lastFrameId;
		packet->nbInputs = nbInputs;
		memcpy(packet->compressedData, out.data(), out.size());
		return {packet, deleteArray};
	}

	std::string PacketReplay::toString() const
	{
		return "Packet REPLAY: " + std::to_string(this->compressedSize) + " compressed bytes";
	}

	unsigned PacketReplay::getSize() const
	{
		return sizeof(PacketReplay) + this->compressedSize;
	}

	PacketQuit::PacketQuit() :
		opcode(OPCODE_QUIT)
	{
	}

	std::string PacketQuit::toString() const
	{
		return "Packet QUIT";
	}

	PacketGameQuit::PacketGameQuit() :
		opcode(OPCODE_GAME_QUIT)
	{
	}

	std::string PacketGameQuit::toString() const
	{
		return "Packet GAME_QUIT";
	}

	PacketDesyncDetected::PacketDesyncDetected(unsigned myChecksum, unsigned yourChecksum, unsigned frameId) :
		opcode(OPCODE_DESYNC_DETECTED),
		computedChecksum(myChecksum),
		receivedChecksum(yourChecksum),
		frameId(frameId)
	{
	}

	std::string PacketDesyncDetected::toString() const
	{
		char buffer1[9];
		char buffer2[9];

		sprintf(buffer1, "%x", this->computedChecksum);
		sprintf(buffer2, "%x", this->receivedChecksum);
		return "Packet DESYNC_DETECTED:"
			" frameId " + std::to_string(this->frameId) +
			" computedChecksum " + buffer1 +
			" receivedChecksum " + buffer2;
	}

	PacketTimeSync::PacketTimeSync() :
		opcode(OPCODE_TIME_SYNC)
	{
	}

	std::string PacketTimeSync::toString() const
	{
		std::string result = "Packet TIME_SYNC: " + std::to_string(this->nbDiff) + " diffs from frame " + std::to_string(this->frameId) + " expecting " + std::to_string(this->lastRecvFrameId);

		result.reserve(result.size() + 4 + this->nbDiff * 12);
		for (unsigned i = 0; i < this->nbDiff; i++) {
			result += " [";
			result += std::to_string(i);
			result += "] ";
			result += std::to_string(this->timeDiff[i]);
		}
		return result;
	}

	std::shared_ptr<PacketTimeSync> PacketTimeSync::create(std::list<std::pair<unsigned int, long long int>> &diffs, unsigned int lastRecvFrameId)
	{
		void *buffer = new char[diffs.size() * sizeof(*PacketTimeSync::timeDiff) + sizeof(PacketTimeSync)];
		auto *packet = new(buffer) PacketTimeSync();
		size_t i = 0;

		my_assert(!diffs.empty());
		my_assert((void *)buffer == (void *)packet);
		packet->lastRecvFrameId = lastRecvFrameId;
		packet->frameId = diffs.front().first;
		packet->nbDiff = diffs.size();
		for (auto &input : diffs) {
			packet->timeDiff[i] = input.second;
			i++;
		}
		return {packet, deleteArray};
	}

	size_t PacketTimeSync::getSize()
	{
		return this->nbDiff * sizeof(*this->timeDiff) + sizeof(*this);
	}

	PacketReplayRequest::PacketReplayRequest(unsigned gameId, unsigned int frame, bool allowState) :
		opcode(OPCODE_REPLAY_REQUEST),
		frame(frame),
		allowState(allowState)
	{
	}

	std::string PacketReplayRequest::toString() const
	{
		return "Packet REPLAY_REQUEST:"
		       " frame " + std::to_string(this->frame) +
		       " allowState " + (this->allowState ? "true" : "false");
	}

	PacketReplayList::PacketReplayList() :
		opcode(OPCODE_REPLAY_LIST)
	{
	}

	std::string PacketReplayList::toString() const
	{
		return "Packet REPLAY_LIST:"
		       " nbEntries " + std::to_string(this->nbEntries);
	}

	size_t PacketReplayList::getSize()
	{
		return sizeof(*this) + this->nbEntries * sizeof(*this->gameIds);
	}

	std::shared_ptr<PacketReplayList> PacketReplayList::create(const std::vector<unsigned int> &ids)
	{
		void *buffer = new char[ids.size() * sizeof(*PacketReplayList::gameIds) + sizeof(PacketReplayList)];
		auto *packet = new(buffer) PacketReplayList();

		static_assert(sizeof(*ids.data()) == sizeof(*PacketReplayList::gameIds));
		my_assert((void *)buffer == (void *)packet);
		packet->nbEntries = ids.size();
		memcpy(packet->gameIds, ids.data(), ids.size() * sizeof(*ids.data()));
		return {packet, deleteArray};
	}

	PacketReplayListRequest::PacketReplayListRequest() :
		opcode(OPCODE_REPLAY_LIST_REQUEST)
	{
	}

	std::string PacketReplayListRequest::toString() const
	{
		return "Packet REPLAY_LIST_REQUEST";
	}

	std::string Packet::toString() const
	{
		switch (this->opcode) {
		case OPCODE_HELLO:
			return this->hello.toString();
		case OPCODE_OLLEH:
			return this->olleh.toString();
		case OPCODE_REDIRECT:
			return this->redirect.toString();
		case OPCODE_PUNCH:
			return this->punch.toString();
		case OPCODE_PING:
			return this->ping.toString();
		case OPCODE_PONG:
			return this->pong.toString();
		case OPCODE_GAME_FRAME:
			return this->gameFrame.toString();
		case OPCODE_INIT_REQUEST:
			return this->initRequest.toString();
		case OPCODE_INIT_SUCCESS:
			return this->initSuccess.toString();
		case OPCODE_ERROR:
			return this->error.toString();
		case OPCODE_DELAY_UPDATE:
			return this->delayUpdate.toString();
		case OPCODE_MENU_SWITCH:
			return this->menuSwitch.toString();
		case OPCODE_SYNC_TEST:
			return this->syncTest.toString();
		case OPCODE_STATE:
			return this->state.toString();
		case OPCODE_REPLAY:
			return this->replay.toString();
		case OPCODE_QUIT:
			return this->quit.toString();
		case OPCODE_GAME_START:
			return this->gameStart.toString();
		case OPCODE_GAME_QUIT:
			return this->gameQuit.toString();
		case OPCODE_DESYNC_DETECTED:
			return this->desyncDetected.toString();
		case OPCODE_TIME_SYNC:
			return this->timeSync.toString();
		case OPCODE_REPLAY_REQUEST:
			return this->replayRequest.toString();
		case OPCODE_REPLAY_LIST:
			return this->replayList.toString();
		case OPCODE_REPLAY_LIST_REQUEST:
			return this->replayListRequest.toString();
		default:
			return "Packet UNKNOWN (Opcode " + std::to_string(this->opcode) + ")";
		}
	}
}
