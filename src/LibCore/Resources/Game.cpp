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
		assert_exp(!game);
		game = this;
		try {
			assert_eq(this->soundMgr.load("assets/sfxs/se/039.ogg"), BASICSOUND_MENU_MOVE);
			assert_eq(this->soundMgr.load("assets/sfxs/se/041.ogg"), BASICSOUND_MENU_CANCEL);
			assert_eq(this->soundMgr.load("assets/sfxs/se/040.ogg"), BASICSOUND_MENU_CONFIRM);
			assert_eq(this->soundMgr.load("assets/sfxs/se/022.ogg"), BASICSOUND_KNOCKDOWN);
			assert_eq(this->soundMgr.load("assets/sfxs/se/044.ogg"), BASICSOUND_KNOCK_OUT);
			assert_eq(this->soundMgr.load("assets/sfxs/se/038.ogg"), BASICSOUND_GUARD_BREAK);
			assert_eq(this->soundMgr.load("assets/sfxs/se/047.ogg"), BASICSOUND_GUARD_RECOVER);
			assert_eq(this->soundMgr.load("assets/sfxs/se/055.ogg"), BASICSOUND_OVERDRIVE);
			assert_eq(this->soundMgr.load("assets/sfxs/se/036.ogg"), BASICSOUND_OVERDRIVE_RECOVER);
			assert_eq(this->soundMgr.load("assets/sfxs/se/043.ogg"), BASICSOUND_HIGH_JUMP);
			assert_eq(this->soundMgr.load("assets/sfxs/se/030.ogg"), BASICSOUND_LAND);
			assert_eq(this->soundMgr.load("assets/sfxs/se/025.ogg"), BASICSOUND_COUNTER_HIT);
			assert_eq(this->soundMgr.load("assets/sfxs/se/031.ogg"), BASICSOUND_DASH);
			assert_eq(this->soundMgr.load("assets/sfxs/se/020.ogg"), BASICSOUND_BLOCK);
			assert_eq(this->soundMgr.load("assets/sfxs/se/021.ogg"), BASICSOUND_WRONG_BLOCK);
			assert_eq(this->soundMgr.load("assets/sfxs/se/053.ogg"), BASICSOUND_SPELLFLASH);
			assert_eq(this->soundMgr.load("assets/sfxs/se/035.ogg"), BASICSOUND_ROMAN_CANCEL);
			assert_eq(this->soundMgr.load("assets/sfxs/se/033.ogg"), BASICSOUND_ULTIMATE);
			assert_eq(this->soundMgr.load("assets/sfxs/se/054.ogg"), BASICSOUND_PARRY);
			assert_eq(this->soundMgr.load("assets/sfxs/se/072.ogg"), BASICSOUND_BEST_PARRY);
			assert_eq(this->soundMgr.load("assets/sfxs/se/061.ogg"), BASICSOUND_GAME_LAUNCH);
			assert_eq(this->soundMgr.load("assets/sfxs/se/032.ogg"), BASICSOUND_WORST_PARRY);
			assert_eq(this->soundMgr.load("assets/sfxs/se/tenshi/052.ogg"), BASICSOUND_INSTALL_START);
			assert_eq(this->soundMgr.load("assets/sfxs/se/022.ogg"), BASICSOUND_WALL_BOUNCE);
			assert_eq(this->soundMgr.load("assets/sfxs/se/022.ogg"), BASICSOUND_GROUND_SLAM);
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

	Color Game::getColor(const std::string &name)
	{
		if (name == "non-typed" || name == "non_typed" || name == "non typed" || name == "nt")
			return this->typeColors[TYPECOLOR_NON_TYPED];
		if (name == "neutral" || name == "n")
			return this->typeColors[TYPECOLOR_NEUTRAL];
		if (name == "spirit" || name == "s")
			return this->typeColors[TYPECOLOR_SPIRIT];
		if (name == "matter" || name == "m")
			return this->typeColors[TYPECOLOR_MATTER];
		if (name == "void" || name == "v")
			return this->typeColors[TYPECOLOR_VOID];
		assert_not_reached_msg("\"" + name + "\": Invalid color name");
	}
}
