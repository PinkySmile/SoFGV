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
		this->_localInput->fillBuffer();
		this->_remoteInput->fillBuffer();
		this->_remoteRealInput->refreshInputs();
		if (this->_remoteInput->hasInputs())
			return CharacterSelect::update();
		return nullptr;
	}

	void NetworkCharacterSelect::render() const
	{
		CharacterSelect::render();
		game->screen->textSize(20);
		game->screen->displayElement(game->connection->getNames().first, {0, 2}, 540, Screen::ALIGN_CENTER);
		game->screen->displayElement(game->connection->getNames().second, {1140, 2}, 540, Screen::ALIGN_CENTER);
		game->screen->textSize(30);
		game->screen->displayElement("Delay " + std::to_string(game->connection->getCurrentDelay()), {0, 920});
	}
}