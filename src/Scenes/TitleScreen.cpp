//
// Created by Gegel85 on 24/09/2021.
//

#include "TitleScreen.hpp"
#include "../Resources/Game.hpp"
#include "InGame.hpp"
#include "../Logger.hpp"

namespace Battle
{
	TitleScreen::TitleScreen()
	{
		logger.info("Title scene created");
		this->_font.loadFromFile("assets/fonts/comicsansms.ttf");
	}

	void TitleScreen::render() const
	{
		game.screen->setFont(this->_font);

		game.screen->fillColor(this->_selectedEntry == 0 ? sf::Color::Red : sf::Color::Black);
		game.screen->displayElement("Play", {100, 100});

		game.screen->fillColor(this->_selectedEntry == 1 ? sf::Color::Red : sf::Color::Black);
		game.screen->displayElement("Settings", {100, 140});

		game.screen->fillColor(this->_selectedEntry == 2 ? sf::Color::Red : sf::Color::Black);
		game.screen->displayElement("Quit", {100, 180});
	}

	IScene *TitleScreen::update()
	{
		return this->_nextScene;
	}

	void TitleScreen::consumeEvent(const sf::Event &event)
	{
		if (this->_nextScene)
			return;
		switch (event.type) {
		case sf::Event::KeyPressed:
			switch (event.key.code) {
			case sf::Keyboard::Up:
				this->_selectedEntry += 3;
				this->_selectedEntry--;
				this->_selectedEntry %= 3;
				break;
			case sf::Keyboard::Down:
				this->_selectedEntry++;
				this->_selectedEntry %= 3;
				break;
			case sf::Keyboard::X:
				this->_selectedEntry = 2;
				break;
			case sf::Keyboard::Z:
				this->_onConfirm();
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}

	void TitleScreen::_onConfirm()
	{
		switch (this->_selectedEntry) {
		case 0:
			this->_nextScene = new InGame();
			break;
		case 1:
			break;
		case 2:
			game.screen->close();
			break;
		default:
			break;
		}
	}
}