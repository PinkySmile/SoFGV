//
// Created by PinkySmile on 26/08/2022.
//

#include "NetworkCharacterSelect.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	NetworkCharacterSelect::NetworkCharacterSelect() :
		CharacterSelect(nullptr, nullptr, nullptr),
		_localRealInput{new NetworkInput(*game->activeNetInput, *game->connection)},
		_remoteRealInput{new RemoteInput(*game->connection)},
		_localInput{new DelayInput(this->_localRealInput)},
		_remoteInput{new DelayInput(this->_remoteRealInput)}
	{
		game->connection->nextGame();
		this->_localInput->setDelay(CHARACTER_SELECT_DELAY);
		this->_remoteInput->setDelay(CHARACTER_SELECT_DELAY);
	}

	NetworkCharacterSelect::NetworkCharacterSelect(
		int leftPos, int rightPos,
		int leftPalette, int rightPalette,
		int stage, int platformCfg
	) :
		CharacterSelect(nullptr, nullptr, leftPos, rightPos, leftPalette, rightPalette, stage, platformCfg, nullptr),
		_localRealInput{new NetworkInput(*game->activeNetInput, *game->connection)},
		_remoteRealInput{new RemoteInput(*game->connection)},
		_localInput{new DelayInput(this->_localRealInput)},
		_remoteInput{new DelayInput(this->_remoteRealInput)}
	{
		this->_localInput->setDelay(CHARACTER_SELECT_DELAY);
		this->_remoteInput->setDelay(CHARACTER_SELECT_DELAY);
	}

	IScene *NetworkCharacterSelect::update()
	{
		// Dirty fix because we don't actually want to update the input itself.
		// The DelayInput refreshes its buffer in hasInputs().
		// If we don't call this, in delay 0, the local input never sends its first input.
		// In turn, the remote does the same, so we are in both cases waiting for the remote
		// to send their input to proceed, which will never happen.
		this->_localInput->fillBuffer();
		this->_remoteInput->fillBuffer();
		this->_remoteRealInput->refreshInputs();
		if (this->_remoteInput->hasInputs())
			return CharacterSelect::update();
		return nullptr;
	}
}