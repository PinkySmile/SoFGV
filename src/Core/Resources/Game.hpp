//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_GAME_HPP
#define BATTLE_GAME_HPP


#include <memory>
#include <random>
#ifndef __ANDROID__
#include <GGPO/ggpo.h>
#endif
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

#ifdef _MSC_VER
#ifdef MYDLL_EXPORTS
#define MYDLL_API __declspec(dllexport)
#else
#define MYDLL_API __declspec(dllimport)
#endif
#else
#define MYDLL_API
#endif

#ifdef __GNUC__
#define FCT_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define FCT_NAME __FUNCSIG__
#else
#define FCT_NAME __func__
#endif

#define my_assert(_Expression)                                                                 \
	do {                                                                                   \
		if (_Expression);                                                              \
		else                                                                           \
			throw _AssertionFailedException(                                       \
				"Debug Assertion " + std::string(#_Expression) +               \
				" failed in " + __FILE__ +                                     \
				" at line " + std::to_string(__LINE__) +                       \
				" in " + FCT_NAME                                              \
			);                                                                     \
	} while (0)
#define my_assert2(_Expression, msg)                                                           \
	do {                                                                                   \
		if (_Expression);                                                              \
		else                                                                           \
			throw AssertionFailedException(#_Expression, msg);                     \
	} while (0)

#define my_assert_eq(_Expression, _Expression2)                                                \
	do {                                                                                   \
 		auto a = (_Expression);                                                        \
		auto b = (_Expression2);                                                       \
		                                                                               \
		if (a != b)                                                                    \
			throw AssertionFailedException(                                        \
				std::string(#_Expression) + " == " + #_Expression2,            \
				std::to_string(a) + " != " + std::to_string(b)                 \
			);                                                                     \
	} while (0)

#define AssertionFailedException(expr, msg) _AssertionFailedException( \
	"Debug Assertion " + std::string(expr) +                       \
	" failed in " + __FILE__ +                                     \
	" at line " + std::to_string(__LINE__) +                       \
	" in " + FCT_NAME + ": " + msg                                 \
)

namespace SpiralOfFate
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
		/* 22 */ BASICSOUND_WORST_PARRY,
		/*  4 */ BASICSOUND_WALL_BOUNCE = BASICSOUND_KNOCKDOWN,
		/*  4 */ BASICSOUND_GROUND_SLAM = BASICSOUND_KNOCKDOWN,
	};

	struct Game {
		bool hosting = false;
		std::mt19937 random;
		std::mt19937 battleRandom;
		Logger logger;
		sf::Font font;
		std::string lastIp = "127.0.0.1";
		std::unique_ptr<Screen> screen;
		TextureManager textureMgr;
		SoundManager soundMgr;
		std::unique_ptr<BattleManager> battleMgr;
		NetManager networkMgr;
		std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> P1;
		std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> P2;
		std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> menu;
		std::shared_ptr<IScene> scene;

		Game(const std::string &loggerPath = "./latest.log");
		~Game();
	};

#ifndef __ANDROID__
	namespace GGPONetplay {
		bool __cdecl startGame(const char *game);
		bool __cdecl saveState(unsigned char **buffer, int *len, int *checksum, int frame);
		bool __cdecl loadState(unsigned char *buffer, int len);
		bool __cdecl logState(char *filename, unsigned char *buffer, int len);
		void __cdecl freeBuffer(void *buffer);
		bool __cdecl updateGame(int flags);
		bool __cdecl onEvent(GGPOEvent *info);
	}
#endif
	extern MYDLL_API Game *game;
}

class _AssertionFailedException : public std::exception {
private:
	std::string _msg;

public:
	_AssertionFailedException(const std::string &&msg) : _msg(msg) { SpiralOfFate::game->logger.fatal( "AssertionFailedException: " + msg); }
	const char *what() const noexcept override { return this->_msg.c_str(); }
};

#endif //BATTLE_GAME_HPP
