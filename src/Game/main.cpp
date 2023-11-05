#include <iostream>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#include <crtdbg.h>
#include <direct.h>
#endif
#include <sys/stat.h>
#include <LibCore.hpp>
#include "Scenes/Scenes.hpp"
#ifdef VIRTUAL_CONTROLLER
#include "VirtualController.hpp"
#endif

#ifdef _WIN32
std::wstring getLastError(int err = GetLastError())
{
	wchar_t *s = nullptr;
	std::wstring str;

	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&s, 0, nullptr
	);
	str = s;
	LocalFree(s);
	return str;
}

LONG WINAPI UnhandledExFilter(PEXCEPTION_POINTERS ExPtr)
{
	if (!ExPtr) {
		puts("No ExPtr....");
		return 0;
	}
	puts("Caught fatal exception! Generating dump...");

	wchar_t buf[2048];
	wchar_t buf2[MAX_PATH];
	time_t timer;
	char timebuffer[31];
	struct tm* tm_info;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d-%H-%M-%S", tm_info);
	_mkdir("crashes");
	wsprintfW(buf2, L"crashes/crash_%S.dmp", timebuffer);
	wsprintfW(buf, L"Game crashed!\nReceived fatal exception %X at address %x.\n", ExPtr->ExceptionRecord->ExceptionCode, ExPtr->ExceptionRecord->ExceptionAddress);

	HANDLE hFile = CreateFileW(buf2, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile != INVALID_HANDLE_VALUE) {
		MINIDUMP_EXCEPTION_INFORMATION md;
		md.ThreadId = GetCurrentThreadId();
		md.ExceptionPointers = ExPtr;
		md.ClientPointers = FALSE;
		BOOL win = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &md, nullptr, nullptr);

		if (!win) {
			wcscat(buf, L"MiniDumpWriteDump failed.\n");
			wcscat(buf, getLastError().c_str());
			wcscat(buf, L": ");
		} else
			wcscat(buf, L"Minidump created ");
		wcscat(buf, buf2);
		CloseHandle(hFile);
	} else {
		wcscat(buf, L"CreateFileW(");
		wcscat(buf, buf2);
		wcscat(buf, L") failed: ");
		wcscat(buf, getLastError().c_str());
	}
	printf("%S\n", buf);
	MessageBoxW(nullptr, buf, L"Fatal Error", MB_ICONERROR);
	exit(ExPtr->ExceptionRecord->ExceptionCode);
}
#else
#define MessageBox(...) ((void)0)

std::string getLastError(int err = errno)
{
	return strerror(err);
}
#endif

using namespace SpiralOfFate;

std::pair<std::shared_ptr<KeyboardInput>, std::shared_ptr<ControllerInput>> loadPlayerInputs(std::ifstream &stream)
{
	std::map<InputEnum, sf::Keyboard::Key> keyboardMap{
		{ INPUT_LEFT,    sf::Keyboard::Left },
		{ INPUT_RIGHT,   sf::Keyboard::Right },
		{ INPUT_UP,      sf::Keyboard::Up },
		{ INPUT_DOWN,    sf::Keyboard::Down },
		{ INPUT_NEUTRAL, sf::Keyboard::W },
		{ INPUT_MATTER,  sf::Keyboard::X },
		{ INPUT_SPIRIT,  sf::Keyboard::C },
		{ INPUT_VOID,    sf::Keyboard::Q },
		{ INPUT_ASCEND,  sf::Keyboard::S },
		{ INPUT_DASH,    sf::Keyboard::LShift },
		{ INPUT_PAUSE,   sf::Keyboard::Tab }
	};
	std::map<InputEnum, std::pair<bool, int>> controllerMap{
		{ INPUT_LEFT,    {true,  sf::Joystick::Axis::X | (256 - 30) << 3} },
		{ INPUT_RIGHT,   {true,  sf::Joystick::Axis::X | 30 << 3} },
		{ INPUT_UP,      {true,  sf::Joystick::Axis::Y | (256 - 30) << 3} },
		{ INPUT_DOWN,    {true,  sf::Joystick::Axis::Y | 30 << 3} },
		{ INPUT_NEUTRAL, {false, 0} },
		{ INPUT_MATTER,  {false, 2} },
		{ INPUT_SPIRIT,  {false, 1} },
		{ INPUT_VOID,    {false, 3} },
		{ INPUT_ASCEND,  {true,  sf::Joystick::Z | (30 << 3)} },
		{ INPUT_DASH,    {true,  sf::Joystick::Z | ((256 - 30) << 3)} },
		{ INPUT_PAUSE,   {false, 7} }
	};
	std::map<InputEnum, ControllerKey *> realControllerMap;
	std::map<sf::Keyboard::Key, InputEnum> realKeyboardMap;

	if (!stream.fail()) {
		for (auto &pair : keyboardMap)
			stream.read(reinterpret_cast<char *>(&pair.second), sizeof(pair.second));
		for (auto &pair : controllerMap)
			stream.read(reinterpret_cast<char *>(&pair.second), sizeof(pair.second));
	}
	for (auto &pair : keyboardMap)
		realKeyboardMap[pair.second] = pair.first;
	for (auto &pair : controllerMap) {
		realControllerMap[pair.first] = pair.second.first ?
			static_cast<ControllerKey *>(new ControllerAxis(
				0,
				static_cast<sf::Joystick::Axis>(pair.second.second & 7),
				(char)(pair.second.second >> 3)
			)) :
			static_cast<ControllerKey *>(new ControllerButton(
				0,
				pair.second.second
			));
	}
	return {
		std::make_shared<KeyboardInput>(realKeyboardMap),
		std::make_shared<ControllerInput>(realControllerMap)
	};
}

std::pair<std::shared_ptr<KeyboardInput>, std::shared_ptr<ControllerInput>> loadMenuInputs(std::ifstream &stream)
{
	std::map<InputEnum, sf::Keyboard::Key> keyboardMap{
		{ INPUT_LEFT,    sf::Keyboard::Left },
		{ INPUT_RIGHT,   sf::Keyboard::Right },
		{ INPUT_UP,      sf::Keyboard::Up },
		{ INPUT_DOWN,    sf::Keyboard::Down },
		{ INPUT_NEUTRAL, sf::Keyboard::W },
		{ INPUT_SPIRIT,  sf::Keyboard::C },
		{ INPUT_PAUSE,   sf::Keyboard::Tab }
	};
	std::map<InputEnum, std::pair<bool, int>> controllerMap{
		{ INPUT_LEFT,    {true,  sf::Joystick::Axis::X | (256 - 30) << 3} },
		{ INPUT_RIGHT,   {true,  sf::Joystick::Axis::X | 30 << 3} },
		{ INPUT_UP,      {true,  sf::Joystick::Axis::Y | (256 - 30) << 3} },
		{ INPUT_DOWN,    {true,  sf::Joystick::Axis::Y | 30 << 3} },
		{ INPUT_NEUTRAL, {false, 0} },
		{ INPUT_SPIRIT,  {false, 1} },
		{ INPUT_PAUSE,   {false, 7} }
	};
	std::map<InputEnum, ControllerKey *> realControllerMap;
	std::map<sf::Keyboard::Key, InputEnum> realKeyboardMap;

	if (!stream.fail()) {
		for (auto &pair : keyboardMap)
			stream.read(reinterpret_cast<char *>(&pair.second), sizeof(pair.second));
		for (auto &pair : controllerMap)
			stream.read(reinterpret_cast<char *>(&pair.second), sizeof(pair.second));
	}
	for (auto &pair : keyboardMap)
		realKeyboardMap[pair.second] = pair.first;
	for (auto &pair : controllerMap) {
		realControllerMap[pair.first] = pair.second.first ?
			static_cast<ControllerKey *>(new ControllerAxis(
				-1,
				static_cast<sf::Joystick::Axis>(pair.second.second & 7),
				(char)(pair.second.second >> 3)
			)) :
			static_cast<ControllerKey *>(new ControllerButton(
				-1,
				pair.second.second
			));
	}
	return {
		std::make_shared<KeyboardInput>(realKeyboardMap),
		std::make_shared<ControllerInput>(realControllerMap)
	};
}

void	saveSettings()
{
	std::ofstream stream{"settings.dat", std::istream::binary};

	game->P1.first->save(stream);
	game->P1.second->save(stream);
	game->P2.first->save(stream);
	game->P2.second->save(stream);
	game->menu.first->save(stream);
	game->menu.second->save(stream);
}

void	loadSettings()
{
	std::ifstream stream{"settings.dat", std::istream::binary};

	if (stream.fail() && errno != ENOENT)
		Utils::dispMsg("Cannot load settings", "Cannot open settings file: " + std::string(strerror(errno)), MB_ICONERROR);

	struct stat s;
	auto result = stat("settings.dat", &s);

	if (result == -1) {
		if (errno != ENOENT)
			Utils::dispMsg("Cannot load settings", "Cannot stat file: " + std::string(strerror(errno)), MB_ICONERROR);
	} else if (s.st_size != 348)
		Utils::dispMsg("Cannot load settings", "Old settings or corrupted settings detected.\nYou might need to set your settings again in the menu.", MB_ICONWARNING);
	game->P1 = loadPlayerInputs(stream);
	game->P2 = loadPlayerInputs(stream);
	game->menu = loadMenuInputs(stream);
}

#ifdef __ANDROID__
static void logEvent(sf::Event &event)
{
	switch (event.type) {
	case sf::Event::Closed:                 ///< The window requested to be closed (no data)
		game->logger.debug("Closed");
		break;
	case sf::Event::Resized:                ///< The window was resized (data in event.size)
		game->logger.debug("Resized " + std::to_string(event.size.width) + "x" + std::to_string(event.size.height));
		break;
	case sf::Event::LostFocus:              ///< The window lost the focus (no data)
		game->logger.debug("LostFocus");
		break;
	case sf::Event::GainedFocus:            ///< The window gained the focus (no data)
		game->logger.debug("GainedFocus");
		break;
	case sf::Event::TextEntered:            ///< A character was entered (data in event.text)
		game->logger.debug("TextEntered " + std::to_string(event.text.unicode));
		break;
	case sf::Event::KeyPressed:             ///< A key was pressed (data in event.key)
		game->logger.debug("KeyPressed " + std::to_string(event.key.code) + ":" + (event.key.alt ? '1' : '0') + (event.key.control ? '1' : '0') + (event.key.shift ? '1' : '0') + (event.key.system ? '1' : '0'));
		break;
	case sf::Event::KeyReleased:            ///< A key was released (data in event.key)
		game->logger.debug("KeyReleased " + std::to_string(event.key.code) + ":" + (event.key.alt ? '1' : '0') + (event.key.control ? '1' : '0') + (event.key.shift ? '1' : '0') + (event.key.system ? '1' : '0'));
		break;
	case sf::Event::MouseWheelMoved:        ///< The mouse wheel was scrolled (data in event.mouseWheel) (deprecated)
		game->logger.debug("MouseWheelMoved");
		break;
	case sf::Event::MouseWheelScrolled:     ///< The mouse wheel was scrolled (data in event.mouseWheelScroll)
		game->logger.debug("MouseWheelScrolled");
		break;
	case sf::Event::MouseButtonPressed:     ///< A mouse button was pressed (data in event.mouseButton)
		game->logger.debug("MouseButtonPressed");
		break;
	case sf::Event::MouseButtonReleased:    ///< A mouse button was released (data in event.mouseButton)
		game->logger.debug("MouseButtonReleased");
		break;
	case sf::Event::MouseMoved:             ///< The mouse cursor moved (data in event.mouseMove)
		game->logger.debug("MouseMoved");
		break;
	case sf::Event::MouseEntered:           ///< The mouse cursor entered the area of the window (no data)
		game->logger.debug("MouseEntered");
		break;
	case sf::Event::MouseLeft:              ///< The mouse cursor left the area of the window (no data)
		game->logger.debug("MouseLeft");
		break;
	case sf::Event::JoystickButtonPressed:  ///< A joystick button was pressed (data in event.joystickButton)
		game->logger.debug("JoystickButtonPressed " + std::to_string(event.joystickButton.joystickId) + ": " + std::to_string(event.joystickButton.button));
		break;
	case sf::Event::JoystickButtonReleased: ///< A joystick button was released (data in event.joystickButton)
		game->logger.debug("JoystickButtonReleased " + std::to_string(event.joystickButton.joystickId) + ": " + std::to_string(event.joystickButton.button));
		break;
	case sf::Event::JoystickMoved:          ///< The joystick moved along an axis (data in event.joystickMove)
		game->logger.debug("JoystickMoved " + std::to_string(event.joystickMove.joystickId) + ": " + std::to_string(event.joystickMove.axis) + " " + std::to_string(event.joystickMove.position));
		break;
	case sf::Event::JoystickConnected:      ///< A joystick was connected (data in event.joystickConnect)
		game->logger.debug("JoystickConnected " + std::to_string(event.joystickConnect.joystickId));
		break;
	case sf::Event::JoystickDisconnected:   ///< A joystick was disconnected (data in event.joystickConnect)
		game->logger.debug("JoystickDisconnected " + std::to_string(event.joystickConnect.joystickId));
		break;
	case sf::Event::TouchBegan:             ///< A touch event began (data in event.touch)
		game->logger.debug("TouchBegan " + std::to_string(event.touch.finger) + " " + std::to_string(event.touch.x) + "," + std::to_string(event.touch.y));
		break;
	case sf::Event::TouchMoved:             ///< A touch moved (data in event.touch)
		game->logger.debug("TouchMoved " + std::to_string(event.touch.finger) + " " + std::to_string(event.touch.x) + "," + std::to_string(event.touch.y));
		break;
	case sf::Event::TouchEnded:             ///< A touch event ended (data in event.touch)
		game->logger.debug("TouchEnded " + std::to_string(event.touch.finger) + " " + std::to_string(event.touch.x) + "," + std::to_string(event.touch.y));
		break;
	case sf::Event::SensorChanged:          ///< A sensor value changed (data in event.sensor)
		game->logger.debug("SensorChanged");
		break;
	}
}
#endif

void	checkCompilationEnv()
{
	char magic[] = {0x04, 0x03, 0x02, 0x01};

	// We perform an endianness check here and display a warning if it fails.
	// The affected stuff are:
	//   - All the network stack
	//   - Generated replays
	//   - Computed state checksums
	// We officially support only little endian but people can play if they have the same endianness.
	// Regardless, the game should work in singleplayer.
	if (*(unsigned *)magic != 0x01020304)
		Utils::dispMsg(
			"Warning",
			"Your version of the game has not been compiled in " + std::string(*(unsigned *)magic == 0x04030201 ? "big endian" : "middle endian") + " but only little endian is supported\n" +
			"You will not be able to play with players using a different endianness.\n" +
			"Moreover, you won't be able to load replays generated with a different endianness.\n"
			"Your replays will also not be compatible with a different version of the game.",
			MB_ICONWARNING,
			nullptr
		);

	game->battleRandom.seed(0);
	my_assert_eq(game->battleRandom(), 2357136044UL);
	my_assert_eq(game->battleRandom(), 2546248239UL);
	my_assert_eq(game->battleRandom(), 3071714933UL);
	my_assert_eq(game->battleRandom(), 3626093760UL);
	my_assert_eq(game->battleRandom(), 2588848963UL);

	game->battleRandom.seed(0);
	my_assert_eq(game->battleRandom(), 2357136044UL);
	my_assert_eq(game->battleRandom(), 2546248239UL);
	my_assert_eq(game->battleRandom(), 3071714933UL);
	my_assert_eq(game->battleRandom(), 3626093760UL);
	my_assert_eq(game->battleRandom(), 2588848963UL);

	game->battleRandom.seed(0);
	my_assert_eq(game->battleRandom.min(), 0UL);
	my_assert_eq(game->battleRandom.max(), 0xFFFFFFFFUL);
	my_assert_eq(random_distrib(game->battleRandom, 0, 20), 10UL);
	my_assert_eq(random_distrib(game->battleRandom, 0, 20), 11UL);
	my_assert_eq(random_distrib(game->battleRandom, 0, 20), 14UL);
	my_assert_eq(random_distrib(game->battleRandom, 0, 20), 16UL);
	my_assert_eq(random_distrib(game->battleRandom, 0, 20), 12UL);
}

void	registerScenes()
{
	game->scene.registerScene("title_screen", TitleScreen::create, false);
	game->scene.registerScene("loading", LoadingScene::create, false);

	// Single player
	game->scene.registerScene("char_select", CharacterSelect::create, true);
	game->scene.registerScene("in_game", InGame::create, true);
	game->scene.registerScene("practice_in_game", PracticeInGame::create, true);
	game->scene.registerScene("replay_in_game", ReplayInGame::create, true);
#ifdef HAS_NETWORK
#ifdef _DEBUG
	game->scene.registerScene("sync_test_in_game", SyncTestInGame::create, true);
#endif

	// Netplay
	game->scene.registerScene("client_char_select", ClientCharacterSelect::create, true);
	game->scene.registerScene("server_char_select", ServerCharacterSelect::create, true);
	game->scene.registerScene("client_in_game", ClientInGame::create, true);
	game->scene.registerScene("server_in_game", ServerInGame::create, true);
#endif
}

void	run()
{
	Event event;
	sf::Image icon;
	double timer = 0;
	sf::Clock clock;
#ifdef _WIN32
	std::string font = getenv("SYSTEMROOT") + std::string("\\Fonts\\comic.ttf");
#else
	std::string font = "assets/fonts/Retro Gaming.ttf";
#endif

	checkCompilationEnv();
	loadSettings();
	registerScenes();
#ifdef VIRTUAL_CONTROLLER
	game->virtualController = std::make_shared<VirtualController>();
#endif
	if (getenv("BATTLE_FONT"))
		font = getenv("BATTLE_FONT");
	if (!game->font.loadFromFile(font))
		my_assert(game->font.loadFromFile("assets/fonts/Retro Gaming.ttf"));
	game->screen = std::make_unique<Screen>("Spiral of Fate: Grand Vision | version " VERSION_STR);
	if (icon.loadFromFile("assets/gameIcon.png"))
		game->screen->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	game->screen->setFont(game->font);
	game->scene.switchScene("title_screen");
	clock.restart();
	game->screen->setFramerateLimit(60);
	while (game->screen->isOpen()) {
		game->sceneMutex.lock();
	#ifdef HAS_NETWORK
		if (game->connection)
			game->connection->update();
	#endif

		timer += clock.restart().asSeconds();
		while (timer >= 1. / 60.) {
		#ifdef VIRTUAL_CONTROLLER
			game->virtualController->onFrameStart();
			game->virtualController->update();
		#endif
			game->scene.update();
			timer -= 1. / 60.;
		}
		game->scene.render();
	#ifdef VIRTUAL_CONTROLLER
		game->virtualController->render();
	#endif
		game->screen->display();

		while (game->screen->pollEvent(event)) {
			if (event.type == EVENT_WINDOW_CLOSED)
				game->screen->close();
		#ifdef USE_SFML
			game->scene.consumeEvent(event);
		#endif
		#ifdef VIRTUAL_CONTROLLER
			game->virtualController->consumeEvent(event);
		#endif
		}
		game->sceneMutex.unlock();
	}
	saveSettings();
}

int	main()
{
	int ret = EXIT_SUCCESS;

	libraryInit();
#ifdef _WIN32
	SetUnhandledExceptionFilter(UnhandledExFilter);
#endif

#if !defined(_DEBUG) || defined(_WIN32) || defined(__ANDROID__)
	try {
#endif
		new Game();
		game->logger.info("Starting game->");
		run();
		game->logger.info("Goodbye !");
#if !defined(_DEBUG) || defined(_WIN32) || defined(__ANDROID__)
	} catch (std::exception &e) {
		if (game) {
			game->logger.fatal(e.what());
			Utils::dispMsg("Fatal error", e.what(), MB_ICONERROR, &*game->screen);
		} else
			Utils::dispMsg("Fatal error", e.what(), MB_ICONERROR, nullptr);
		ret = EXIT_FAILURE;
	}
#endif
	delete game;
#ifdef __ANDROID__
	// In android, it is possible to exit the app without killing the process.
	// The main gets called again when the app is restarted so we need to make sure the global is set to null.
	game = nullptr;
#endif
	libraryUnInit();
	return ret;
}
