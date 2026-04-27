#include "LibCore.hpp"
#include "FrameDataEditor.hpp"

using namespace SpiralOfFate;

FrameDataEditor *editor;

void run()
{
	while (game->screen->isOpen()) {
		try {
			editor->update();
			game->screen->clear(sf::Color{0x40, 0x40, 0x40, 0xFF});
			editor->render();
			game->gui.draw();
			game->screen->display();
			while (auto event = game->screen->pollEvent()) {
				if (event->is<EVENT_WINDOW_CLOSED>() && editor->closeAll())
					game->screen->close();
				if (auto key = event->getIf<sf::Event::KeyPressed>()) {
					if (editor->canHandleKeyPress(*key)) {
						editor->keyPressed(*key);
						continue;
					}
				}
				if (auto key = event->getIf<sf::Event::KeyReleased>()) {
					editor->keyReleased(*key);
				}
				if (auto mouse = event->getIf<sf::Event::MouseMoved>())
					editor->mouseMovedAbsolute(tgui::Vector2f(mouse->position.x, mouse->position.y));
				game->gui.handleEvent(*event);
			}
		} catch (std::exception &e) {
			Utils::dispMsg(game->gui, "Error", std::string("Uncaught exception in main loop: ") + e.what(), MB_ICONERROR);
		}
	}
}

void initEditor()
{
	sf::Image icon;

	new Game("assets/fonts/Retro Gaming.ttf", "assets/fonts/Retro Gaming.ttf", "settings/settings.json");
	game->logger.info("Starting editor.");
	game->screen = std::make_unique<Screen>("Spiral of Fate: Grand Vision | FrameData Editor");
	if (icon.loadFromFile("assets/editorIcon.png"))
		game->screen->setIcon(icon.getSize(), icon.getPixelsPtr());
	game->screen->setFramerateLimit(240);
	editor = new FrameDataEditor();
}

int main()
{
#ifdef _DEBUG
	if (Utils::isBeingDebugged()) {
		initEditor();
		run();
		game->logger.info("Goodbye !");
		delete editor;
		delete game;
		return EXIT_SUCCESS;
	}
#endif
	try {
		initEditor();
		run();
	} catch (std::exception &e) {
		if (game)
			game->logger.fatal(e.what());
		else
			std::cout << e.what() << std::endl;
		delete game;
		return EXIT_FAILURE;
	}
	game->logger.info("Goodbye !");
	delete editor;
	delete game;
	return EXIT_SUCCESS;
}