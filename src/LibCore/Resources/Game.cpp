//
// Created by PinkySmile on 18/09/2021.
//

#include "Game.hpp"
#include <sstream>
#include <memory>
#include <iomanip>
#include "Logger.hpp"

namespace SpiralOfFate
{
	MYDLL_API Game *game = nullptr;

	Game::Game(const std::string &loggerPath) :
		logger(loggerPath)
	{
		my_assert(!game);
		game = this;
		try {
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/039.ogg"), BASICSOUND_MENU_MOVE);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/041.ogg"), BASICSOUND_MENU_CANCEL);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/040.ogg"), BASICSOUND_MENU_CONFIRM);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/022.ogg"), BASICSOUND_KNOCKDOWN);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/044.ogg"), BASICSOUND_KNOCK_OUT);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/038.ogg"), BASICSOUND_GUARD_BREAK);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/047.ogg"), BASICSOUND_GUARD_RECOVER);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/055.ogg"), BASICSOUND_OVERDRIVE);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/036.ogg"), BASICSOUND_OVERDRIVE_RECOVER);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/043.ogg"), BASICSOUND_HIGH_JUMP);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/030.ogg"), BASICSOUND_LAND);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/025.ogg"), BASICSOUND_COUNTER_HIT);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/031.ogg"), BASICSOUND_DASH);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/020.ogg"), BASICSOUND_BLOCK);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/021.ogg"), BASICSOUND_WRONG_BLOCK);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/053.ogg"), BASICSOUND_SPELLFLASH);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/035.ogg"), BASICSOUND_ROMAN_CANCEL);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/033.ogg"), BASICSOUND_ULTIMATE);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/054.ogg"), BASICSOUND_PARRY);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/072.ogg"), BASICSOUND_BEST_PARRY);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/061.ogg"), BASICSOUND_GAME_LAUNCH);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/032.ogg"), BASICSOUND_WORST_PARRY);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/tenshi/052.ogg"), BASICSOUND_INSTALL_START);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/022.ogg"), BASICSOUND_WALL_BOUNCE);
			my_assert_eq(this->soundMgr.load("assets/sfxs/se/022.ogg"), BASICSOUND_GROUND_SLAM);
		} catch (...) {
			game = nullptr;
			throw;
		}
	}

	std::vector<std::string> Game::getCharacters()
	{
		std::vector<std::string> result;

		for (auto &entry : this->fileMgr.listDirectory("assets/characters")) {
			if (!entry.dir)
				continue;

			auto file = entry.path + "/chr.json";

			if (!this->fileMgr.exists(file))
				continue;
			result.push_back(entry.path);
		}
		return result;
	}
}
