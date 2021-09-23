#include <iostream>
#include <memory>
#include "Logger.hpp"
#include "Resources/Screen.hpp"
#include "Resources/Game.hpp"
#include "Objects/ACharacter.hpp"

Battle::Logger	logger("./latest.log");

void	run()
{
	sf::View view{{-840, -480, 1680, 960}};

	Battle::game.screen = std::make_unique<Battle::Screen>();
	Battle::game.battleMgr = std::make_unique<Battle::BattleManager>(
		new Battle::ACharacter{"assets/characters/test/framedata.json"},
		new Battle::ACharacter{"assets/characters/test/framedata.json"}
	);
	Battle::game.screen->setView(view);
	while (Battle::game.screen->isOpen()) {
		Battle::game.battleMgr->update();
		Battle::game.screen->clear(sf::Color::Cyan);
		Battle::game.battleMgr->render();
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