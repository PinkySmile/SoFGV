//
// Created by Gegel85 on 07/02/2022.
//

#include "NetplayCharacterSelect.hpp"
#include "../Resources/Game.hpp"
#include "NetplayInGame.hpp"
#include "TitleScreen.hpp"
#include "../Logger.hpp"

namespace Battle
{
	NetplayCharacterSelect::NetplayCharacterSelect() :
		CharacterSelect(game.networkMgr._leftInput, game.networkMgr._rightInput)
	{
		game.logger.info("NetplayCharacterSelect scene created");
	}

	NetplayCharacterSelect::~NetplayCharacterSelect()
	{
		game.logger.info("NetplayCharacterSelect scene destroyed");
	}

	void NetplayCharacterSelect::consumeEvent(const sf::Event &event)
	{
		CharacterSelect::consumeEvent(event);
		game.networkMgr.consumeEvent(event);
	}

	IScene *NetplayCharacterSelect::update()
	{
		if (!game.networkMgr.isConnected())
			return new TitleScreen(game.P1, game.P2);
		game.networkMgr.nextFrame();
		return nullptr;
	}

	void NetplayCharacterSelect::_saveState(void *data, int *len)
	{
		if (data) {
			auto savedData = reinterpret_cast<int *>(data);

			savedData[0] = this->_leftPos;
			savedData[1] = this->_rightPos;
			savedData[2] = this->_leftPalette;
			savedData[3] = this->_rightPalette;
			savedData[4] = this->_selectingStage;
		}
		if (len)
			*len = 5 * sizeof(int);
	}

	void NetplayCharacterSelect::_loadState(void *data)
	{
		auto savedData = reinterpret_cast<int *>(data);

		this->_leftPos = savedData[0];
		this->_rightPos = savedData[1];
		this->_leftPalette = savedData[2];
		this->_rightPalette = savedData[3];
		this->_selectingStage = savedData[4];
	}

	IScene *NetplayCharacterSelect::_realUpdate()
	{
		return CharacterSelect::update();
	}

	InGame *NetplayCharacterSelect::_launchGame()
	{
		std::uniform_int_distribution<size_t> dist{0, this->_entries.size() - 1};
		auto &stage = this->_stages[this->_stage];

		if (this->_leftPos < 0)
			this->_leftPalette = 0;
		if (this->_rightPos < 0)
			this->_rightPalette = 0;
		if (this->_leftPos < 0)
			this->_leftPos = dist(game.random);
		if (this->_rightPos < 0)
			this->_rightPos = dist(game.random);
		if (this->_leftPos == this->_rightPos && this->_entries[this->_leftPos].palettes.size() <= 1) {
			this->_leftPalette = 0;
			this->_rightPalette = 0;
		} else if (this->_leftPos == this->_rightPos && this->_entries[this->_leftPos].palettes.size() == 2 && this->_leftPalette == this->_rightPalette) {
			this->_leftPalette = 0;
			this->_rightPalette = 1;
		}
		if (this->_leftPos == this->_rightPos && this->_leftPalette == this->_rightPalette && this->_entries[this->_leftPos].palettes.size() > 1) {
			this->_rightPalette++;
			this->_rightPalette %= this->_entries[this->_leftPos].palettes.size();
		}

		auto lchr = this->_createCharacter(this->_leftPos,  this->_leftPalette,  this->_leftInput);
		auto rchr = this->_createCharacter(this->_rightPos, this->_rightPalette, this->_rightInput);

		game.soundMgr.play(BASICSOUND_MENU_CONFIRM);
		return new NetplayInGame(
			{static_cast<unsigned>(this->_stage), 0, static_cast<unsigned>(this->_platform)},
			stage.platforms[this->_platform],
			stage,
			lchr,
			rchr,
			this->_entries[this->_leftPos].entry,
			this->_entries[this->_rightPos].entry
		);
	}
}
