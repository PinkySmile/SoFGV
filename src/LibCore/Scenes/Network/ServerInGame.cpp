//
// Created by PinkySmile on 13/09/2022.
//

#include "ServerInGame.hpp"

namespace SpiralOfFate
{
	ServerInGame::ServerInGame(
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
		InGame(params, platforms, stage, leftChr, rightChr, licon, ricon, lJson, rJson),
		_rMachine(leftChr, rightChr)
	{
	}

	IScene *ServerInGame::update()
	{
		if (this->_nextScene)
			return this->_nextScene;

		auto status = this->_rMachine.update(true, true);

		if (status == RollbackMachine::UPDATESTATUS_NO_INPUTS)
			return nullptr;
		return InGame::update();
	}

	void ServerInGame::consumeEvent(const sf::Event &event)
	{
		this->_rMachine.consumeEvent(event);
		InGame::consumeEvent(event);
	}
}