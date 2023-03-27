//
// Created by PinkySmile on 26/08/2022.
//

#include "NetworkCharacterSelect.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	NetworkCharacterSelect::NetworkCharacterSelect() :
		CharacterSelect(nullptr, nullptr, ""),
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
		CharacterSelect(nullptr, nullptr, leftPos, rightPos, leftPalette, rightPalette, stage, platformCfg, ""),
		_localRealInput{new NetworkInput(*game->activeNetInput, *game->connection)},
		_remoteRealInput{new RemoteInput(*game->connection)},
		_localInput{new DelayInput(this->_localRealInput)},
		_remoteInput{new DelayInput(this->_remoteRealInput)}
	{
		this->_localInput->setDelay(CHARACTER_SELECT_DELAY);
		this->_remoteInput->setDelay(CHARACTER_SELECT_DELAY);
	}

	void NetworkCharacterSelect::update()
	{
		this->_localInput->fillBuffer();
		this->_remoteInput->fillBuffer();
		this->_remoteRealInput->refreshInputs();
		if (this->_remoteInput->hasInputs()) {
			this->_leftInput->update();
			this->_rightInput->update();

			if (this->_quit) {
				game->connection->quitGame();
				game->connection->terminate();
				return game->scene.switchScene("title_screen");
			}
			if (this->_selectingStage)
				this->_selectStageUpdate();
			else
				this->_selectCharacterUpdate();
		}
	}

	void NetworkCharacterSelect::render() const
	{
		CharacterSelect::render();
		game->screen->textSize(20);
		if (!game->connection)
			return;
		game->screen->displayElement(game->connection->getNames().first, {0, 2}, 540, Screen::ALIGN_CENTER);
		game->screen->displayElement(game->connection->getNames().second, {1140, 2}, 540, Screen::ALIGN_CENTER);
		game->screen->textSize(30);
		game->screen->displayElement("Delay " + std::to_string(game->connection->getCurrentDelay()), {0, 920});
	}

	InGame::InitParams NetworkCharacterSelect::createParams(SceneArguments *args, Connection::GameStartParams params)
	{
		auto &lentry = this->_entries[params.p1chr];
		auto &rentry = this->_entries[params.p2chr];
		auto &licon = lentry.icon[params.p1pal];
		auto &ricon = rentry.icon[params.p2pal];

		if (args->reportProgressW)
			args->reportProgressW(L"Loading P1's character (" + this->_entries[params.p1chr].name + L")");

		auto lchr = this->_createCharacter(params.p1chr, params.p1pal, this->_leftInput);

		if (args->reportProgressW)
			args->reportProgressW(L"Loading P2's character (" + this->_entries[params.p2chr].name + L")");

		auto rchr = this->_createCharacter(params.p2chr, params.p2pal, this->_rightInput);
		auto &stage = this->_stages[params.stage];

		return {
			{static_cast<unsigned>(params.stage), 0, static_cast<unsigned>(params.platformConfig)},
			stage.platforms[params.platformConfig],
			stage,
			lchr,
			rchr,
			licon.textureHandle,
			ricon.textureHandle,
			lentry.entry,
			rentry.entry
		};
	}

	std::shared_ptr<RemoteInput> NetworkCharacterSelect::getRemoteRealInput()
	{
		return this->_remoteRealInput;
	}

	void NetworkCharacterSelect::flushInputs(unsigned int delay)
	{
		this->_localInput->flush(delay);
		this->_remoteInput->flush(delay);
	}
}