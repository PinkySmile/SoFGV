//
// Created by PinkySmile on 26/08/2022.
//

#include "NetworkCharacterSelect.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	NetworkCharacterSelect::NetworkCharacterSelect(std::shared_ptr<IInput> localInput) :
		CharacterSelect(nullptr, nullptr, nullptr),
		_localRealInput{new NetworkInput(*localInput, *game->connection)},
		_remoteRealInput{new RemoteInput(*game->connection)},
		_localInput{new DelayInput(*this->_localRealInput)},
		_remoteInput{new DelayInput(*this->_remoteRealInput)}
	{
		this->_localInput->setDelay(CHARACTER_SELECT_DELAY);
		this->_remoteInput->setDelay(CHARACTER_SELECT_DELAY);
	}

	NetworkCharacterSelect::NetworkCharacterSelect(
		std::shared_ptr<IInput> localInput,
		int leftPos, int rightPos,
		int leftPalette, int rightPalette,
		int stage, int platformCfg
	) :
		CharacterSelect(nullptr, nullptr, leftPos, rightPos, leftPalette, rightPalette, stage, platformCfg, nullptr),
		_localRealInput{new NetworkInput(*localInput, *game->connection)},
		_remoteRealInput{new RemoteInput(*game->connection)},
		_localInput{new DelayInput(*this->_localRealInput)},
		_remoteInput{new DelayInput(*this->_remoteRealInput)}
	{
		this->_localInput->setDelay(CHARACTER_SELECT_DELAY);
		this->_remoteInput->setDelay(CHARACTER_SELECT_DELAY);
	}

	IScene *NetworkCharacterSelect::update()
	{
		this->_remoteRealInput->refreshInputs();
		if (this->_remoteInput->hasInputs())
			return CharacterSelect::update();
		return nullptr;
	}
}