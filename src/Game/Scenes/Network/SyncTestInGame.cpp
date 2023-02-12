//
// Created by PinkySmile on 15/08/2022.
//

#include "Resources/Game.hpp"
#include "SyncTestInGame.hpp"
#include "../TitleScreen.hpp"
#include "../CharacterSelect.hpp"

namespace SpiralOfFate
{
	SyncTestInGame::SyncTestInGame(
		const InGame::GameParams &params,
		const std::vector<struct PlatformSkeleton> &platforms,
		const struct StageEntry &stage,
		Character *leftChr, Character *rightChr,
		unsigned int licon, unsigned int ricon,
		const nlohmann::json &lJson, const nlohmann::json &rJson
	) :
		InGame(params, platforms, stage, leftChr, rightChr, licon, ricon, lJson, rJson, true, "char_select"),
		_rollback(leftChr, rightChr)
	{
	}

	void SyncTestInGame::update()
	{
		auto linput = game->battleMgr->getLeftCharacter()->getInput();
		auto rinput = game->battleMgr->getRightCharacter()->getInput();

		if (this->_moveList) {
			linput->update();
			rinput->update();
			this->_moveListUpdate((this->_paused == 1 ? linput : rinput)->getInputs());
		} else if (!this->_paused) {
			auto res = this->_rollback.update(true, true);

			this->_rollback.debugRollback();
			if (res == RollbackMachine::UPDATESTATUS_GAME_ENDED) {
				auto args = new CharacterSelect::Arguments();

				args->leftInput = game->battleMgr->getLeftCharacter()->getInput();
				args->rightInput = game->battleMgr->getRightCharacter()->getInput();
				args->leftPos = game->battleMgr->getLeftCharacter()->index & 0xFFFF;
				args->rightPos = game->battleMgr->getRightCharacter()->index & 0xFFFF;
				args->leftPalette = game->battleMgr->getLeftCharacter()->index >> 16;
				args->rightPalette = game->battleMgr->getRightCharacter()->index >> 16;
				//TODO: Save the stage and platform config properly
				args->stage = 0;
				args->platformCfg = 0;
				args->inGameName = game->scene.getCurrentScene().first;
				game->scene.switchScene(this->_endScene, args);
				return;
			}
			if (linput->getInputs().pause == 1)
				this->_paused = 1;
			else if (rinput->getInputs().pause == 1)
				this->_paused = 2;
		} else
			this->_pauseUpdate();
	}

	void SyncTestInGame::consumeEvent(const sf::Event &event)
	{
		this->_rollback.consumeEvent(event);
	}

	SyncTestInGame *SyncTestInGame::create(SceneArguments *args)
	{
		checked_cast(realArgs, InGame::Arguments, args);

		auto params = realArgs->characterSelectScene->createParams(args);

		if (args->reportProgressA)
			args->reportProgressA("Creating scene...");
		return new SyncTestInGame(
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
	}

}
