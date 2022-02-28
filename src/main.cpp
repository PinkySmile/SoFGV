#include <iostream>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#include <crtdbg.h>
#endif
#include <sys/stat.h>
#include "Logger.hpp"
#include "Resources/Screen.hpp"
#include "Resources/Game.hpp"
#include "Scenes/TitleScreen.hpp"
#include "Inputs/KeyboardInput.hpp"
#include "Inputs/ControllerInput.hpp"

#ifdef _WIN32
std::string getLastError(int err = GetLastError())
{
	char *s = nullptr;
	std::string str;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&s, 0, nullptr
	);
	str = s;
	LocalFree(s);
	return str;
#else
#define MessageBox(...) ((void)0)
std::string getLastError(int err = errno)
{
	return strerror(err);
#endif
}

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
	sprintf(buf2, "crash_%s.dmp", timebuffer);

	HANDLE hFile = CreateFile(buf2, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION md;
		md.ThreadId = GetCurrentThreadId();
		md.ExceptionPointers = ExPtr;
		md.ClientPointers = FALSE;
		BOOL win = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &md, 0, 0);

		if (!win)
			sprintf(buf, "Le jeu a un peu crash en fait.\nMiniDumpWriteDump failed.\n%s: %s", getLastError().c_str(), buf2);
		else
			sprintf(buf, "Le jeu a un peu crash en fait.\nMinidump created %s", buf2);
		CloseHandle(hFile);
	} else
		sprintf(buf, "Le jeu a un peu crash en fait.\nCould not create file %s\n%s", buf2, getLastError().c_str());
	MessageBox(nullptr, buf, "Alors...", MB_ICONERROR);
	Battle::game.logger.fatal(buf);
	exit(EXIT_FAILURE);
}
#endif

std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> loadPlayerInputs(std::ifstream &stream, bool oldVersion)
{
	std::map<Battle::InputEnum, sf::Keyboard::Key> keyboardMap{
		{ Battle::INPUT_LEFT,    sf::Keyboard::Left },
		{ Battle::INPUT_RIGHT,   sf::Keyboard::Right },
		{ Battle::INPUT_UP,      sf::Keyboard::Up },
		{ Battle::INPUT_DOWN,    sf::Keyboard::Down },
		{ Battle::INPUT_NEUTRAL, sf::Keyboard::W },
		{ Battle::INPUT_MATTER,  sf::Keyboard::X },
		{ Battle::INPUT_SPIRIT,  sf::Keyboard::C },
		{ Battle::INPUT_VOID,    sf::Keyboard::Q },
		{ Battle::INPUT_ASCEND,  sf::Keyboard::S },
		{ Battle::INPUT_DASH,    sf::Keyboard::LShift }
	};
	std::map<Battle::InputEnum, std::pair<bool, int>> controllerMap{
		{ Battle::INPUT_LEFT,    {true,  sf::Joystick::Axis::X | (256 - 30) << 3} },
		{ Battle::INPUT_RIGHT,   {true,  sf::Joystick::Axis::X | 30 << 3} },
		{ Battle::INPUT_UP,      {true,  sf::Joystick::Axis::Y | (256 - 30) << 3} },
		{ Battle::INPUT_DOWN,    {true,  sf::Joystick::Axis::Y | 30 << 3} },
		{ Battle::INPUT_NEUTRAL, {false, 0} },
		{ Battle::INPUT_MATTER,  {false, 2} },
		{ Battle::INPUT_SPIRIT,  {false, 1} },
		{ Battle::INPUT_VOID,    {false, 3} },
		{ Battle::INPUT_ASCEND,  {true,  sf::Joystick::Z | (30 << 3)} },
		{ Battle::INPUT_DASH,    {true,  sf::Joystick::Z | ((256 - 30) << 3)} },
	};
	std::map<Battle::InputEnum, Battle::ControllerKey *> realControllerMap;
	std::map<sf::Keyboard::Key, Battle::InputEnum> realKeyboardMap;

	if (!oldVersion) {
		keyboardMap[Battle::INPUT_PAUSE] = sf::Keyboard::Tab;
		controllerMap[Battle::INPUT_PAUSE] = {false, 7};
	}
	if (!stream.fail()) {
		for (auto &pair : keyboardMap)
			stream.read(reinterpret_cast<char *>(&pair.second), sizeof(pair.second));
		for (auto &pair : controllerMap)
			stream.read(reinterpret_cast<char *>(&pair.second), sizeof(pair.second));
	}
	if (oldVersion) {
		keyboardMap[Battle::INPUT_PAUSE] = sf::Keyboard::Tab;
		controllerMap[Battle::INPUT_PAUSE] = {false, 7};
	}
	for (auto &pair : keyboardMap)
		realKeyboardMap[pair.second] = pair.first;
	for (auto &pair : controllerMap) {
		realControllerMap[pair.first] = pair.second.first ?
			static_cast<Battle::ControllerKey *>(new Battle::ControllerAxis(
				0,
				static_cast<sf::Joystick::Axis>(pair.second.second & 7),
				(char)(pair.second.second >> 3)
			)) :
			static_cast<Battle::ControllerKey *>(new Battle::ControllerButton(
				0,
				pair.second.second
			));
	}
	return {
		std::make_shared<Battle::KeyboardInput>(realKeyboardMap),
		std::make_shared<Battle::ControllerInput>(realControllerMap)
	};
}

void	saveSettings()
{
	std::ofstream stream{"settings.dat", std::istream::binary};

	Battle::game.P1.first->save(stream);
	Battle::game.P1.second->save(stream);
	Battle::game.P2.first->save(stream);
	Battle::game.P2.second->save(stream);
}

void	loadSettings()
{
	std::ifstream stream{"settings.dat", std::istream::binary};
	struct stat s;
	auto result = stat("settings.dat", &s);

	Battle::game.P1 = loadPlayerInputs(stream, result != -1 && s.st_size == 240);
	Battle::game.P2 = loadPlayerInputs(stream, result != -1 && s.st_size == 240);
}

void	run()
{
	sf::Event event;

	loadSettings();
#ifdef _WIN32
	std::string font = getenv("SYSTEMROOT") + std::string("\\Fonts\\comic.ttf");
#else
	std::string font = "/usr/share/fonts/TTF/DejaVuSerif.ttf";
#endif
	if (getenv("BATTLE_FONT"))
		font = getenv("BATTLE_FONT");
	Battle::game.font.loadFromFile(font);
	Battle::game.screen = std::make_unique<Battle::Screen>("Le jeu de combat de Pinky et le second degr\xE9");
	Battle::game.screen->setFont(Battle::game.font);
	Battle::game.scene = std::make_unique<Battle::TitleScreen>(Battle::game.P1, Battle::game.P2);
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
		Battle::game.networkMgr.update();
	}
	saveSettings();
}

int	main()
{
#ifdef _WIN32
	SetUnhandledExceptionFilter(UnhandledExFilter);
#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif
	Battle::game.logger.info("Starting game.");
	#ifndef _DEBUG
	try {
	#endif
		run();
	#ifndef _DEBUG
	} catch (std::exception &e) {
		Battle::game.logger.fatal(e.what());
		MessageBox(nullptr, e.what(), "Fatal error", MB_ICONERROR);
	}
	#endif
	Battle::game.logger.info("Goodbye !");
	return EXIT_SUCCESS;
}
