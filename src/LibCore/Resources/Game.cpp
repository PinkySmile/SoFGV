//
// Created by PinkySmile on 18/09/2021
//

#include "Game.hpp"
#include <sstream>
#include <memory>
#include <iomanip>
#include "../Logger.hpp"

namespace SpiralOfFate
{
	MYDLL_API Game *game = nullptr;

	Game::Game(const std::string &loggerPath) :
		random(std::random_device()()),
		logger(loggerPath)
	{
		my_assert(!game);
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
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/061.wav"), BASICSOUND_GAME_LAUNCH);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/032.wav"), BASICSOUND_WORST_PARRY);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/022.wav"), BASICSOUND_WALL_BOUNCE);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/022.wav"), BASICSOUND_GROUND_SLAM);
		} catch (...) {
			game = nullptr;
			throw;
		}
	}

	Game::~Game()
	{
		this->logger.debug("~Game()>");
		this->scene.reset();
		this->logger.debug("~Game()<");
	}
#ifndef __ANDROID__

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
#endif
}
