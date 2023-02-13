//
// Created by PinkySmile on 26/08/2022.
//

#include "ClientCharacterSelect.hpp"

#include <utility>
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	ClientCharacterSelect::ClientCharacterSelect()
	{
		this->_leftInput = std::shared_ptr<IInput>(this->_remoteInput);
		this->_rightInput = std::shared_ptr<IInput>(this->_localInput);
	}

	ClientCharacterSelect::ClientCharacterSelect(
		int leftPos, int rightPos,
		int leftPalette, int rightPalette,
		int stage, int platformCfg
	) :
		NetworkCharacterSelect(leftPos, rightPos, leftPalette, rightPalette, stage, platformCfg)
	{
		this->_leftInput = std::shared_ptr<IInput>(this->_remoteInput);
		this->_rightInput = std::shared_ptr<IInput>(this->_localInput);
	}

	void ClientCharacterSelect::_launchGame()
	{
		game->soundMgr.play(BASICSOUND_GAME_LAUNCH);
	}

	ClientCharacterSelect *ClientCharacterSelect::create(SceneArguments *args)
	{
		checked_cast(realArgs, ClientConnection::CharSelectArguments, args);
		if (args->reportProgressA)
			args->reportProgressA("Loading assets...");

		auto result = realArgs->restore ?
			new ClientCharacterSelect(
				realArgs->startParams.p1chr,
				realArgs->startParams.p2chr,
				realArgs->startParams.p1pal,
				realArgs->startParams.p2pal,
				realArgs->startParams.stage,
				realArgs->startParams.platformConfig
			) : new ClientCharacterSelect();

		realArgs->connection->notifySwitchMenu();
		if (args->reportProgressA)
			args->reportProgressA("Waiting for opponent to finish loading...");
		realArgs->connection->waitForOpponent();
		return result;
	}
}