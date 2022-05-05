//
// Created by Gegel85 on 27/04/2022.
//

#ifndef SOFGV_PROTOCOL_HPP
#define SOFGV_PROTOCOL_HPP


#include <memory>

namespace SpiralOfFateNet
{
	constexpr unsigned magicStart = 0x9DAC3E63;

	enum Opcode : unsigned char {
		OPCODE_HELLO,
		OPCODE_OLLEH,
		OPCODE_REDIRECT,
		OPCODE_PUNCH,
		OPCODE_PING,
		OPCODE_PONG,
		OPCODE_GAME_FRAME,
		OPCODE_INIT_REQUEST,
		OPCODE_INIT_SUCCESS,
		OPCODE_INIT_ERROR,
		OPCODE_DELAY_UPDATE,
		OPCODE_MENU_SWITCH,
		OPCODE_SYNC_TEST,
		OPCODE_STATE,
		OPCODE_REPLAY,
		OPCODE_QUIT,
	};

	enum Error : unsigned char {
		ERROR_GAME_NOT_STARTED,
		ERROR_SPECTATORS_DISABLED,
		ERROR_BLACKLISTED
	};

#pragma pack(push, 1)
	struct PacketHello {
	private:
		Opcode opcode;
		unsigned magic;

	public:
		unsigned targetIp;
		unsigned targetPort;

		PacketHello(const char *version, unsigned targetIp, unsigned targetPort) :
			opcode(OPCODE_HELLO),
			magic(magicStart),
			targetIp(targetIp),
			targetPort(targetPort)
		{
			for (int i = 0; version[i]; i++) {
				this->magic <<= 1;
				this->magic += version[i];
			}
		}

		unsigned int getMagic() const
		{
			return this->magic;
		}
	};

	struct PacketOlleh {
	private:
		Opcode opcode;

	public:
		PacketOlleh() : opcode(OPCODE_OLLEH) {}
	};

	struct PacketRedirect {
	private:
		Opcode opcode;

	public:
		unsigned targetIp;
		unsigned targetPort;

		PacketRedirect(unsigned targetIp, unsigned targetPort) :
			opcode(OPCODE_REDIRECT),
			targetIp(targetIp),
			targetPort(targetPort)
		{}
	};

	struct PacketPunch {
	private:
		Opcode opcode;

	public:
		unsigned targetIp;
		unsigned targetPort;

		PacketPunch(unsigned targetIp, unsigned targetPort) :
			opcode(OPCODE_PUNCH),
			targetIp(targetIp),
			targetPort(targetPort)
		{}
	};

	struct PacketPing {
	private:
		Opcode opcode;

	public:
		unsigned seqId;

		PacketPing(unsigned seqId) :
			opcode(OPCODE_PING),
			seqId(seqId)
		{}
	};

	struct PacketPong {
	private:
		Opcode opcode;

	public:
		unsigned seqId;

		PacketPong(unsigned seqId) :
			opcode(OPCODE_PONG),
			seqId(seqId)
		{}
	};

	struct PacketGameFrame {
	private:
		Opcode opcode;

		PacketGameFrame() :
			opcode(OPCODE_GAME_FRAME)
		{}
	public:
		unsigned frameIdStart;
		unsigned frameIdEnd;

		std::shared_ptr<PacketGameFrame> create(void *data, size_t size)
		{
			void *buffer = malloc(size + 1);
			auto *packet = new(buffer) PacketGameFrame();

			memcpy(buffer + 1, data, size);
			return std::shared_ptr<PacketGameFrame>(packet, free);
		}
	};

	struct PacketInitRequest {
	private:
		Opcode opcode;

		PacketInitRequest() :
			opcode(OPCODE_INIT_REQUEST)
		{}
	public:
		std::shared_ptr<PacketInitRequest> create(void *data, size_t size)
		{
			void *buffer = malloc(size + 1);
			auto *packet = new(buffer) PacketInitRequest();

			memcpy(buffer + 1, data, size);
			return std::shared_ptr<PacketInitRequest>(packet, free);
		}
	};

	struct PacketInitSuccess {
	private:
		Opcode opcode;

		PacketInitSuccess() :
			opcode(OPCODE_INIT_SUCCESS)
		{}
	public:
		std::shared_ptr<PacketInitSuccess> create(void *ldata, size_t lsize, void *rdata, size_t rsize)
		{
			void *buffer = malloc(lsize + rsize + 1);
			auto *packet = new(buffer) PacketInitSuccess();

			memcpy(buffer + 1, ldata, lsize);
			memcpy(buffer + 1 + lsize, rdata, rsize);
			return std::shared_ptr<PacketInitSuccess>(packet, free);
		}
	};

	struct PacketInitError {
	private:
		Opcode opcode;

	public:
		unsigned code;

		PacketInitError(unsigned code) :
			opcode(OPCODE_INIT_ERROR),
			code(code)
		{}
	};

	struct PacketDelayUpdate {
	private:
		Opcode opcode;

	public:
		unsigned newDelay;

		PacketDelayUpdate(unsigned newDelay) :
			opcode(OPCODE_DELAY_UPDATE),
			newDelay(newDelay)
		{}
	};

	struct PacketMenuSwitch {
	private:
		Opcode opcode;

	public:
		unsigned menuId;

		PacketMenuSwitch(unsigned menuId) :
			opcode(OPCODE_MENU_SWITCH),
			menuId(menuId)
		{}
	};

	struct PacketSyncTest {
	private:
		Opcode opcode;

	public:
		unsigned stateChecksum;

		PacketSyncTest(unsigned stateChecksum) :
			opcode(OPCODE_SYNC_TEST),
			stateChecksum(stateChecksum)
		{}
	};

	struct PacketState {
	private:
		Opcode opcode;

		PacketState() :
			opcode(OPCODE_STATE)
		{}
	public:
		std::shared_ptr<PacketState> create(void *data, size_t size)
		{
			void *buffer = malloc(size + 1);
			auto *packet = new(buffer) PacketState();

			memcpy(buffer + 1, data, size);
			return std::shared_ptr<PacketState>(packet, free);
		}
	};

	struct PacketReplay {
	private:
		Opcode opcode;

		PacketReplay() :
			opcode(OPCODE_REPLAY)
		{}
	public:
		unsigned compressedSize;
		char compressedData[0];

		std::shared_ptr<PacketReplay> create(void *data, size_t size)
		{
			void *buffer = malloc(size + 5);
			auto *packet = new(buffer) PacketReplay();

			packet->compressedSize = size;
			memcpy(this->compressedData, data, size);
			return std::shared_ptr<PacketReplay>(packet, free);
		}
	};

	struct PacketQuit {
	private:
		Opcode opcode;

	public:
		PacketQuit() : opcode(OPCODE_QUIT) {}
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
		PacketInitError initError;
		PacketDelayUpdate delayUpdate;
		PacketMenuSwitch menuSwitch;
		PacketSyncTest syncTest;
		PacketState state;
		PacketReplay replay;
		PacketQuit quit;
	};
#pragma pack(pop)
}


#endif //SOFGV_PROTOCOL_HPP
