#include <iostream>
#include <memory>
#include <TGUI/TGUI.hpp>
#include "Logger.hpp"
#include "Resources/Screen.hpp"
#include "Resources/Game.hpp"
#include "Objects/ACharacter.hpp"

Battle::Logger logger("./editor.log");
float updateTimer = 0;
float timer = 0;
bool dragUp = false;
bool dragDown = false;
bool dragLeft = false;
bool dragRight = false;
std::unique_ptr<Battle::ACharacter> character;
Battle::Box *selectedBox;
Battle::Box startValues;
std::array<tgui::Button::Ptr, 8> resizeButtons;

void	placeGuiHooks(tgui::Gui &gui)
{
	auto bar = gui.get<tgui::MenuBar>("main_bar");
	auto panel = gui.get<tgui::Panel>("Panel1");

	bar->connectMenuItem({"File", "New framedata"}, [bar]{
	});

	for (unsigned i = 0; i < resizeButtons.size(); i++) {
		auto &resizeButton = resizeButtons[i];

		resizeButton = tgui::Button::create();
		resizeButton->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
		resizeButton->getRenderer()->setBorderColor(tgui::Color::Red);
		resizeButton->connect("MousePressed", [i]{
			startValues = *selectedBox;
			dragUp = i < 3;
			dragDown = i > 4;
			dragLeft = i == 0 || i == 3 || i == 5;
			dragRight = i == 2 || i == 4 || i == 7;
		});
	}
}

void	run()
{
	Battle::game.screen = std::make_unique<Battle::Screen>("Frame data editor");

	tgui::Gui gui{*Battle::game.screen};
	sf::View view{{-840, -480, 1680, 960}};
	sf::Image icon;
	sf::Event event;

	if (icon.loadFromFile("assets/editorIcon.png"))
		Battle::game.screen->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	gui.loadWidgetsFromFile("assets/gui/editor.gui");
	placeGuiHooks(gui);
	Battle::game.screen->setView(view);
	while (Battle::game.screen->isOpen()) {
		timer++;
		Battle::game.screen->clear(sf::Color::Cyan);
		if (character) {
			if (timer >= updateTimer) {
				character->update();
				timer -= updateTimer;
			}
			character->render();
		}

		while (Battle::game.screen->pollEvent(event)) {
			gui.handleEvent(event);
			if (event.type == sf::Event::Closed)
				Battle::game.screen->close();
		}
		gui.draw();
		Battle::game.screen->display();
	}
}

int	main()
{
	logger.info("Starting editor.");
	run();
	logger.info("Goodbye !");
	return EXIT_SUCCESS;
}