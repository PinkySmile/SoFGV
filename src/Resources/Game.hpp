//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_GAME_HPP
#define BATTLE_GAME_HPP


#include <memory>
#include "Screen.hpp"
#include "TextureManager.hpp"
#include "BattleManager.hpp"
#include "../Scenes/IScene.hpp"

namespace Battle
{
	struct Game {
		std::unique_ptr<Screen> screen;
		TextureManager textureMgr;
		std::unique_ptr<BattleManager> battleMgr;
		std::unique_ptr<IScene> scene;
	};

	extern Game game;
}


#endif //BATTLE_GAME_HPP
