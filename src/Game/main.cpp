#include <iostream>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#include <crtdbg.h>
#include <direct.h>
#endif
#include <sys/stat.h>
#include "../LibCore/LibCore.hpp"

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
#endif

std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> loadPlayerInputs(std::ifstream &stream)
{
	std::map<SpiralOfFate::InputEnum, sf::Keyboard::Key> keyboardMap{
		{ SpiralOfFate::INPUT_LEFT,    sf::Keyboard::Left },
		{ SpiralOfFate::INPUT_RIGHT,   sf::Keyboard::Right },
		{ SpiralOfFate::INPUT_UP,      sf::Keyboard::Up },
		{ SpiralOfFate::INPUT_DOWN,    sf::Keyboard::Down },
		{ SpiralOfFate::INPUT_NEUTRAL, sf::Keyboard::W },
		{ SpiralOfFate::INPUT_MATTER,  sf::Keyboard::X },
		{ SpiralOfFate::INPUT_SPIRIT,  sf::Keyboard::C },
		{ SpiralOfFate::INPUT_VOID,    sf::Keyboard::Q },
		{ SpiralOfFate::INPUT_ASCEND,  sf::Keyboard::S },
		{ SpiralOfFate::INPUT_DASH,    sf::Keyboard::LShift },
		{ SpiralOfFate::INPUT_PAUSE,   sf::Keyboard::Tab }
	};
	std::map<SpiralOfFate::InputEnum, std::pair<bool, int>> controllerMap{
		{ SpiralOfFate::INPUT_LEFT,    {true,  sf::Joystick::Axis::X | (256 - 30) << 3} },
		{ SpiralOfFate::INPUT_RIGHT,   {true,  sf::Joystick::Axis::X | 30 << 3} },
		{ SpiralOfFate::INPUT_UP,      {true,  sf::Joystick::Axis::Y | (256 - 30) << 3} },
		{ SpiralOfFate::INPUT_DOWN,    {true,  sf::Joystick::Axis::Y | 30 << 3} },
		{ SpiralOfFate::INPUT_NEUTRAL, {false, 0} },
		{ SpiralOfFate::INPUT_MATTER,  {false, 2} },
		{ SpiralOfFate::INPUT_SPIRIT,  {false, 1} },
		{ SpiralOfFate::INPUT_VOID,    {false, 3} },
		{ SpiralOfFate::INPUT_ASCEND,  {true,  sf::Joystick::Z | (30 << 3)} },
		{ SpiralOfFate::INPUT_DASH,    {true,  sf::Joystick::Z | ((256 - 30) << 3)} },
		{ SpiralOfFate::INPUT_PAUSE,   {false, 7} }
	};
	std::map<SpiralOfFate::InputEnum, SpiralOfFate::ControllerKey *> realControllerMap;
	std::map<sf::Keyboard::Key, SpiralOfFate::InputEnum> realKeyboardMap;

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
			static_cast<SpiralOfFate::ControllerKey *>(new SpiralOfFate::ControllerAxis(
				0,
				static_cast<sf::Joystick::Axis>(pair.second.second & 7),
				(char)(pair.second.second >> 3)
			)) :
			static_cast<SpiralOfFate::ControllerKey *>(new SpiralOfFate::ControllerButton(
				0,
				pair.second.second
			));
	}
	return {
		std::make_shared<SpiralOfFate::KeyboardInput>(realKeyboardMap),
		std::make_shared<SpiralOfFate::ControllerInput>(realControllerMap)
	};
}

std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> loadMenuInputs(std::ifstream &stream)
{
	std::map<SpiralOfFate::InputEnum, sf::Keyboard::Key> keyboardMap{
		{ SpiralOfFate::INPUT_LEFT,    sf::Keyboard::Left },
		{ SpiralOfFate::INPUT_RIGHT,   sf::Keyboard::Right },
		{ SpiralOfFate::INPUT_UP,      sf::Keyboard::Up },
		{ SpiralOfFate::INPUT_DOWN,    sf::Keyboard::Down },
		{ SpiralOfFate::INPUT_NEUTRAL, sf::Keyboard::W },
		{ SpiralOfFate::INPUT_SPIRIT,  sf::Keyboard::C },
		{ SpiralOfFate::INPUT_PAUSE,   sf::Keyboard::Tab }
	};
	std::map<SpiralOfFate::InputEnum, std::pair<bool, int>> controllerMap{
		{ SpiralOfFate::INPUT_LEFT,    {true,  sf::Joystick::Axis::X | (256 - 30) << 3} },
		{ SpiralOfFate::INPUT_RIGHT,   {true,  sf::Joystick::Axis::X | 30 << 3} },
		{ SpiralOfFate::INPUT_UP,      {true,  sf::Joystick::Axis::Y | (256 - 30) << 3} },
		{ SpiralOfFate::INPUT_DOWN,    {true,  sf::Joystick::Axis::Y | 30 << 3} },
		{ SpiralOfFate::INPUT_NEUTRAL, {false, 0} },
		{ SpiralOfFate::INPUT_SPIRIT,  {false, 1} },
		{ SpiralOfFate::INPUT_PAUSE,   {false, 7} }
	};
	std::map<SpiralOfFate::InputEnum, SpiralOfFate::ControllerKey *> realControllerMap;
	std::map<sf::Keyboard::Key, SpiralOfFate::InputEnum> realKeyboardMap;

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
			static_cast<SpiralOfFate::ControllerKey *>(new SpiralOfFate::ControllerAxis(
				-1,
				static_cast<sf::Joystick::Axis>(pair.second.second & 7),
				(char)(pair.second.second >> 3)
			)) :
			static_cast<SpiralOfFate::ControllerKey *>(new SpiralOfFate::ControllerButton(
				-1,
				pair.second.second
			));
	}
	return {
		std::make_shared<SpiralOfFate::KeyboardInput>(realKeyboardMap),
		std::make_shared<SpiralOfFate::ControllerInput>(realControllerMap)
	};
}

void	saveSettings()
{
	std::ofstream stream{"settings.dat", std::istream::binary};

	SpiralOfFate::game->P1.first->save(stream);
	SpiralOfFate::game->P1.second->save(stream);
	SpiralOfFate::game->P2.first->save(stream);
	SpiralOfFate::game->P2.second->save(stream);
	SpiralOfFate::game->menu.first->save(stream);
	SpiralOfFate::game->menu.second->save(stream);
}

void	loadSettings()
{
	std::ifstream stream{"settings.dat", std::istream::binary};

	if (stream.fail() && errno != ENOENT)
		SpiralOfFate::Utils::dispMsg("Cannot load settings", "Cannot open settings file: " + std::string(strerror(errno)), MB_ICONERROR);

	struct stat s;
	auto result = stat("settings.dat", &s);

	if (result == -1) {
		if (errno != ENOENT)
			SpiralOfFate::Utils::dispMsg("Cannot load settings", "Cannot stat file: " + std::string(strerror(errno)), MB_ICONERROR);
	} else if (s.st_size != 348)
		SpiralOfFate::Utils::dispMsg("Cannot load settings", "Old settings or corrupted settings detected.\nYou might need to set your settings again in the menu.", MB_ICONWARNING);
	SpiralOfFate::game->P1 = loadPlayerInputs(stream);
	SpiralOfFate::game->P2 = loadPlayerInputs(stream);
	SpiralOfFate::game->menu = loadMenuInputs(stream);
}

#ifdef __ANDROID__
static void logEvent(sf::Event &event)
{
	switch (event.type) {
	case sf::Event::Closed:                 ///< The window requested to be closed (no data)
		SpiralOfFate::game->logger.debug("Closed");
		break;
	case sf::Event::Resized:                ///< The window was resized (data in event.size)
		SpiralOfFate::game->logger.debug("Resized " + std::to_string(event.size.width) + "x" + std::to_string(event.size.height));
		break;
	case sf::Event::LostFocus:              ///< The window lost the focus (no data)
		SpiralOfFate::game->logger.debug("LostFocus");
		break;
	case sf::Event::GainedFocus:            ///< The window gained the focus (no data)
		SpiralOfFate::game->logger.debug("GainedFocus");
		break;
	case sf::Event::TextEntered:            ///< A character was entered (data in event.text)
		SpiralOfFate::game->logger.debug("TextEntered " + std::to_string(event.text.unicode));
		break;
	case sf::Event::KeyPressed:             ///< A key was pressed (data in event.key)
		SpiralOfFate::game->logger.debug("KeyPressed " + std::to_string(event.key.code) + ":" + (event.key.alt ? '1' : '0') + (event.key.control ? '1' : '0') + (event.key.shift ? '1' : '0') + (event.key.system ? '1' : '0'));
		break;
	case sf::Event::KeyReleased:            ///< A key was released (data in event.key)
		SpiralOfFate::game->logger.debug("KeyReleased " + std::to_string(event.key.code) + ":" + (event.key.alt ? '1' : '0') + (event.key.control ? '1' : '0') + (event.key.shift ? '1' : '0') + (event.key.system ? '1' : '0'));
		break;
	case sf::Event::MouseWheelMoved:        ///< The mouse wheel was scrolled (data in event.mouseWheel) (deprecated)
		SpiralOfFate::game->logger.debug("MouseWheelMoved");
		break;
	case sf::Event::MouseWheelScrolled:     ///< The mouse wheel was scrolled (data in event.mouseWheelScroll)
		SpiralOfFate::game->logger.debug("MouseWheelScrolled");
		break;
	case sf::Event::MouseButtonPressed:     ///< A mouse button was pressed (data in event.mouseButton)
		SpiralOfFate::game->logger.debug("MouseButtonPressed");
		break;
	case sf::Event::MouseButtonReleased:    ///< A mouse button was released (data in event.mouseButton)
		SpiralOfFate::game->logger.debug("MouseButtonReleased");
		break;
	case sf::Event::MouseMoved:             ///< The mouse cursor moved (data in event.mouseMove)
		SpiralOfFate::game->logger.debug("MouseMoved");
		break;
	case sf::Event::MouseEntered:           ///< The mouse cursor entered the area of the window (no data)
		SpiralOfFate::game->logger.debug("MouseEntered");
		break;
	case sf::Event::MouseLeft:              ///< The mouse cursor left the area of the window (no data)
		SpiralOfFate::game->logger.debug("MouseLeft");
		break;
	case sf::Event::JoystickButtonPressed:  ///< A joystick button was pressed (data in event.joystickButton)
		SpiralOfFate::game->logger.debug("JoystickButtonPressed " + std::to_string(event.joystickButton.joystickId) + ": " + std::to_string(event.joystickButton.button));
		break;
	case sf::Event::JoystickButtonReleased: ///< A joystick button was released (data in event.joystickButton)
		SpiralOfFate::game->logger.debug("JoystickButtonReleased " + std::to_string(event.joystickButton.joystickId) + ": " + std::to_string(event.joystickButton.button));
		break;
	case sf::Event::JoystickMoved:          ///< The joystick moved along an axis (data in event.joystickMove)
		SpiralOfFate::game->logger.debug("JoystickMoved " + std::to_string(event.joystickMove.joystickId) + ": " + std::to_string(event.joystickMove.axis) + " " + std::to_string(event.joystickMove.position));
		break;
	case sf::Event::JoystickConnected:      ///< A joystick was connected (data in event.joystickConnect)
		SpiralOfFate::game->logger.debug("JoystickConnected " + std::to_string(event.joystickConnect.joystickId));
		break;
	case sf::Event::JoystickDisconnected:   ///< A joystick was disconnected (data in event.joystickConnect)
		SpiralOfFate::game->logger.debug("JoystickDisconnected " + std::to_string(event.joystickConnect.joystickId));
		break;
	case sf::Event::TouchBegan:             ///< A touch event began (data in event.touch)
		SpiralOfFate::game->logger.debug("TouchBegan " + std::to_string(event.touch.finger) + " " + std::to_string(event.touch.x) + "," + std::to_string(event.touch.y));
		break;
	case sf::Event::TouchMoved:             ///< A touch moved (data in event.touch)
		SpiralOfFate::game->logger.debug("TouchMoved " + std::to_string(event.touch.finger) + " " + std::to_string(event.touch.x) + "," + std::to_string(event.touch.y));
		break;
	case sf::Event::TouchEnded:             ///< A touch event ended (data in event.touch)
		SpiralOfFate::game->logger.debug("TouchEnded " + std::to_string(event.touch.finger) + " " + std::to_string(event.touch.x) + "," + std::to_string(event.touch.y));
		break;
	case sf::Event::SensorChanged:          ///< A sensor value changed (data in event.sensor)
		SpiralOfFate::game->logger.debug("SensorChanged");
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
		SpiralOfFate::Utils::dispMsg(
			"Warning",
			"Your version of the game has not been compiled in " + std::string(*(unsigned *)magic == 0x04030201 ? "big endian" : "middle endian") + " but only little endian is supported\n" +
			"You will not be able to play with players using a different endianness.\n" +
			"Moreover, you won't be able to load replays generated with a different endianness.\n"
			"Your replays will also not be compatible with a different version of the game.",
			MB_ICONWARNING,
			nullptr
		);

	SpiralOfFate::game->battleRandom.seed(0);
	my_assert_eq(SpiralOfFate::game->battleRandom(), 2357136044UL);
	my_assert_eq(SpiralOfFate::game->battleRandom(), 2546248239UL);
	my_assert_eq(SpiralOfFate::game->battleRandom(), 3071714933UL);
	my_assert_eq(SpiralOfFate::game->battleRandom(), 3626093760UL);
	my_assert_eq(SpiralOfFate::game->battleRandom(), 2588848963UL);

	SpiralOfFate::game->battleRandom.seed(0);
	my_assert_eq(SpiralOfFate::game->battleRandom.min(), 0UL);
	my_assert_eq(SpiralOfFate::game->battleRandom.max(), 0xFFFFFFFFUL);
	my_assert_eq(random_distrib(SpiralOfFate::game->battleRandom, 0, 20), 10UL);
	my_assert_eq(random_distrib(SpiralOfFate::game->battleRandom, 0, 20), 11UL);
	my_assert_eq(random_distrib(SpiralOfFate::game->battleRandom, 0, 20), 14UL);
	my_assert_eq(random_distrib(SpiralOfFate::game->battleRandom, 0, 20), 16UL);
	my_assert_eq(random_distrib(SpiralOfFate::game->battleRandom, 0, 20), 12UL);
}

void	run()
{
	sf::Event event;
	sf::Image icon;
#ifdef _WIN32
	std::string font = getenv("SYSTEMROOT") + std::string("\\Fonts\\comic.ttf");
#else
	std::string font = "assets/fonts/Retro Gaming.ttf";
#endif

	checkCompilationEnv();
	loadSettings();
	if (getenv("BATTLE_FONT"))
		font = getenv("BATTLE_FONT");
	if (!SpiralOfFate::game->font.loadFromFile(font))
		my_assert(SpiralOfFate::game->font.loadFromFile("assets/fonts/Retro Gaming.ttf"));
	SpiralOfFate::game->screen = std::make_unique<SpiralOfFate::Screen>("Spiral of Fate: Grand Vision | version " VERSION_STR);
	if (icon.loadFromFile("assets/gameIcon.png"))
		SpiralOfFate::game->screen->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	SpiralOfFate::game->screen->setFont(SpiralOfFate::game->font);
	SpiralOfFate::game->scene = std::make_unique<SpiralOfFate::TitleScreen>(SpiralOfFate::game->P1, SpiralOfFate::game->P2);
	while (SpiralOfFate::game->screen->isOpen()) {
		SpiralOfFate::game->sceneMutex.lock();
		if (SpiralOfFate::game->connection)
			SpiralOfFate::game->connection->update();

		SpiralOfFate::IScene *newScene = SpiralOfFate::game->scene->update();

		if (newScene)
			SpiralOfFate::game->scene.reset(newScene);
		SpiralOfFate::game->screen->clear(sf::Color::White);
		SpiralOfFate::game->scene->render();
		SpiralOfFate::game->screen->display();

		while (SpiralOfFate::game->screen->pollEvent(event)) {
#ifdef __ANDROID__
			logEvent(event);
#endif
			if (event.type == sf::Event::Closed)
				SpiralOfFate::game->screen->close();
			SpiralOfFate::game->scene->consumeEvent(event);
		}
		SpiralOfFate::game->sceneMutex.unlock();
	}
	saveSettings();
}

int	main()
{
	int ret = EXIT_SUCCESS;

#ifdef _WIN32
	SetUnhandledExceptionFilter(UnhandledExFilter);
#endif

#if !defined(_DEBUG) || defined(_WIN32) || defined(__ANDROID__)
	try {
#endif
		new SpiralOfFate::Game();
		SpiralOfFate::game->logger.info("Starting game->");
		run();
		SpiralOfFate::game->logger.info("Goodbye !");
#if !defined(_DEBUG) || defined(_WIN32) || defined(__ANDROID__)
	} catch (std::exception &e) {
		if (SpiralOfFate::game) {
			SpiralOfFate::game->logger.fatal(e.what());
			SpiralOfFate::Utils::dispMsg("Fatal error", e.what(), MB_ICONERROR, &*SpiralOfFate::game->screen);
		} else
			SpiralOfFate::Utils::dispMsg("Fatal error", e.what(), MB_ICONERROR, nullptr);
		ret = EXIT_FAILURE;
	}
#endif
	delete SpiralOfFate::game;
#ifdef __ANDROID__
	// In android, it is possible to exit the app without killing the process.
	// The main gets called again when the app is restarted so we need to make sure the global is set to null.
	SpiralOfFate::game = nullptr;
#endif
	return ret;
}
