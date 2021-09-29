//
// Created by Gegel85 on 28/09/2021.
//

#include "NetplayGame.hpp"
#include "../Objects/ACharacter.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"

namespace Battle
{
	NetplayGame::NetplayGame(RemoteInput *remote, NetworkInput *input, IInput *leftInput, IInput *rightInput) :
		_remote(remote),
		_input(input)
	{
		sf::View view{{-50, -500, 1100, 618.75}};

		logger.info("NetplayGame scene created");
		Battle::game.screen->setView(view);
		game.battleMgr = std::make_unique<BattleManager>(
			new ACharacter{
				"assets/characters/test/framedata.json",
				leftInput
			},
			new ACharacter{
				"assets/characters/test/framedata.json",
				rightInput
			}
		);
		this->_hosts = leftInput == input;
	}

	void NetplayGame::render() const
	{
		Battle::game.battleMgr->render();
	}

	IScene *NetplayGame::update()
	{
		//if (!this->_hosts)
		//	this->_remote->receiveData();
		//this->_input->sendInputs();
		//if (this->_hosts)
		//	this->_remote->receiveData();
		Battle::game.battleMgr->update();
		return nullptr;
	}

	void NetplayGame::consumeEvent(const sf::Event &event)
	{
		game.battleMgr->consumeEvent(event);
	}
}