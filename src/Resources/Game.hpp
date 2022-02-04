//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_GAME_HPP
#define BATTLE_GAME_HPP


#include <memory>
#include <random>
#include "Screen.hpp"
#include "TextureManager.hpp"
#include "BattleManager.hpp"
#include "../Scenes/IScene.hpp"
#include "../Inputs/KeyboardInput.hpp"
#include "../Inputs/ControllerInput.hpp"
#include "SoundManager.hpp"

namespace Battle
{
	struct Game {
		sf::Font font;
		std::unique_ptr<Screen> screen;
		TextureManager textureMgr;
		SoundManager soundMgr;
		std::unique_ptr<BattleManager> battleMgr;
		std::unique_ptr<IScene> scene;
		std::mt19937 random;
		std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> P1;
		std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> P2;
	};

	extern Game game;
}


#endif //BATTLE_GAME_HPP
