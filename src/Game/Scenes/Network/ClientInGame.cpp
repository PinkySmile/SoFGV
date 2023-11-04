//
// Created by PinkySmile on 13/09/2022.
//

#include "ClientInGame.hpp"
#include "Resources/Game.hpp"
#include "../CharacterSelect.hpp"
#include "ClientCharacterSelect.hpp"

namespace SpiralOfFate
{
	ClientInGame::ClientInGame(
		std::shared_ptr<RemoteInput> input,
		const InGame::GameParams &params,
		const std::vector<struct PlatformSkeleton> &platforms,
		const struct StageEntry &stage,
		Character *leftChr,
		Character *rightChr,
		unsigned int licon,
		unsigned int ricon,
		const nlohmann::json &lJson,
		const nlohmann::json &rJson
	) :
		NetworkInGame(std::move(input), params, platforms, stage, leftChr, rightChr, licon, ricon, lJson, rJson)
	{
	}

	void ClientInGame::_onGameEnd() {}

	void ClientInGame::consumeEvent(const sf::Event &event)
	{
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
			game->soundMgr.play(BASICSOUND_MENU_CANCEL);
			game->connection->quitGame();
			return;
		}
		NetworkInGame::consumeEvent(event);
	}

	ClientInGame *ClientInGame::create(SceneArguments *args)
	{
		checked_cast(realArgs, InGameArguments, args);
		checked_cast(scene, NetworkCharacterSelect, realArgs->currentScene);

		auto params = scene->createParams(args, realArgs->startParams);

		game->battleRandom.seed(realArgs->startParams.seed);
		scene->flushInputs(realArgs->connection->getCurrentDelay());
		if (args->reportProgressA)
			args->reportProgressA("Creating scene...");

		auto result = new ClientInGame(
			scene->getRemoteRealInput(),
			params.params,
			params.platforms,
			params.stage,
			params.leftChr,
			params.rightChr,
			params.licon,
			params.ricon,
			params.lJson,
			params.rJson
		);

		realArgs->connection->notifySwitchMenu();
		if (args->reportProgressA)
			args->reportProgressA("Waiting for opponent to finish loading...");
		realArgs->connection->waitForOpponent();
		return result;
	}
}