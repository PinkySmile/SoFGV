//
// Created by Gegel85 on 24/09/2021.
//

#include "InGame.hpp"
#include "../Objects/ACharacter.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"

namespace Battle
{
	InGame::InGame(ACharacter *leftChr, ACharacter *rightChr, const nlohmann::json &lJson, const nlohmann::json &rJson)
	{
		sf::View view{{-50, -500, 1100, 618.75}};

		logger.info("InGame scene created");
		Battle::game.screen->setView(view);
		game.battleMgr = std::make_unique<BattleManager>(
			BattleManager::CharacterParams{
				leftChr,
				lJson["hp"],
				{lJson["gravity"]["x"], lJson["gravity"]["y"]},
				lJson["jump_count"]
			},
			BattleManager::CharacterParams{
				rightChr,
				rJson["hp"],
				{rJson["gravity"]["x"], rJson["gravity"]["y"]},
				rJson["jump_count"]
			}
		);
	}

	void InGame::render() const
	{
		Battle::game.battleMgr->render();
	}

	IScene *InGame::update()
	{
		Battle::game.battleMgr->update();
		return nullptr;
	}

	void InGame::consumeEvent(const sf::Event &event)
	{
		game.battleMgr->consumeEvent(event);
	}
}