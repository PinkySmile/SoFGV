//
// Created by Gegel85 on 28/09/2021.
//

#include "../Objects/ACharacter.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"
#include "NetplayInGame.hpp"
#include "NetplayCharacterSelect.hpp"

namespace Battle
{
	NetplayInGame::NetplayInGame(ACharacter *leftChr, ACharacter *rightChr, const nlohmann::json &lJson, const nlohmann::json &rJson) :
		InGame(leftChr, rightChr, lJson, rJson)
	{
		sf::View view{{-50, -600, 1100, 700}};

		logger.info("NetplayGame scene created");
		Battle::game.screen->setView(view);
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
			game.networkMgr.endSession();
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
		*len = size;
	}

	void NetplayInGame::_loadState(void *data)
	{
		game.battleMgr->restoreFromBuffer(data);
	}

	IScene *NetplayInGame::_realUpdate()
	{
		if (this->_nextScene)
			return this->_nextScene;

		if (!Battle::game.battleMgr->update())
			this->_nextScene = new NetplayCharacterSelect();
		return this->_nextScene;
	}
}