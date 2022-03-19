//
// Created by PinkySmile on 18/09/2021
//

#include "Game.hpp"
#include <sstream>
#include <memory>
#include <iomanip>
#include "../Scenes/NetplayInGame.hpp"
#include "../Logger.hpp"

namespace Battle
{
	Game *game = nullptr;

	Game::Game() :
		random(std::random_device()())
	{
		assert(!game);
		game = this;
		try {
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/039.wav"), BASICSOUND_MENU_MOVE);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/041.wav"), BASICSOUND_MENU_CANCEL);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/040.wav"), BASICSOUND_MENU_CONFIRM);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/022.wav"), BASICSOUND_KNOCKDOWN);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/044.wav"), BASICSOUND_KNOCK_OUT);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/038.wav"), BASICSOUND_GUARD_BREAK);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/047.wav"), BASICSOUND_GUARD_RECOVER);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/055.wav"), BASICSOUND_OVERDRIVE);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/036.wav"), BASICSOUND_OVERDRIVE_RECOVER);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/043.wav"), BASICSOUND_HIGH_JUMP);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/030.wav"), BASICSOUND_LAND);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/025.wav"), BASICSOUND_COUNTER_HIT);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/031.wav"), BASICSOUND_DASH);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/020.wav"), BASICSOUND_BLOCK);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/021.wav"), BASICSOUND_WRONG_BLOCK);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/053.wav"), BASICSOUND_SPELLFLASH);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/035.wav"), BASICSOUND_ROMAN_CANCEL);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/033.wav"), BASICSOUND_ULTIMATE);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/054.wav"), BASICSOUND_PARRY);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/072.wav"), BASICSOUND_BEST_PARRY);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/022.wav"), BASICSOUND_WALL_BOUNCE);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/022.wav"), BASICSOUND_GROUND_SLAM);
		} catch (...) {
			game = nullptr;
			throw;
		}
	}

	namespace GGPONetplay
	{
		/*
		 * Simple checksum function stolen from wikipedia:
		 *
		 *   http://en.wikipedia.org/wiki/Fletcher%27s_checksum
		 */
		int fletcher32_checksum(short *data, size_t len)
		{
			int sum1 = 0xffff, sum2 = 0xffff;

			while (len) {
				size_t tlen = len > 360 ? 360 : len;
				len -= tlen;
				do {
					sum1 += *data++;
					sum2 += sum1;
				} while (--tlen);
				sum1 = (sum1 & 0xffff) + (sum1 >> 16);
				sum2 = (sum2 & 0xffff) + (sum2 >> 16);
			}

			/* Second reduction step to reduce sums to 16 bits */
			sum1 = (sum1 & 0xffff) + (sum1 >> 16);
			sum2 = (sum2 & 0xffff) + (sum2 >> 16);
			return sum2 << 16 | sum1;
		}

		bool __cdecl startGame(const char *)
		{
			game->battleRandom.seed(0);
			game->networkMgr.beginSession();
			return true;
		}

		bool __cdecl saveState(unsigned char **buffer, int *len, int *checksum, int)
		{
			std::stringstream str;

			game->networkMgr.save(reinterpret_cast<void **>(buffer), len);
			*checksum = fletcher32_checksum(reinterpret_cast<short *>(*buffer), *len / 2);
			game->logger.debug("Saved frame to buffer @" + std::to_string((ptrdiff_t)*buffer) + " (len " + std::to_string(*len) + " bytes, checksum " + std::to_string(*checksum) + ")");
#ifdef _DEBUG
			str << "Data dump" << std::endl;
			for (int i = 0; i < *len; i += 0x10) {
				int j, k;

				str << std::hex << std::setfill('0') << std::setw(4) << i << " " << std::setfill('0') << std::setw(4) << i + 0xF << " | ";
				for (j = i, k = 0; k < 0x10 && j < *len; j++, k++)
					str << std::hex << std::setfill('0') << std::setw(2) << (int)(*buffer)[j] << " ";
				for (; k < 0x10; k++)
					str << "   ";
				str << "| ";
				for (j = i, k = 0; k < 0x10 && j < *len; j++, k++)
					if (std::isprint((*buffer)[j]))
						str << (*buffer)[j];
					else
						str << '.';
				if (i + 0x10 < *len)
					str << std::endl;
			}
			game->logger.debug(str.str());
#endif
			return true;
		}

		bool __cdecl loadState(unsigned char *buffer, int len)
		{
			game->networkMgr.load(buffer);
			game->logger.debug("Loaded frame from buffer @" + std::to_string((ptrdiff_t)buffer) + " (len " + std::to_string(len) + " bytes)");
			return true;
		}

		bool __cdecl logState(char *, unsigned char *, int)
		{
			return true;
		}

		void __cdecl freeBuffer(void *buffer)
		{
			NetManager::free(buffer);
			game->logger.debug("Deleted buffer @" + std::to_string((ptrdiff_t)buffer));
		}

		bool __cdecl updateGame(int)
		{
			game->networkMgr.advanceState();
			return true;
		}

		bool __cdecl onEvent(GGPOEvent *info)
		{
			game->networkMgr.consumeEvent(info);
			return true;
		}
	}
}

int __attribute__((noreturn)) _my_assert(const char *expr, const char *file, int line)
{
	auto err = "Debug Assertion " + std::string(expr) + " failed at " + file + " line " +std::to_string(line);

	Battle::game->logger.fatal(err);
	throw AssertionFailedException(err);
}