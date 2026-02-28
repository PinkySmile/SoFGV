#include <iostream>
#include <memory>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#include <crtdbg.h>
#include <direct.h>
#endif
#include <sys/stat.h>
#include <LibCore.hpp>
#include "Scenes/Scenes.hpp"
#include "Scenes/Network/SpectatorCharacterSelect.hpp"
#include "Scenes/Network/SpectatorInGame.hpp"

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
#define MessageBox(...) ((void)0)

std::string getLastError(int err = errno)
{
	return strerror(err);
}
#endif

using namespace SpiralOfFate;

void saveInputs(const std::pair<std::shared_ptr<KeyboardInput>, std::shared_ptr<ControllerInput>> &input, const std::string &path)
{
	auto parent = std::filesystem::path(path).parent_path();

	if (!parent.empty())
		std::filesystem::create_directories(parent);

	std::ofstream stream{path};

	input.first->save(stream);
	input.second->save(stream);
}

std::pair<std::shared_ptr<KeyboardInput>, std::shared_ptr<ControllerInput>> loadInputs(const std::string &path)
{
	std::ifstream istream{path};
	std::pair<std::shared_ptr<KeyboardInput>, std::shared_ptr<ControllerInput>> result;

	if (istream.fail()) {
		result.first = std::make_shared<KeyboardInput>();
		result.second = std::make_shared<ControllerInput>();
		saveInputs(result, path);
	} else {
		result.first = std::make_shared<KeyboardInput>(istream);
		result.second = std::make_shared<ControllerInput>(istream);
	}
	return result;
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
	auto magicPtr = reinterpret_cast<unsigned *>(magic);

	// We perform an endianness check here and display a warning if it fails.
	// The affected stuff are:
	//   - All the network stack
	//   - Generated replays
	//   - Computed state checksums
	// We officially support only little endian but people can play if they have the same endianness.
	// Regardless, the game should work in singleplayer.
	if (*magicPtr != 0x01020304)
		Utils::dispMsg(
			game->gui,
			"Warning",
			"Your version of the game has been compiled in " + std::string(*magicPtr == 0x04030201 ? "big endian" : "middle endian") + " but only little endian is supported\n" +
			"You will not be able to play with players using a different endianness.\n" +
			"Moreover, you won't be able to load replays generated with a different endianness.\n"
			"Your replays will also not be compatible with a different version of the game.",
			MB_ICONWARNING
		);

	game->battleRandom.seed(0);
	assert_eq(game->battleRandom(), 2357136044UL);
	assert_eq(game->battleRandom(), 2546248239UL);
	assert_eq(game->battleRandom(), 3071714933UL);
	assert_eq(game->battleRandom(), 3626093760UL);
	assert_eq(game->battleRandom(), 2588848963UL);

	game->battleRandom.seed(0);
	assert_eq(game->battleRandom(), 2357136044UL);
	assert_eq(game->battleRandom(), 2546248239UL);
	assert_eq(game->battleRandom(), 3071714933UL);
	assert_eq(game->battleRandom(), 3626093760UL);
	assert_eq(game->battleRandom(), 2588848963UL);

	game->battleRandom.seed(0);
	assert_eq(game->battleRandom.min(), 0UL);
	assert_eq(game->battleRandom.max(), 0xFFFFFFFFUL);
	assert_eq(random_distrib(game->battleRandom, 0, 20), 10UL);
	assert_eq(random_distrib(game->battleRandom, 0, 20), 11UL);
	assert_eq(random_distrib(game->battleRandom, 0, 20), 14UL);
	assert_eq(random_distrib(game->battleRandom, 0, 20), 16UL);
	assert_eq(random_distrib(game->battleRandom, 0, 20), 12UL);
}

void	registerScenes()
{
	bool hasLoading = true;

#ifdef __EMSCRIPTEN__
	hasLoading = false;
#endif

	game->scene.registerScene("title_screen", TitleScreen::create, false);
	game->scene.registerScene("loading", LoadingScene::create, false);

	// Single player
	game->scene.registerScene("char_select", CharacterSelect::create, hasLoading);
	game->scene.registerScene("in_game", InGame::create, hasLoading);
	game->scene.registerScene("practice_in_game", PracticeInGame::create, hasLoading);
	game->scene.registerScene("replay_in_game", ReplayInGame::create, hasLoading);
#ifdef HAS_NETWORK
#ifdef _DEBUG
	game->scene.registerScene("sync_test_in_game", SyncTestInGame::create, hasLoading);
#endif

	// Netplay
	game->scene.registerScene("client_char_select", ClientCharacterSelect::create, hasLoading);
	game->scene.registerScene("server_char_select", ServerCharacterSelect::create, hasLoading);
	game->scene.registerScene("spectator_char_select", SpectatorCharacterSelect::create, false);
	game->scene.registerScene("client_in_game", ClientInGame::create, hasLoading);
	game->scene.registerScene("server_in_game", ServerInGame::create, hasLoading);
	game->scene.registerScene("spectator_in_game", SpectatorInGame::create, hasLoading);
#endif
}

void	run()
{
	bool step = false;
	bool force = false;
	sf::Image icon;
	double timer = 0;
	sf::Clock clock;

	checkCompilationEnv();
	game->menu = loadInputs("menuInputs.in");
	game->P1 = loadInputs(game->settings.inputPresetP1);
	game->P2 = loadInputs(game->settings.inputPresetP2);
	registerScenes();
#ifdef VIRTUAL_CONTROLLER
	game->virtualController = std::make_shared<VirtualController>();
#endif
	if (icon.loadFromFile("assets/gameIcon.png"))
		game->screen->setIcon(icon.getSize(), icon.getPixelsPtr());
	game->screen->setFont(game->font);
	game->scene.switchScene("title_screen");
	clock.restart();
	game->screen->setFramerateLimit(60);
	game->gui.setWindow(*game->screen);
	while (game->screen->isOpen()) {
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
			if (!step || force)
				game->scene.update();
			force = false;
			timer -= 1. / 60.;
		}
		game->scene.render();
	#ifdef VIRTUAL_CONTROLLER
		game->virtualController->render();
	#endif
	#ifndef NO_TGUI
		game->gui.draw();
	#endif
		game->screen->display();

		while (auto event = game->screen->pollEvent()) {
			if (event->is<EVENT_WINDOW_CLOSED>())
				game->screen->close();
		#ifdef _DEBUG
			if (auto e = event->getIf<sf::Event::KeyPressed>()) {
				if (e->code == sf::Keyboard::Key::F12 && e->control && e->shift)
					step = !step;
			}
			if (auto e = event->getIf<sf::Event::KeyPressed>()) {
				if (e->code == sf::Keyboard::Key::F11 && e->control && e->shift)
					force = true;
			}
		#endif
		#ifdef USE_SFML
			game->scene.consumeEvent(*event);
		#endif
		#ifdef VIRTUAL_CONTROLLER
			game->virtualController->consumeEvent(event);
		#endif
		#ifndef NO_TGUI
			game->gui.handleEvent(*event);
		#endif
		}
	}
	// TODO: Move
	saveInputs(game->menu, "menuInputs.in");
	saveInputs(game->P1, game->settings.inputPresetP1);
	saveInputs(game->P2, game->settings.inputPresetP2);
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
		new Game("Spiral of Fate: Grand Vision | version " VERSION_STR, "assets/fonts/Retro Gaming.ttf", "settings.json");
		game->logger.info("Starting game->");
		run();
		game->logger.info("Goodbye !");
#if !defined(_DEBUG) || defined(_WIN32) || defined(__ANDROID__)
	} catch (std::exception &e) {
		if (game) {
			game->logger.fatal(e.what());
#ifdef _WIN32
			MessageBoxA(game->screen->getNativeHandle(), e.what(), "Fatal error", MB_ICONERROR);
		} else
			MessageBoxA(nullptr, e.what(), "Fatal error", MB_ICONERROR);
#else
		//	Utils::dispMsg("Fatal error", e.what(), MB_ICONERROR, &*game->screen);
		}// else
		//	Utils::dispMsg("Fatal error", e.what(), MB_ICONERROR, nullptr);
#endif
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
