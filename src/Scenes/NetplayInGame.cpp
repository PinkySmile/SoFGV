//
// Created by Gegel85 on 28/09/2021.
//

#include "../Objects/Character.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"
#include "NetplayInGame.hpp"
#include "NetplayCharacterSelect.hpp"
#include "TitleScreen.hpp"

namespace Battle
{
	NetplayInGame::NetplayInGame(const GameParams &params, const std::vector<struct PlatformSkeleton> &platforms, const struct StageEntry &stage, Character *leftChr, Character *rightChr, const nlohmann::json &lJson, const nlohmann::json &rJson) :
		InGame(params, platforms, stage, leftChr, rightChr, lJson, rJson)
	{
		sf::View view{{-50, -600, 1100, 700}};

		game.logger.info("NetplayGame scene created");
		Battle::game.screen->setView(view);
	}

	NetplayInGame::~NetplayInGame()
	{
		game.logger.debug("NetplayInGame scene destroyed");
	}

	void NetplayInGame::consumeEvent(const sf::Event &event)
	{
		InGame::consumeEvent(event);
		game.networkMgr.consumeEvent(event);
	}

	IScene *NetplayInGame::update()
	{
		auto linput = game.battleMgr->getLeftCharacter()->getInput();
		auto rinput = game.battleMgr->getRightCharacter()->getInput();

		if( !game.networkMgr.isConnected())
			return new TitleScreen(game.P1, game.P2);
		if (this->_moveList)
			this->_moveListUpdate();
		else if (!this->_paused) {
			if (linput->getInputs().pause == 1)
				this->_paused = 1;
			else if (rinput->getInputs().pause == 1)
				this->_paused = 2;
		} else
			this->_pauseUpdate();
		if (!this->_nextScene)
			game.networkMgr.nextFrame();
		else
			return game.networkMgr.endSession(), new TitleScreen(game.P1, game.P2);
		return this->_nextScene;
	}

	void NetplayInGame::render() const
	{
		InGame::render();
		game.networkMgr.postRender();
	}

	void NetplayInGame::_saveState(void *data, int *len)
	{
		auto size = game.battleMgr->getBufferSize();

		if (data)
			game.battleMgr->copyToBuffer(data);
		if (len)
			*len = size;
	}

	void NetplayInGame::_loadState(void *data)
	{
		game.battleMgr->restoreFromBuffer(data);
	}

	IScene *NetplayInGame::_realUpdate()
	{
		if (!Battle::game.battleMgr->update())
			return new NetplayCharacterSelect();
		return nullptr;
	}
}