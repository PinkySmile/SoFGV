//
// Created by PinkySmile on 15/08/2022.
//

#include "Resources/Game.hpp"
#include "SyncTestInGame.hpp"
#include "../TitleScreen.hpp"
#include "../CharacterSelect.hpp"

namespace SpiralOfFate
{
	InGame *createSyncTestInGameSceneIScene(const InGame::GameParams &params, const std::vector<struct PlatformSkeleton> &platforms, const struct StageEntry &stage, Character *leftChr, Character *rightChr, unsigned licon, unsigned ricon, const nlohmann::json &lJson, const nlohmann::json &rJson)
	{
		return new SyncTestInGame(params, platforms, stage, leftChr, rightChr, licon, ricon, lJson, rJson);
	}

	SyncTestInGame::SyncTestInGame(
		const InGame::GameParams &params,
		const std::vector<struct PlatformSkeleton> &platforms,
		const struct StageEntry &stage,
		Character *leftChr, Character *rightChr,
		unsigned int licon, unsigned int ricon,
		const nlohmann::json &lJson, const nlohmann::json &rJson
	) :
		InGame(params, platforms, stage, leftChr, rightChr, licon, ricon, lJson, rJson),
		_rollback(leftChr, rightChr)
	{
	}

	IScene *SyncTestInGame::update()
	{
		if (this->_nextScene)
			return this->_nextScene;

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
				if (this->_goBackToTitle)
					this->_nextScene = new TitleScreen(game->P1, game->P2);
				else
					this->_nextScene = new CharacterSelect(
						game->battleMgr->getLeftCharacter()->getInput(),
						game->battleMgr->getRightCharacter()->getInput(),
						game->battleMgr->getLeftCharacter()->index & 0xFFFF,
						game->battleMgr->getRightCharacter()->index & 0xFFFF,
						game->battleMgr->getLeftCharacter()->index >> 16,
						game->battleMgr->getRightCharacter()->index >> 16,
						//TODO: Save the stage and platform config properly
						0, 0,
						createSyncTestInGameSceneIScene
					);
				return this->_nextScene;
			}
			if (linput->getInputs().pause == 1)
				this->_paused = 1;
			else if (rinput->getInputs().pause == 1)
				this->_paused = 2;
		} else
			this->_pauseUpdate();
		return this->_nextScene;
	}

	void SyncTestInGame::consumeEvent(const sf::Event &event)
	{
		this->_rollback.consumeEvent(event);
	}

}
