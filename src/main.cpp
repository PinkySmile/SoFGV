#include <iostream>
#include <memory>
#include "Logger.hpp"
#include "Resources/Screen.hpp"
#include "Resources/Game.hpp"

Battle::Logger	logger("./latest.log");

void	run()
{
	Battle::game.screen = std::make_unique<Battle::Screen>();
	while (Battle::game.screen->isOpen()) {
		Battle::game.screen->clear();
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