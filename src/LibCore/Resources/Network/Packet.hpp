//
// Created by PinkySmile on 17/08/2022.
//

#ifndef SOFGV_PACKET_HPP
#define SOFGV_PACKET_HPP


#include <memory>
#include <string>
#include <vector>
#include <list>
#include <array>

namespace SpiralOfFate
{
	//TODO: Handle endianness properly
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
		OPCODE_GAME_QUIT,
		OPCODE_DESYNC_DETECTED,
		OPCODE_TIME_SYNC,
		OPCODE_REPLAY_REQUEST,
		OPCODE_REPLAY_LIST,
		OPCODE_REPLAY_LIST_REQUEST,
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
		ERROR_INVALID_DATA,
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
		"Invalid data"
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
		"Quit",
		"Game quit",
		"Desync detected",
		"Time sync",
		"Replay request",
		"Replay list",
		"Replay list request"
	};

#pragma pack(push, 1)
	struct PacketHello {
	private:
		Opcode opcode;
		uint32_t magic;

	public:
		uint32_t targetIp;
		uint32_t targetPort;

		static unsigned computeMagic(const char *version);

		PacketHello(const char *version, unsigned targetIp, unsigned targetPort);
		unsigned int getMagic() const;
		std::string toString() const;
	};

	struct PacketOlleh {
	private:
		Opcode opcode;

	public:
		PacketOlleh();
		std::string toString() const;
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
		);
		std::string toString() const;
	};

	struct PacketRedirect {
	private:
		Opcode opcode;

	public:
		uint32_t targetIp;
		uint32_t targetPort;

		PacketRedirect(unsigned targetIp, unsigned targetPort);
		std::string toString() const;
	};

	struct PacketPunch {
	private:
		Opcode opcode;

	public:
		uint32_t targetIp;
		uint32_t targetPort;

		PacketPunch(unsigned targetIp, unsigned targetPort);
		std::string toString() const;
	};

	struct PacketPing {
	private:
		Opcode opcode;

	public:
		uint32_t seqId;

		PacketPing(unsigned seqId);
		std::string toString() const;
	};

	struct PacketPong {
	private:
		Opcode opcode;

	public:
		uint32_t seqId;

		PacketPong(unsigned seqId);
		std::string toString() const;
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

		PacketGameFrame();
	public:
		uint32_t gameId;
		uint32_t frameId;
		uint32_t lastRecvFrameId;
		uint32_t nbInputs;
		PacketInput inputs[];

		static std::shared_ptr<PacketGameFrame> create(std::list<std::pair<unsigned, PacketInput>> &inputs, uint32_t lastRecvFrameId, uint32_t gameId);
		size_t getSize();
		std::string toString() const;
	};

	struct PacketInitRequest {
	private:
		Opcode opcode;

	public:
		int8_t spectator;
		char playerName[32];
		char gameVersion[32];

		PacketInitRequest(const char *name, const char *version, bool spectator);
		std::string toString() const;
	};

	struct PacketInitSuccess {
	private:
		Opcode opcode;

	public:
		char player1Name[32];
		char player2Name[32];
		char gameVersion[32];

		PacketInitSuccess(const char *name1, const char *name2, const char *version);
		std::string toString() const;
	};

	struct PacketError {
	private:
		Opcode opcode;

	public:
		uint32_t code;
		Opcode offendingPacket;
		uint64_t offendingPacketSize;

		PacketError(unsigned code, Opcode offendingPacket, size_t offendingPacketSize);
		std::string toString() const;
	};

	struct PacketDelayUpdate {
	private:
		Opcode opcode;

	public:
		uint32_t frameId;
		uint32_t newDelay;

		PacketDelayUpdate(unsigned newDelay, unsigned frameId);
		std::string toString() const;
	};

	struct PacketMenuSwitch {
	private:
		Opcode opcode;

	public:
		uint32_t menuId;
		uint32_t opMenuId;

		PacketMenuSwitch(unsigned menuId, unsigned opMenuId);
		std::string toString() const;
	};

	struct PacketSyncTest {
	private:
		Opcode opcode;

	public:
		uint32_t frameId;
		uint32_t stateChecksum;

		PacketSyncTest(unsigned stateChecksum, unsigned frameId);
		std::string toString() const;
	};

	struct PacketState {
	private:
		Opcode opcode;

		PacketState();
	public:
		uint32_t compressedSize;
		uint8_t compressedData[0];

		std::shared_ptr<PacketState> create(void *data, size_t size);
		std::string toString() const;
	};

	struct PacketReplay {
	private:
		Opcode opcode;

		PacketReplay();
	public:
		uint32_t gameId;
		uint32_t frameId;
		uint32_t lastFrameId;
		uint32_t nbInputs;
		uint32_t compressedSize;
		uint8_t compressedData[0];

		unsigned getSize() const;
		std::string toString() const;

		static std::shared_ptr<PacketReplay> create(void *data, size_t size, unsigned gameId, unsigned frameId, unsigned lastFrameId, unsigned nbInputs);
	};

	struct PacketQuit {
	private:
		Opcode opcode;

	public:
		PacketQuit();
		std::string toString() const;
	};

	struct PacketGameQuit {
	private:
		Opcode opcode;

	public:
		PacketGameQuit();
		std::string toString() const;
	};

	struct PacketDesyncDetected {
	private:
		Opcode opcode;

	public:
		uint32_t computedChecksum;
		uint32_t receivedChecksum;
		uint32_t frameId;

		PacketDesyncDetected(unsigned myChecksum, unsigned yourChecksum, unsigned frameId);
		std::string toString() const;
	};

	struct PacketTimeSync {
	private:
		Opcode opcode;

		PacketTimeSync();

	public:
		uint32_t frameId;
		uint32_t lastRecvFrameId;
		uint32_t nbDiff;
		int64_t timeDiff[];

		size_t getSize();
		std::string toString() const;

		static std::shared_ptr<PacketTimeSync> create(std::list<std::pair<unsigned, long long>> &diffs, unsigned lastRecvFrameId);
	};

	struct PacketReplayRequest {
	private:
		Opcode opcode;

	public:
		uint32_t gameId;
		uint32_t frame;
		uint8_t allowState;

		PacketReplayRequest(unsigned gameId, unsigned frame, bool allowState);
		std::string toString() const;
	};

	struct PacketReplayList {
	private:
		Opcode opcode;

		PacketReplayList();
	public:
		uint32_t nbEntries;
		uint32_t gameIds[0];

		size_t getSize();
		std::string toString() const;

		static std::shared_ptr<PacketReplayList> create(const std::vector<unsigned> &ids);
	};

	struct PacketReplayListRequest {
	private:
		Opcode opcode;

	public:
		PacketReplayListRequest();
		std::string toString() const;
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
		PacketGameQuit gameQuit;
		PacketDesyncDetected desyncDetected;
		PacketTimeSync timeSync;
		PacketReplayRequest replayRequest;
		PacketReplayList replayList;
		PacketReplayListRequest replayListRequest;

		std::string toString() const;
	};
#pragma pack(pop)
}


#endif //SOFGV_PACKET_HPP
