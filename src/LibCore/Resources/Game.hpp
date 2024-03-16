//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_GAME_HPP
#define SOFGV_GAME_HPP


#include <mutex>
#include <memory>
#include <random>
#include "Screen.hpp"
#include "TextureManager.hpp"
#include "Resources/Battle/BattleManager.hpp"
#include "SoundManager.hpp"
#include "SceneManager.hpp"
#include "KeyboardInput.hpp"
#include "ControllerInput.hpp"
#include "Inputs/RollbackInput.hpp"
#include "Logger.hpp"
#include "Resources/Battle/RandomWrapper.hpp"
#include "MSVCMacros.hpp"
#include "Resources/Assets/FileManager.hpp"
#include "ObjectFactory.hpp"
#ifdef VIRTUAL_CONTROLLER
#include "VirtualController.hpp"
#endif
#ifdef HAS_NETWORK
#include "Resources/Network/Connection.hpp"
#endif

#define random_distrib(r, mi, ma) ((((uint64_t)(r)() - (r).min()) * ((ma) - (mi))) / ((r).max() - (r).min()) + (mi))

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
		/* 23 */ BASICSOUND_INSTALL_START,
		/*  4 */ BASICSOUND_WALL_BOUNCE = BASICSOUND_KNOCKDOWN,
		/*  4 */ BASICSOUND_GROUND_SLAM = BASICSOUND_KNOCKDOWN,
	};

	enum TypeColor {
		TYPECOLOR_NON_TYPED,
		TYPECOLOR_NEUTRAL,
		TYPECOLOR_SPIRIT,
		TYPECOLOR_MATTER,
		TYPECOLOR_VOID,
	};

	struct Game {
		bool hosting = false;
		std::random_device random;
		RandomWrapper battleRandom;
		Logger logger;
		sf::Font font;
		std::string lastIp = "127.0.0.1";
		unsigned short lastPort = 0;
		std::unique_ptr<Screen> screen;
		FileManager fileMgr;
		ObjectFactory objFactory;
		TextureManager textureMgr;
		SoundManager soundMgr;
	#ifdef VIRTUAL_CONTROLLER
		std::shared_ptr<VirtualController> virtualController;
	#endif
		std::unique_ptr<BattleManager> battleMgr;
		std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> P1;
		std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> P2;
		std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> menu;
		std::array<sf::Color, 5> typeColors = {
			sf::Color{0xA6, 0xA6, 0xA6},
			sf::Color{0xFF, 0xFF, 0x00},
			sf::Color{0x33, 0xCC, 0xCC},
			sf::Color{0xBB, 0x5E, 0x00},
			sf::Color{0x67, 0x03, 0x3D}
		};
	#ifdef HAS_NETWORK
		std::shared_ptr<class Connection> connection;
	#endif
		SceneManager scene;
		std::mutex sceneMutex;
		std::shared_ptr<IInput> activeNetInput;

		Game(const std::string &loggerPath = "./latest.log");
		std::vector<std::string> getCharacters();
	};
	extern MYDLL_API Game *game;
}


#endif //SOFGV_GAME_HPP
