//
// Created by PinkySmile on 26/08/2022.
//

#include "ClientCharacterSelect.hpp"

#include <utility>
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	ClientCharacterSelect::ClientCharacterSelect(std::shared_ptr<IInput> localInput) :
		NetworkCharacterSelect(std::move(localInput))
	{
		this->_leftInput = std::shared_ptr<IInput>(this->_remoteInput);
		this->_rightInput = std::shared_ptr<IInput>(this->_localInput);
	}

	ClientCharacterSelect::ClientCharacterSelect(
		std::shared_ptr<IInput> localInput,
		int leftPos, int rightPos,
		int leftPalette, int rightPalette,
		int stage, int platformCfg
	) :
		NetworkCharacterSelect(std::move(localInput), leftPos, rightPos, leftPalette, rightPalette, stage, platformCfg)
	{
		this->_leftInput = std::shared_ptr<IInput>(this->_remoteInput);
		this->_rightInput = std::shared_ptr<IInput>(this->_localInput);
	}

	InGame *ClientCharacterSelect::_launchGame()
	{
		game->soundMgr.play(BASICSOUND_GAME_LAUNCH);
		return nullptr;
	}
}