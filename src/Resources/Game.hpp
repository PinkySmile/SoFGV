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

namespace Battle
{
	struct Game {
		bool hosting;
		std::mt19937 random;
		sf::Font font;
		std::unique_ptr<Screen> screen;
		TextureManager textureMgr;
		SoundManager soundMgr;
		NetManager networkMgr;
		std::unique_ptr<BattleManager> battleMgr;
		std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> P1;
		std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> P2;
		std::shared_ptr<IScene> scene;
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
	extern Game game;
}


#endif //BATTLE_GAME_HPP
