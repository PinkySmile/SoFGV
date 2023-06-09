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


namespace SpiralOfFate
{
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
		void *buffer = malloc(inputs.size() * sizeof(*PacketGameFrame::inputs) + sizeof(PacketGameFrame));
		auto *packet = new(buffer) PacketGameFrame();
		size_t i = 0;

		my_assert(!inputs.empty());
		packet->lastRecvFrameId = lastRecvFrameId;
		packet->frameId = inputs.front().first;
		packet->nbInputs = inputs.size();
		packet->gameId = gameId;
		for (auto &input : inputs) {
			packet->inputs[i] = input.second;
			i++;
		}
		return std::shared_ptr<PacketGameFrame>(packet, free);
	}

	std::string PacketGameFrame::toString() const
	{
		return "Packet GAMEFRAME: " + std::to_string(this->nbInputs) + " inputs in game " + std::to_string(this->gameId) + " from frame " + std::to_string(this->frameId) + " expecting " + std::to_string(this->lastRecvFrameId);
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

	PacketInitSuccess::PacketInitSuccess(const char *name, const char *version) :
		opcode(OPCODE_INIT_SUCCESS)
	{
		strncpy(this->playerName, name, sizeof(this->playerName));
		strncpy(this->gameVersion, version, sizeof(this->gameVersion));
	}

	std::string PacketInitSuccess::toString() const
	{
		char buffer[130];

		memset(buffer, 0, sizeof(buffer));
		strcpy(buffer, "Packet INITSUCCESS: From player ");
		strncat(buffer, this->playerName, sizeof(this->playerName));
		strcat(buffer, " using game version ");
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
		void *buffer = malloc(size + 1);
		auto *packet = new(buffer) PacketState();

		//TODO: Compress the data
		return std::shared_ptr<PacketState>(packet, free);
	}

	std::string PacketState::toString() const
	{
		return "Packet STATE: " + std::to_string(this->compressedSize) + " compressed bytes";
	}

	PacketReplay::PacketReplay() :
		opcode(OPCODE_REPLAY)
	{
	}

	std::shared_ptr<PacketReplay> PacketReplay::create(void *data, size_t size)
	{
		void *buffer = malloc(size + 5);
		auto *packet = new(buffer) PacketReplay();

		//TODO: Compress the data
		return std::shared_ptr<PacketReplay>(packet, free);
	}

	std::string PacketReplay::toString() const
	{
		return "Packet REPLAY: " + std::to_string(this->compressedSize) + " compressed bytes";
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

	PacketDesyncDetected::PacketDesyncDetected(uint32_t myChecksum, uint32_t yourChecksum, uint32_t frameId) :
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
			" computedChecksum " + std::string(buffer1) +
			" receivedChecksum " + buffer2 +
			" frameId " + std::to_string(this->frameId);
	}

	PacketTimeSync::PacketTimeSync() :
		opcode(OPCODE_TIME_SYNC)
	{

	}

	std::string PacketTimeSync::toString() const
	{
		return "Packet TIME_SYNC";
	}

	std::string Packet::toString() const
	{
		switch (opcode) {
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
		default:
			return "Packet UNKNOWN";
		}
	}
}
