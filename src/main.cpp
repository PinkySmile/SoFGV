#include <iostream>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#endif
#include "Logger.hpp"
#include "Resources/Screen.hpp"
#include "Resources/Game.hpp"
#include "Scenes/TitleScreen.hpp"

Battle::Logger	logger("./latest.log");

void	run()
{
	sf::Event event;

	Battle::game.screen = std::make_unique<Battle::Screen>();
	Battle::game.scene = std::make_unique<Battle::TitleScreen>();
	while (Battle::game.screen->isOpen()) {
		Battle::IScene *newScene = Battle::game.scene->update();

		Battle::game.screen->clear(sf::Color::White);
		Battle::game.scene->render();
		Battle::game.screen->display();

		while (Battle::game.screen->pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				Battle::game.screen->close();
			Battle::game.scene->consumeEvent(event);
		}
		if (newScene)
			Battle::game.scene.reset(newScene);
	}
}

int	main()
{
	logger.info("Starting game.");
	#ifdef _WIN32
	try {
	#endif
		run();
	#ifdef _WIN32
	} catch (std::exception &e) {
		MessageBox(nullptr, e.what(), "Fatal error", MB_ICONERROR);
	}
	#endif
	logger.info("Goodbye !");
	return EXIT_SUCCESS;
}