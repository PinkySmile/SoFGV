//
// Created by Gegel85 on 07/02/2022.
//

#include "NetplayCharacterSelect.hpp"
#include "../Resources/Game.hpp"
#include "NetplayInGame.hpp"

namespace Battle
{
	NetplayCharacterSelect::NetplayCharacterSelect() :
		CharacterSelect(game.networkMgr._leftInput, game.networkMgr._rightInput)
	{
	}

	IScene *NetplayCharacterSelect::update()
	{
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
		}
		*len = 4 * sizeof(int);
	}

	void NetplayCharacterSelect::_loadState(void *data)
	{
		auto savedData = reinterpret_cast<int *>(data);

		this->_leftPos = savedData[0];
		this->_rightPos = savedData[1];
		this->_leftPalette = savedData[2];
		this->_rightPalette = savedData[3];
	}

	IScene *NetplayCharacterSelect::_realUpdate()
	{
		this->_leftInput->update();
		this->_rightInput->update();

		auto lInputs = this->_leftInput->getInputs();
		auto rInputs = this->_rightInput->getInputs();

		if (lInputs.horizontalAxis == -1) {
			if (this->_leftPos == -1)
				this->_leftPos = static_cast<int>(this->_entries.size());
			this->_leftPos--;
		} else if (lInputs.horizontalAxis == 1) {
			this->_leftPos++;
			if (this->_leftPos == static_cast<int>(this->_entries.size()))
				this->_leftPos = -1;
		}

		if (rInputs.horizontalAxis == -1) {
			if (this->_rightPos == -1)
				this->_rightPos = static_cast<int>(this->_entries.size());
			this->_rightPos--;
		} else if (rInputs.horizontalAxis == 1) {
			this->_rightPos++;
			if (this->_rightPos == static_cast<int>(this->_entries.size()))
				this->_rightPos = -1;
		}

		if (this->_leftPos >= 0) {
			if (lInputs.verticalAxis == -1) {
				do {
					if (this->_leftPalette == 0)
						this->_leftPalette = static_cast<int>(this->_entries[this->_leftPos].palettes.size());
					this->_leftPalette--;
				} while (this->_entries[this->_leftPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette);
			} else if (lInputs.verticalAxis == 1) {
				do {
					this->_leftPalette++;
					if (this->_leftPalette == static_cast<int>(this->_entries[this->_leftPos].palettes.size()))
						this->_leftPalette = 0;
				} while (this->_entries[this->_leftPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette);
			}
		}
		if (this->_rightPos >= 0) {
			if (rInputs.verticalAxis == -1) {
				do {
					if (this->_rightPalette == 0)
						this->_rightPalette = static_cast<int>(this->_entries[this->_rightPos].palettes.size());
					this->_rightPalette--;
				} while (this->_entries[this->_rightPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette);
			} else if (rInputs.verticalAxis == 1) {
				do {
					this->_rightPalette++;
					if (this->_rightPalette == static_cast<int>(this->_entries[this->_rightPos].palettes.size()))
						this->_rightPalette = 0;
				} while (this->_entries[this->_rightPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette);
			}
		}

		if (lInputs.n == 1) {
			std::uniform_int_distribution<size_t> dist{0, this->_entries.size() - 1};

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

			return new NetplayInGame(
				lchr,
				rchr,
				this->_entries[this->_leftPos].entry,
				this->_entries[this->_rightPos].entry
			);
		}
		return nullptr;
	}
}
