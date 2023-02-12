//
// Created by PinkySmile on 13/09/2022.
//

#include "ServerInGame.hpp"
#include "Resources/Game.hpp"
#include "../CharacterSelect.hpp"
#include "Resources/Network/ServerConnection.hpp"

namespace SpiralOfFate
{
	ServerInGame::ServerInGame(
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

	void ServerInGame::_onGameEnd()
	{
		auto conn = reinterpret_cast<ServerConnection *>(&*game->connection);

		conn->switchMenu(MENUSTATE_CHARSELECT, false);
		this->_nextScene = conn->getChrLoadingScreen();
	}

	void ServerInGame::consumeEvent(const sf::Event &event)
	{
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
			game->soundMgr.play(BASICSOUND_MENU_CANCEL);
			return this->_onGameEnd();
		}
		NetworkInGame::consumeEvent(event);
	}
}