//
// Created by PinkySmile on 17/08/2022.
//

#ifndef SOFGV_PACKET_HPP
#define SOFGV_PACKET_HPP


#include <memory>
#include <cstdlib>
#include <string>
#include <cstring>
#include <vector>
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	constexpr unsigned magicStart = 0x9DAC3E63;

	enum Opcode : unsigned char {
		OPCODE_HELLO,
		OPCODE_OLLEH,
		OPCODE_REDIRECT,
		OPCODE_PUNCH,
		OPCODE_PING,
		OPCODE_PONG,
		OPCODE_GAME_START,
		OPCODE_GAME_FRAME,
		OPCODE_INIT_REQUEST,
		OPCODE_INIT_SUCCESS,
		OPCODE_ERROR,
		OPCODE_DELAY_UPDATE,
		OPCODE_MENU_SWITCH,
		OPCODE_SYNC_TEST,
		OPCODE_STATE,
		OPCODE_REPLAY,
		OPCODE_QUIT,
		OPCODE_COUNT
	};

	enum Error : uint8_t {
		ERROR_NOT_IMPLEMENTED,
		ERROR_UNEXPECTED_OPCODE,
		ERROR_INVALID_OPCODE,
		ERROR_SIZE_MISMATCH,
		ERROR_VERSION_MISMATCH,
		ERROR_GAME_NOT_STARTED,
		ERROR_GAME_ALREADY_STARTED,
		ERROR_SPECTATORS_DISABLED,
		ERROR_BLACKLISTED,
		ERROR_DESYNC_DETECTED,
		ERROR_COUNT
	};

	constexpr std::array<const char *, ERROR_COUNT> errorStrings{
		"Not implemented",
		"Unexpected opcode",
		"Invalid opcode",
		"Size mismatch",
		"Version mismatch",
		"Game not started",
		"Game already started",
		"Spectators disabled",
		"Blacklisted",
		"Desync detected"
	};
	constexpr std::array<const char *, OPCODE_COUNT> opcodeStrings{
		"Hello",
		"Olleh",
		"Redirect",
		"Punch",
		"Ping",
		"Pong",
		"Game start",
		"Game frame",
		"Init request",
		"Init success",
		"Init error",
		"Delay update",
		"Menu switch",
		"Sync test",
		"State",
		"Replay",
		"Quit"
	};

#pragma pack(push, 1)
	struct PacketHello {
	private:
		Opcode opcode;
		uint32_t magic;

	public:
		uint32_t targetIp;
		uint32_t targetPort;

		static unsigned computeMagic(const char *version)
		{
			unsigned magic = magicStart;

			for (int i = 0; version[i]; i++) {
				magic <<= 1;
				magic += version[i];
			}
			return magic;
		}

		PacketHello(const char *version, unsigned targetIp, unsigned targetPort) :
			opcode(OPCODE_HELLO),
			magic(computeMagic(version)),
			targetIp(targetIp),
			targetPort(targetPort)
		{
		}

		unsigned int getMagic() const
		{
			return this->magic;
		}

		std::string toString() const
		{
			return "Packet HELLO: magic " + std::to_string(this->magic) + " targetIp " + std::to_string(((char *)&this->targetIp)[0]) + "." + std::to_string(((char *)&this->targetIp)[1]) + "." + std::to_string(((char *)&this->targetIp)[2]) + "." + std::to_string(((char *)&this->targetIp)[3]) + " targetPort " + std::to_string(this->targetPort);
		}
	};

	struct PacketOlleh {
	private:
		Opcode opcode;

	public:
		PacketOlleh() : opcode(OPCODE_OLLEH) {}

		std::string toString() const
		{
			return "Packet OLLEH";
		}
	};

	struct PacketGameStart {
	private:
		Opcode opcode;

	public:
		uint32_t seed;
		uint32_t p1chr;
		uint32_t p1pal;
		uint32_t p2chr;
		uint32_t p2pal;
		uint32_t stage;
		uint32_t platformConfig;

		PacketGameStart(
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
		{}

		std::string toString() const
		{
			return "Packet GAMESTART"
			       "seed" + std::to_string(this->seed) +
			       "p1chr" + std::to_string(this->p1chr) +
			       "p1pal" + std::to_string(this->p1pal) +
			       "p2chr" + std::to_string(this->p2chr) +
			       "p2pal" + std::to_string(this->p2pal) +
			       "stage" + std::to_string(this->stage) +
			       "platformConfig" + std::to_string(this->platformConfig);
		}
	};

	struct PacketRedirect {
	private:
		Opcode opcode;

	public:
		uint32_t targetIp;
		uint32_t targetPort;

		PacketRedirect(unsigned targetIp, unsigned targetPort) :
			opcode(OPCODE_REDIRECT),
			targetIp(targetIp),
			targetPort(targetPort)
		{}

		std::string toString() const
		{
			return "Packet REDIRECT: targetIp " + std::to_string(((char *)&this->targetIp)[0]) + "." + std::to_string(((char *)&this->targetIp)[1]) + "." + std::to_string(((char *)&this->targetIp)[2]) + "." + std::to_string(((char *)&this->targetIp)[3]) + " targetPort " + std::to_string(this->targetPort);
		}
	};

	struct PacketPunch {
	private:
		Opcode opcode;

	public:
		uint32_t targetIp;
		uint32_t targetPort;

		PacketPunch(unsigned targetIp, unsigned targetPort) :
			opcode(OPCODE_PUNCH),
			targetIp(targetIp),
			targetPort(targetPort)
		{}

		std::string toString() const
		{
			return "Packet PUNCH: targetIp " + std::to_string(((char *)&this->targetIp)[0]) + "." + std::to_string(((char *)&this->targetIp)[1]) + "." + std::to_string(((char *)&this->targetIp)[2]) + "." + std::to_string(((char *)&this->targetIp)[3]) + " targetPort " + std::to_string(this->targetPort);
		}
	};

	struct PacketPing {
	private:
		Opcode opcode;

	public:
		uint32_t seqId;

		PacketPing(unsigned seqId) :
			opcode(OPCODE_PING),
			seqId(seqId)
		{}

		std::string toString() const
		{
			return "Packet PING: seqId " + std::to_string(this->seqId);
		}
	};

	struct PacketPong {
	private:
		Opcode opcode;

	public:
		uint32_t seqId;

		PacketPong(unsigned seqId) :
			opcode(OPCODE_PONG),
			seqId(seqId)
		{}

		std::string toString() const
		{
			return "Packet PONG: seqId " + std::to_string(this->seqId);
		}
	};

	struct PacketInput {
		bool n : 1;
		bool m : 1;
		bool v : 1;
		bool s : 1;
		bool a : 1;
		bool d : 1;
		char _h : 2;
		char _v : 2;
	};

	struct PacketGameFrame {
	private:
		Opcode opcode;

		PacketGameFrame() :
			opcode(OPCODE_GAME_FRAME)
		{}
	public:
		uint32_t frameId;
		uint32_t nbInputs;
		PacketInput inputs[];

		static std::shared_ptr<PacketGameFrame> create(std::list<std::pair<unsigned, PacketInput>> &inputs)
		{
			void *buffer = malloc(inputs.size() * sizeof(*PacketGameFrame::inputs) + sizeof(PacketGameFrame));
			auto *packet = new(buffer) PacketGameFrame();
			size_t i = 0;

			my_assert(!inputs.empty());
			packet->frameId = inputs.front().first;
			packet->nbInputs = inputs.size();
			for (auto &input : inputs) {
				packet->inputs[i] = input.second;
				i++;
			}
			return std::shared_ptr<PacketGameFrame>(packet, free);
		}

		std::string toString() const
		{
			return "Packet GAMEFRAME: " + std::to_string(this->nbInputs) + " inputs from frame " + std::to_string(this->frameId);
		}
	};

	struct PacketInitRequest {
	private:
		Opcode opcode;

	public:
		int8_t spectator;
		char playerName[32];
		char gameVersion[32];

		PacketInitRequest(const char *name, const char *version, bool spectator) :
			opcode(OPCODE_INIT_REQUEST),
			spectator(spectator)
		{
			my_assert(strlen(name) < sizeof(this->playerName));
			my_assert(strlen(version) < sizeof(this->gameVersion));
			strncpy(this->playerName, name, sizeof(this->playerName));
			strncpy(this->gameVersion, version, sizeof(this->gameVersion));
		}

		std::string toString() const
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
	};

	struct PacketInitSuccess {
	private:
		Opcode opcode;

	public:
		char playerName[32];
		char gameVersion[32];

		PacketInitSuccess(const char *name, const char *version) :
			opcode(OPCODE_INIT_SUCCESS)
		{
			strncpy(this->playerName, name, sizeof(this->playerName));
			strncpy(this->gameVersion, version, sizeof(this->gameVersion));
		}

		std::string toString() const
		{
			char buffer[130];

			memset(buffer, 0, sizeof(buffer));
			strcpy(buffer, "Packet INITSUCCESS: From player ");
			strncat(buffer, this->playerName, sizeof(this->playerName));
			strcat(buffer, " using game version ");
			strncat(buffer, this->gameVersion, sizeof(this->gameVersion));
			return buffer;
		}
	};

	struct PacketError {
	private:
		Opcode opcode;

	public:
		uint32_t code;
		Opcode offendingPacket;
		uint64_t offendingPacketSize;

		PacketError(unsigned code, Opcode offendingPacket, size_t offendingPacketSize) :
			opcode(OPCODE_ERROR),
			code(code),
			offendingPacket(offendingPacket),
			offendingPacketSize(offendingPacketSize)
		{}

		std::string toString() const
		{
			return "Packet ERROR: code " + std::to_string(this->code) + " (" + (this->code < errorStrings.size() ? errorStrings[this->code] : "Unknown code") + "). "
			       "Faulting packet: " + std::to_string(this->offendingPacket) + " (" + (this->offendingPacket < opcodeStrings.size() ? opcodeStrings[this->offendingPacket] : "Unknown opcode") + ") of size " + std::to_string(this->offendingPacketSize) + " bytes.";
		}
	};

	struct PacketDelayUpdate {
	private:
		Opcode opcode;

	public:
		uint32_t frameId;
		uint32_t newDelay;

		//TODO: Implement this so that it desyncs if both players don't use the same delay.
		//      This will prevent cheating by sending this packet over to the client so that
		//      the Player 2 thinks the delay is higher and use a higher delay than Player 1.
		//      Send the inputs for the current frame and add delay after receiving?
		PacketDelayUpdate(unsigned newDelay, unsigned frameId) :
			opcode(OPCODE_DELAY_UPDATE),
			frameId(frameId),
			newDelay(newDelay)
		{}

		std::string toString() const
		{
			return "Packet DELAYUPDATE: delay " + std::to_string(this->newDelay) + " frameId " + std::to_string(this->frameId);
		}
	};

	struct PacketMenuSwitch {
	private:
		Opcode opcode;

	public:
		uint32_t menuId;

		PacketMenuSwitch(unsigned menuId) :
			opcode(OPCODE_MENU_SWITCH),
			menuId(menuId)
		{}

		std::string toString() const
		{
			return "Packet MENUSWITCH: menuId " + std::to_string(this->menuId);
		}
	};

	struct PacketSyncTest {
	private:
		Opcode opcode;

	public:
		uint32_t frameId;
		uint32_t stateChecksum;

		PacketSyncTest(unsigned stateChecksum, unsigned frameId) :
			opcode(OPCODE_SYNC_TEST),
			frameId(frameId),
			stateChecksum(stateChecksum)
		{}

		std::string toString() const
		{
			return "Packet SYNCTEST: stateChecksum " + std::to_string(this->stateChecksum) + " frameId " + std::to_string(this->frameId);
		}
	};

	struct PacketState {
	private:
		Opcode opcode;

		PacketState() :
			opcode(OPCODE_STATE)
		{}
	public:
		uint32_t compressedSize;
		uint8_t compressedData[0];

		std::shared_ptr<PacketState> create(void *data, size_t size)
		{
			void *buffer = malloc(size + 1);
			auto *packet = new(buffer) PacketState();

			//TODO: Compress the data
			return std::shared_ptr<PacketState>(packet, free);
		}

		std::string toString() const
		{
			return "Packet STATE: " + std::to_string(this->compressedSize) + " compressed bytes";
		}
	};

	struct PacketReplay {
	private:
		Opcode opcode;

		PacketReplay() :
			opcode(OPCODE_REPLAY)
		{}
	public:
		uint32_t compressedSize;
		uint8_t compressedData[0];

		std::shared_ptr<PacketReplay> create(void *data, size_t size)
		{
			void *buffer = malloc(size + 5);
			auto *packet = new(buffer) PacketReplay();

			//TODO: Compress the data
			return std::shared_ptr<PacketReplay>(packet, free);
		}

		std::string toString() const
		{
			return "Packet REPLAY: " + std::to_string(this->compressedSize) + " compressed bytes";
		}
	};

	struct PacketQuit {
	private:
		Opcode opcode;

	public:
		PacketQuit() : opcode(OPCODE_QUIT) {}

		std::string toString() const
		{
			return "Packet QUIT";
		}
	};

	union Packet {
		Opcode opcode;
		PacketHello hello;
		PacketOlleh olleh;
		PacketRedirect redirect;
		PacketPunch punch;
		PacketPing ping;
		PacketPong pong;
		PacketGameFrame gameFrame;
		PacketInitRequest initRequest;
		PacketInitSuccess initSuccess;
		PacketError error;
		PacketDelayUpdate delayUpdate;
		PacketMenuSwitch menuSwitch;
		PacketSyncTest syncTest;
		PacketState state;
		PacketReplay replay;
		PacketQuit quit;
		PacketGameStart gameStart;

		std::string toString() const
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
			default:
				return "Packet UNKNOWN";
			}
		}
	};
#pragma pack(pop)
}


#endif //SOFGV_PACKET_HPP
