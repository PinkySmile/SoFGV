//
// Created by PinkySmile on 13/09/2022.
//

#include "ServerInGame.hpp"
#include "Resources/Game.hpp"
#include "Scenes/CharacterSelect.hpp"
#include "ServerCharacterSelect.hpp"
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
		reinterpret_cast<ServerConnection *>(&*game->connection)->switchMenu(2, false);
		this->_nextScene = new ServerCharacterSelect(
			game->battleMgr->getLeftCharacter()->index & 0xFFFF,
			game->battleMgr->getRightCharacter()->index & 0xFFFF,
			game->battleMgr->getLeftCharacter()->index >> 16,
			game->battleMgr->getRightCharacter()->index >> 16,
			//TODO: Save the stage and platform config properly
			0, 0
		);
	}
}