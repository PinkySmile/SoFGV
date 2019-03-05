#include <SFML/Graphics.hpp>
#include <iostream>
#include "Logger.hpp"
#include "Screen.hpp"

Logger	logger("./latest.log");

void	game()
{
	Screen	screen;

	while (screen.isOpen()) {
		screen.clear();
		screen.handleEvents();
		screen.display();
	}
}

int	main()
{
	logger.info("Starting game.");
	game();
	logger.info("Goodbye !");
	return EXIT_SUCCESS;
}