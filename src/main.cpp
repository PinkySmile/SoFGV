#include <iostream>
#include <memory>
#include "Logger.hpp"
#include "Resources/Screen.hpp"
#include "Resources/Game.hpp"
#include "Objects/ACharacter.hpp"

Battle::Logger	logger("./latest.log");

void	run()
{
	Battle::game.screen = std::make_unique<Battle::Screen>();

	sf::View view{{-840, -480, 1680, 960}};
	Battle::ACharacter character{"assets/characters/test/framedata.json"};

	Battle::game.screen->setView(view);
	while (Battle::game.screen->isOpen()) {
		character.update();
		Battle::game.screen->clear(sf::Color::Cyan);
		character.render();
		Battle::game.screen->handleEvents();
		Battle::game.screen->display();
	}
}

int	main()
{
	logger.info("Starting game.");
	run();
	logger.info("Goodbye !");
	return EXIT_SUCCESS;
}