//
// Created by PinkySmile on 03/12/22
//

#include "SpectatorCharacterSelect.hpp"
#include "Resources/Game.hpp"
#include "Resources/Network/SpectatorConnection.hpp"

namespace SpiralOfFate
{
	SpectatorCharacterSelect::SpectatorCharacterSelect(SpectatorConnection &connection) :
		connection(connection)
	{
		connection.onReplayData = nullptr;
	}

	SpectatorCharacterSelect::~SpectatorCharacterSelect()
	{
		game->logger.debug("~SpectatorCharacterSelect()");
	}

	void SpiralOfFate::SpectatorCharacterSelect::render() const
	{
		ViewPort view{{0, 0, 1680, 960}};

		game->screen->setView(view);
		game->screen->clear({0xA0, 0xA0, 0xA0});
		game->screen->borderColor(2, sf::Color::Black);
		game->screen->fillColor();
		game->screen->textSize(50);
		game->screen->displayElement("Waiting for players...", {0, 440}, 1680, Screen::ALIGN_CENTER);
		game->screen->displayElement("Character selection is in progress", {0, 490}, 1680, Screen::ALIGN_CENTER);
		game->screen->borderColor();
		game->screen->textSize(30);
	}

	void SpiralOfFate::SpectatorCharacterSelect::update()
	{
		this->connection.requestInputs(0);
	}

	void SpiralOfFate::SpectatorCharacterSelect::consumeEvent(const sf::Event &event)
	{
		game->P1.first->consumeEvent(event);
		game->P2.first->consumeEvent(event);
		game->P1.second->consumeEvent(event);
		game->P2.second->consumeEvent(event);
	}

	SpectatorCharacterSelect *SpectatorCharacterSelect::create(SceneArguments *args)
	{
		checked_cast(arg, SpectatorArguments, args);
		return new SpectatorCharacterSelect(*arg->connection);
	}
}