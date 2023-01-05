//
// Created by PinkySmile on 26/08/2022.
//

#include "ServerCharacterSelect.hpp"
#include "Resources/Game.hpp"
#include "Resources/Network/ServerConnection.hpp"

namespace SpiralOfFate
{
	ServerCharacterSelect::ServerCharacterSelect()
	{
		this->_leftInput = std::shared_ptr<IInput>(this->_localInput);
		this->_rightInput = std::shared_ptr<IInput>(this->_remoteInput);
	}

	ServerCharacterSelect::ServerCharacterSelect(
		int leftPos, int rightPos,
		int leftPalette, int rightPalette,
		int stage, int platformCfg
	) :
		NetworkCharacterSelect(leftPos, rightPos, leftPalette, rightPalette, stage, platformCfg)
	{
		this->_leftInput = std::shared_ptr<IInput>(this->_localInput);
		this->_rightInput = std::shared_ptr<IInput>(this->_remoteInput);
	}

	void ServerCharacterSelect::consumeEvent(const sf::Event &event)
	{
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::B) {
			if (game->connection->getCurrentDelay() > 0)
				game->connection->updateDelay(game->connection->getCurrentDelay() - 1);
		} else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::N) {
			if (game->connection->getCurrentDelay() < 8)
				game->connection->updateDelay(game->connection->getCurrentDelay() + 1);
		}
		CharacterSelect::consumeEvent(event);
	}

	LoadingScene *ServerCharacterSelect::_launchGame()
	{
		std::uniform_int_distribution<size_t> dist{0, this->_entries.size() - 1};
		std::uniform_int_distribution<size_t> dist2{0, this->_stages.size() - 1};

		if (this->_stage == -1) {
			this->_platform = -1;
			this->_stage = dist2(game->random);
		}

		std::uniform_int_distribution<size_t> dist3{0, this->_stages[this->_stage].platforms.size() - 1};

		if (this->_platform == -1)
			this->_platform = dist3(game->random);
		if (this->_leftPos < 0)
			this->_leftPalette = 0;
		if (this->_rightPos < 0)
			this->_rightPalette = 0;
		if (this->_leftPos < 0)
			this->_leftPos = dist(game->random);
		if (this->_rightPos < 0)
			this->_rightPos = dist(game->random);
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
		game->soundMgr.play(BASICSOUND_GAME_LAUNCH);
		reinterpret_cast<ServerConnection *>(&*game->connection)->startGame(
			game->random(),
			this->_leftPos,
			this->_leftPalette,
			this->_rightPos,
			this->_rightPalette,
			this->_stage,
			this->_platform
		);
		return nullptr;
	}
}