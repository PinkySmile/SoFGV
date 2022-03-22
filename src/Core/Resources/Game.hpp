//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_GAME_HPP
#define BATTLE_GAME_HPP


#include <memory>
#include <random>
#include <GGPO/ggpo.h>
#include "Screen.hpp"
#include "TextureManager.hpp"
#include "BattleManager.hpp"
#include "SoundManager.hpp"
#include "../Scenes/IScene.hpp"
#include "../Inputs/KeyboardInput.hpp"
#include "../Inputs/ControllerInput.hpp"
#include "../Inputs/RemoteInput.hpp"
#include "NetManager.hpp"
#include "../Logger.hpp"


#define my_assert(_Expression) \
 (void) \
 ((!!(_Expression)) || \
  (_my_assert(#_Expression, __FILE__,__LINE__)))
#define my_assert_eq(_Expression, _Expression2)                                                \
	do {                                                                                   \
 		auto a = (_Expression);                                                        \
		auto b = (_Expression2);                                                       \
		                                                                               \
		if (a != b)                                                                    \
			_my_assert_eq(#_Expression, #_Expression2, a, b, __FILE__,__LINE__);   \
	} while (0)

namespace Battle
{
	enum BasicSounds {
		/*  1 */ BASICSOUND_MENU_MOVE = 1,
		/*  2 */ BASICSOUND_MENU_CANCEL,
		/*  3 */ BASICSOUND_MENU_CONFIRM,
		/*  4 */ BASICSOUND_KNOCKDOWN,
		/*  5 */ BASICSOUND_KNOCK_OUT,
		/*  6 */ BASICSOUND_GUARD_BREAK,
		/*  7 */ BASICSOUND_GUARD_RECOVER,
		/*  8 */ BASICSOUND_OVERDRIVE,
		/*  9 */ BASICSOUND_OVERDRIVE_RECOVER,
		/* 10 */ BASICSOUND_HIGH_JUMP,
		/* 11 */ BASICSOUND_LAND,
		/* 12 */ BASICSOUND_COUNTER_HIT,
		/* 13 */ BASICSOUND_DASH,
		/* 14 */ BASICSOUND_BLOCK,
		/* 15 */ BASICSOUND_WRONG_BLOCK,
		/* 16 */ BASICSOUND_SPELLFLASH,
		/* 17 */ BASICSOUND_ROMAN_CANCEL,
		/* 18 */ BASICSOUND_ULTIMATE,
		/* 19 */ BASICSOUND_PARRY,
		/* 20 */ BASICSOUND_BEST_PARRY,
		/* 21 */ BASICSOUND_GAME_LAUNCH,
		/*  4 */ BASICSOUND_WALL_BOUNCE = BASICSOUND_KNOCKDOWN,
		/*  4 */ BASICSOUND_GROUND_SLAM = BASICSOUND_KNOCKDOWN,
	};

	struct Game {
		bool hosting = false;
		std::mt19937 random;
		std::mt19937 battleRandom;
		Logger logger{"./latest.log"};
		sf::Font font;
		std::string lastIp = "127.0.0.1";
		std::unique_ptr<Screen> screen;
		TextureManager textureMgr;
		SoundManager soundMgr;
		NetManager networkMgr;
		std::unique_ptr<BattleManager> battleMgr;
		std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> P1;
		std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> P2;
		std::shared_ptr<IScene> scene;

		Game();
	};

	namespace GGPONetplay {
		bool __cdecl startGame(const char *game);
		bool __cdecl saveState(unsigned char **buffer, int *len, int *checksum, int frame);
		bool __cdecl loadState(unsigned char *buffer, int len);
		bool __cdecl logState(char *filename, unsigned char *buffer, int len);
		void __cdecl freeBuffer(void *buffer);
		bool __cdecl updateGame(int flags);
		bool __cdecl onEvent(GGPOEvent *info);
	}
	extern Game *game;
}

class AssertionFailedException : public std::exception {
private:
	std::string _msg;

public:
	AssertionFailedException(const std::string &msg) : _msg(std::move(msg)) {}
	const char *what() const noexcept override { return this->_msg.c_str(); }
};

template<typename T, typename T2>
int __attribute__((noreturn)) _my_assert_eq(const char *expr, const char *expr2, T exprR, T2 expr2R, const char *file, int line)
{
	auto err = "Debug Assertion " + std::string(expr) + " == " + expr2 + " failed (" + std::to_string(exprR) + " != " + std::to_string(expr2R) + ") in " + file + " at line " + std::to_string(line);

	Battle::game->logger.fatal(err);
	throw AssertionFailedException(err);
}
int __attribute__((noreturn)) _my_assert(const char *expr, const char *file, int line);

#endif //BATTLE_GAME_HPP
