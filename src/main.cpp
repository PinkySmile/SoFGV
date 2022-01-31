#include <iostream>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#endif
#include "Logger.hpp"
#include "Resources/Screen.hpp"
#include "Resources/Game.hpp"
#include "Scenes/TitleScreen.hpp"

Battle::Logger	logger("./latest.log");

#ifdef _WIN32
LONG WINAPI UnhandledExFilter(PEXCEPTION_POINTERS ExPtr)
{
	char buf[MAX_PATH], buf2[MAX_PATH];

	time_t timer;
	char timebuffer[26];
	struct tm* tm_info;
	time(&timer);
	tm_info = localtime(&timer);
	strftime(timebuffer, 26, "%Y%m%d%H%M%S", tm_info);
	HANDLE hFile = CreateFile(buf2, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	sprintf(buf2, "crash_%s.dmp", timebuffer);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION md;
		md.ThreadId = GetCurrentThreadId();
		md.ExceptionPointers = ExPtr;
		md.ClientPointers = FALSE;
		BOOL win = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &md, 0, 0);

		if (!win)
			sprintf(buf, "Le jeu à un peu crash en fait. MiniDumpWriteDump failed. Error: %lu \n(%s)", GetLastError(), buf2);
		else
			sprintf(buf, "Le jeu à un peu crash en fait. Minidump created:\n%s", buf2);
		CloseHandle(hFile);

	} else
		sprintf(buf, "Le jeu à un peu crash en fait. Could not create minidump:\n%s", buf2);
	MessageBox(nullptr, "Alors...", buf, MB_ICONERROR);
	logger.fatal(buf);
	exit(EXIT_FAILURE);    //do whatever u want here
}
#endif

void	run()
{
	sf::Event event;

	Battle::game.screen = std::make_unique<Battle::Screen>("Le jeu de combat de Pinky et le second degr\xE9");
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
#ifdef _WIN32
	SetUnhandledExceptionFilter(UnhandledExFilter);
#endif
	logger.info("Starting game.");
	#ifndef _DEBUG
	try {
	#endif
		run();
	#ifndef _DEBUG
	} catch (std::exception &e) {
		logger.fatal(e.what());
		MessageBox(nullptr, e.what(), "Fatal error", MB_ICONERROR);
	}
	#endif
	logger.info("Goodbye !");
	return EXIT_SUCCESS;
}