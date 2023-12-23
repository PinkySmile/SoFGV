//
// Created by PinkySmile on 24/09/2021.
//

#ifdef _WIN32
#include <windows.h>
#undef max
#undef min
#else
// TODO: Put native boxes on windows
#define MessageBox(...)
#include <arpa/inet.h>
#include <dirent.h>
#endif
#include <utility>
#include "TitleScreen.hpp"
#include "InGame.hpp"
#include "CharacterSelect.hpp"
#include "Resources/Game.hpp"
#include "Logger.hpp"
#include "KeyboardInput.hpp"
#include "ControllerInput.hpp"
#include "Utils.hpp"
#include "Inputs/ReplayInput.hpp"
#include "Resources/version.h"
#include "ReplayInGame.hpp"
#ifdef HAS_NETWORK
#include "Resources/Network/ServerConnection.hpp"
#include "Resources/Network/ClientConnection.hpp"
#endif
#ifdef VIRTUAL_CONTROLLER
#include "VirtualController.hpp"
#endif

#define THRESHOLD 50

enum TitleScreenButton {
	#define PLAY_BUTTON PLAY_BUTTON
	PLAY_BUTTON,
	#define PRACTICE_BUTTON PRACTICE_BUTTON
	PRACTICE_BUTTON,
	#define REPLAY_BUTTON REPLAY_BUTTON
	REPLAY_BUTTON,
#ifdef HAS_NETWORK
	#define HOST_BUTTON HOST_BUTTON
	HOST_BUTTON,
	#define CONNECT_BUTTON CONNECT_BUTTON
	CONNECT_BUTTON,
#endif
	#define SETTINGS_BUTTON SETTINGS_BUTTON
	SETTINGS_BUTTON,
	#define QUIT_BUTTON QUIT_BUTTON
	QUIT_BUTTON,
#if defined(_DEBUG) && defined(HAS_NETWORK)
	SYNC_TEST_BUTTON,
	#define SYNC_TEST_BUTTON SYNC_TEST_BUTTON
#endif
};

#define STICK_ID_KEYBOARD 0
#define STICK_ID_VPAD 1
#define STICK_ID_PPAD1 2

extern std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> loadPlayerInputs(std::ifstream &stream);

namespace SpiralOfFate
{
	static const unsigned inputs[]{
		INPUT_LEFT,
		INPUT_RIGHT,
		INPUT_UP,
		INPUT_DOWN,
		INPUT_NEUTRAL,
		INPUT_SPIRIT,
		INPUT_PAUSE
	};

	TitleScreen::TitleScreen() :
		_menuObject{{
			{"vsplayer", "Play a game against a human opponent", [this]{
				this->_askingInputs = true;
			}},
			{"practicemode", "Free training", [this]{
				this->_askingInputs = true;
			}},
			{"replays", "Select a replay to watch", [this]{
				auto path = Utils::openFileDialog("Open replay", "./replays", {{".+[.]replay", "Replay file"}});

				if (!path.empty()) {
					try {
						this->_loadReplay(path);
					} catch (std::exception &e) {
						Utils::dispMsg("Replay loading failed", "This replay is invalid, corrupted or was created for an different version of the game: " + std::string(e.what()), MB_ICONERROR);
					}
				}
			}},
		#ifdef HAS_NETWORK
			{"host", "Host an online game", [this]{
				this->_askingInputs = true;
			}},
			{"connect", "Connect to ip from clipboard", [this]{
				this->_askingInputs = true;
			}},
		#endif
			{"settings", "Change inputs", [this]{
				this->_changingInputs = 1;
				this->_cursorInputs = 0;
			}},
			{"quit", "Quit game", []{
				game->screen->close();
			}},
		#ifdef SYNC_TEST_BUTTON
			{"synctest", "Verify that rollback doesn't desync", [this]{
				this->_askingInputs = true;
			}},
		#endif
		}}
	{
		game->logger.info("Title scene created");
		this->_titleBg.textureHandle = game->textureMgr.load("assets/ui/titlebackground.png");
		this->_titleLogo.textureHandle = game->textureMgr.load("assets/ui/title.png");
		this->_titleLogo.setPosition({275, 31});
		this->_titleSpiral.textureHandle = game->textureMgr.load("assets/ui/spiral.png");
		this->_titleSpiral.setOrigin({140, 131});
		this->_titleSpiral.setPosition({328, 139});
		this->_netbellSound = game->soundMgr.load("assets/sfxs/se/057.ogg");
		this->_inputs.resize(INPUT_NUMBER);
		this->_inputs[INPUT_LEFT].textureHandle = game->textureMgr.load("assets/icons/inputs/4.png");
		this->_inputs[INPUT_RIGHT].textureHandle = game->textureMgr.load("assets/icons/inputs/6.png");
		this->_inputs[INPUT_UP].textureHandle = game->textureMgr.load("assets/icons/inputs/8.png");
		this->_inputs[INPUT_DOWN].textureHandle = game->textureMgr.load("assets/icons/inputs/2.png");
		this->_inputs[INPUT_N].textureHandle = game->textureMgr.load("assets/icons/inputs/neutral.png");
		this->_inputs[INPUT_M].textureHandle = game->textureMgr.load("assets/icons/inputs/matter.png");
		this->_inputs[INPUT_S].textureHandle = game->textureMgr.load("assets/icons/inputs/spirit.png");
		this->_inputs[INPUT_V].textureHandle = game->textureMgr.load("assets/icons/inputs/void.png");
		this->_inputs[INPUT_A].textureHandle = game->textureMgr.load("assets/icons/inputs/ascend.png");
		this->_inputs[INPUT_D].textureHandle = game->textureMgr.load("assets/icons/inputs/dash.png");
		this->_inputs[INPUT_PAUSE].textureHandle = game->textureMgr.load("assets/icons/inputs/pause.png");
		this->_menuObject.displayed = true;
		this->_lastInput = &*game->menu.first;
	}

	TitleScreen::~TitleScreen()
	{
		if (this->onDestruct)
			this->onDestruct();
		game->logger.debug("~TitleScreen");
		game->soundMgr.remove(this->_netbellSound);
		game->textureMgr.remove(this->_titleBg.textureHandle);
		game->textureMgr.remove(this->_titleLogo.textureHandle);
		game->textureMgr.remove(this->_titleSpiral.textureHandle);
		game->textureMgr.remove(this->_inputs[INPUT_LEFT].textureHandle);
		game->textureMgr.remove(this->_inputs[INPUT_RIGHT].textureHandle);
		game->textureMgr.remove(this->_inputs[INPUT_UP].textureHandle);
		game->textureMgr.remove(this->_inputs[INPUT_DOWN].textureHandle);
		game->textureMgr.remove(this->_inputs[INPUT_N].textureHandle);
		game->textureMgr.remove(this->_inputs[INPUT_M].textureHandle);
		game->textureMgr.remove(this->_inputs[INPUT_S].textureHandle);
		game->textureMgr.remove(this->_inputs[INPUT_V].textureHandle);
		game->textureMgr.remove(this->_inputs[INPUT_A].textureHandle);
		game->textureMgr.remove(this->_inputs[INPUT_D].textureHandle);
		game->textureMgr.remove(this->_inputs[INPUT_PAUSE].textureHandle);
		if (this->_thread.joinable())
			this->_thread.join();
	}

	void TitleScreen::render() const
	{
		ViewPort view{{0, 0, 1680, 960}};

		game->screen->setView(view);
		game->textureMgr.render(this->_titleBg);
		game->textureMgr.render(this->_titleSpiral);
		game->textureMgr.render(this->_titleLogo);
		this->_menuObject.render();
		if (!this->_errorMsg.empty()) {
			game->screen->fillColor(sf::Color::White);
			game->screen->displayElement({540, 280, 600, 100}, sf::Color{0x50, 0x50, 0x50});
			game->screen->displayElement(this->_errorMsg, {540, 300}, 600, Screen::ALIGN_CENTER);
		}
	#ifdef HAS_NETWORK
		else if (this->_connecting)
			this->_showConnectMessage();
		else if (game->connection)
			this->_showHostMessage();
		else if (this->_chooseSpecCount)
			this->_showChooseSpecCount();
	#endif
		else if (this->_askingInputs)
			this->_showAskInputBox();
		if (this->_changingInputs)
			this->_showEditKeysMenu();
	}

	void TitleScreen::update()
	{
		this->_titleSpiral.setRotation(std::fmod(this->_titleSpiral.getRotation() + 0.25, 360));
		game->random();
		this->_oldRemote = this->_remote;
		game->menu.first->update();
		game->menu.second->update();

		auto inputs = this->_lastInput->getInputs();

	#ifdef HAS_NETWORK
		if (game->connection && game->connection->isTerminated()) {
			if (this->_connecting) {
				this->_errorMsg = "Failed to connect";
				this->_connecting = false;
			}
			game->connection.reset();
		}
	#endif
		if (!this->_errorMsg.empty())  {
			if (this->_errorTimer == 0)
				game->soundMgr.play(BASICSOUND_MENU_CANCEL);
			this->_errorTimer++;
			if (this->_errorTimer > 180) {
				this->_errorTimer = 0;
				this->_errorMsg.clear();
			}
		} else {
			if (inputs.verticalAxis == -1 || (inputs.verticalAxis < -36 && inputs.verticalAxis % 6 == 0))
				this->_onGoDown();
			else if (inputs.verticalAxis == 1 || (inputs.verticalAxis > 36 && inputs.verticalAxis % 6 == 0))
				this->_onGoUp();
			if (inputs.horizontalAxis == -1 || (inputs.horizontalAxis < -36 && inputs.horizontalAxis % 6 == 0))
				this->_onGoLeft();
			else if (inputs.horizontalAxis == 1 || (inputs.horizontalAxis > 36 && inputs.horizontalAxis % 6 == 0))
				this->_onGoRight();
		}
		if (inputs.s == 1)
			this->_onCancel();
		if (inputs.n == 1)
			this->_onConfirm(this->_latestJoystickId);
		if (
		#ifdef HAS_NETWORK
			game->connection ||
			this->_chooseSpecCount ||
		#endif
			this->_changingInputs ||
			this->_askingInputs
		)
			this->_menuObject.update({});
		else
			this->_menuObject.update(inputs);
	}

	void TitleScreen::consumeEvent(const sf::Event &event)
	{
		switch (event.type) {
		case sf::Event::KeyPressed:
			if (this->_onKeyPressed(event.key))
				return;
			break;
		case sf::Event::JoystickButtonPressed:
			if (this->_onJoystickPressed(event.joystickButton))
				return;
			break;
		case sf::Event::JoystickMoved:
			if (this->_onJoystickMoved(event.joystickMove))
				return;
			break;
#ifdef VIRTUAL_CONTROLLER
		case sf::Event::MouseMoved:
		case sf::Event::MouseButtonPressed:
		case sf::Event::TouchBegan:
		case sf::Event::TouchMoved:
			if (this->_changingInputs)
				break;
			this->_latestJoystickId = STICK_ID_VPAD;
			this->_lastInput = &*game->virtualController;
			break;
#endif
		default:
			break;
		}
		game->menu.first->consumeEvent(event);
		game->menu.second->consumeEvent(event);
	}

#ifdef HAS_NETWORK
	void TitleScreen::_host(bool spec)
	{
		game->activeNetInput = this->_leftInput == 1 ? static_cast<std::shared_ptr<IInput>>(game->P1.first) : static_cast<std::shared_ptr<IInput>>(game->P1.second);

		// TODO: Handle names
		auto con = new ServerConnection("SpiralOfFate::ServerConnection");
		std::ifstream stream{"hostPort.txt"};

		if (stream) {
			stream >> this->_hostingPort;
		}
		game->connection.reset(con);
		con->onConnection = [this](Connection::Remote &remote, PacketInitRequest &packet){
			std::string name{packet.playerName, strnlen(packet.playerName, sizeof(packet.playerName))};
			std::string vers{packet.gameVersion, strnlen(packet.gameVersion, sizeof(packet.gameVersion))};

			game->logger.info(name + " connected with game version " + vers);
			this->_onConnect(remote.ip.toString());
		};
		con->onError = [](Connection::Remote &remote, const PacketError &e){
			game->logger.error(remote.ip.toString() + " -> " + e.toString());
		};
		con->onDisconnect = [this](Connection::Remote &remote){
			this->_onDisconnect(remote.ip.toString());
		};
		con->spectatorEnabled = spec;
		con->host(this->_hostingPort);
		this->onDestruct = [con]{
			con->onConnection = nullptr;
			con->onDisconnect = nullptr;
		};
	}

	void TitleScreen::_connect()
	{
		game->activeNetInput = this->_leftInput == 1 ? static_cast<std::shared_ptr<IInput>>(game->P1.first) : static_cast<std::shared_ptr<IInput>>(game->P1.second);

		// TODO: Handle names
		auto con = new ClientConnection("SpiralOfFate::ClientConnection");
		auto ipString = sf::Clipboard::getString();

		if (ipString.isEmpty()) {
			Utils::dispMsg("Error", "No ip is copied to the clipboard", MB_ICONERROR, &*game->screen);
			delete con;
			return;
		}

		size_t pos = ipString.find(':');
		sf::IpAddress ip = static_cast<std::string>(ipString.substring(0, pos));
		unsigned short port = 10800;

		if (ip == sf::IpAddress()) {
			Utils::dispMsg("Error", "Clipboard doesn't contain a valid IP address", MB_ICONERROR, &*game->screen);
			delete con;
			return;
		}
		if (pos != std::string::npos) {
			try {
				auto p = std::stoul(static_cast<std::string>(ipString.substring(pos + 1)));

				if (p > UINT16_MAX)
					throw std::exception();
				port = p;
			} catch (...) {
				Utils::dispMsg("Error", "Clipboard doesn't contain a valid IP address", MB_ICONERROR, &*game->screen);
				delete con;
				return;
			}
		}
		game->connection.reset(con);
		game->lastIp = ip.toString();
		game->lastPort = port;
		con->onConnection = [this](Connection::Remote &remote, PacketInitSuccess &packet){
			std::string name{packet.player1Name, strnlen(packet.player1Name, sizeof(packet.player1Name))};
			std::string vers{packet.gameVersion, strnlen(packet.gameVersion, sizeof(packet.gameVersion))};

			game->logger.info("Connected to " + name + " with game version " + vers);
			this->_onConnect(remote.ip.toString());
		};
		con->onError = [](Connection::Remote &remote, const PacketError &e){
			game->logger.error(remote.ip.toString() + " -> " + e.toString());
			// TODO: Abort connection and display error on UI
		};
		con->onDisconnect = [this](Connection::Remote &remote){
			this->_onDisconnect(remote.ip.toString());
		};
		con->connect(ip, port);
		this->_connecting = true;
		this->onDestruct = [con]{
			con->onConnection = nullptr;
			con->onDisconnect = nullptr;
		};
	}
#endif

	void TitleScreen::_onInputsChosen()
	{
		CharacterSelect::Arguments *args;

		switch (this->_menuObject.getSelectedItem()) {
		case PLAY_BUTTON:
			args = new CharacterSelect::Arguments();
			args->leftInput = _getInputFromId(this->_leftInput - 1, game->P1);
			args->rightInput = _getInputFromId(this->_rightInput - 1, game->P2);
			args->inGameName = "in_game";
			game->scene.switchScene("char_select", args);
			break;
		case PRACTICE_BUTTON:
			args = new CharacterSelect::Arguments();
			args->leftInput = _getInputFromId(this->_leftInput - 1, game->P1);
			args->rightInput = _getInputFromId(this->_rightInput - 1, game->P2);
			args->inGameName = "practice_in_game";
			game->scene.switchScene("char_select", args);
			break;
	#ifdef HAS_NETWORK
		case HOST_BUTTON:
			this->_chooseSpecCount = true;
			break;
		case CONNECT_BUTTON:
			this->_connect();
			break;
	#ifdef _DEBUG
		case SYNC_TEST_BUTTON:
			args = new CharacterSelect::Arguments();
			args->leftInput = _getInputFromId(this->_leftInput - 1, game->P1);
			args->rightInput = _getInputFromId(this->_rightInput - 1, game->P2);
			args->inGameName = "sync_test_in_game";
			game->scene.switchScene("char_select", args);
			break;
	#endif
	#endif
		}
	}

	bool TitleScreen::_onKeyPressed(sf::Event::KeyEvent ev)
	{
		if (ev.code == sf::Keyboard::F2 && this->_changingInputs > 1) {
			auto path = Utils::saveFileDialog("Save inputs", "./profiles", {{".*\\.in", "Input files"}});

			if (path.empty())
				return false;

			std::ofstream stream{path};

			if (!stream)
				Utils::dispMsg("Saving error", strerror(errno), MB_ICONERROR, &*game->screen);

			auto &pair = (this->_changingInputs == 2 ? game->P1 : game->P2);

			pair.first->save(stream);
			pair.second->save(stream);
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			return false;
		}
		if (ev.code == sf::Keyboard::F3 && this->_changingInputs > 1) {
			auto path = Utils::openFileDialog("Load inputs", "./profiles", {{".*\\.in", "Input files"}});

			if (path.empty())
				return false;

			std::ifstream stream{path};

			if (!stream)
				Utils::dispMsg("Loading error", strerror(errno), MB_ICONERROR, &*game->screen);

			auto &pair = (this->_changingInputs == 2 ? game->P1 : game->P2);

			pair = loadPlayerInputs(stream);
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			return false;
		}
		if (this->_changeInput && this->_changingInputs) {
			if (sf::Keyboard::Escape == ev.code) {
				game->soundMgr.play(BASICSOUND_MENU_CANCEL);
				this->_changeInput = false;
				return false;
			}
			if (this->_latestJoystickId != STICK_ID_KEYBOARD)
				return false;

			auto &pair = this->_changingInputs == 1 ? game->menu : (this->_changingInputs == 2 ? game->P1 : game->P2);

			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			pair.first->changeInput(static_cast<InputEnum>(this->_cursorInputs), ev.code);
			this->_changeInput = false;
			return true;
		}

		this->_latestJoystickId = STICK_ID_KEYBOARD;
		this->_lastInput = &*game->menu.first;
		switch (ev.code) {
		case sf::Keyboard::Escape:
			this->_onCancel();
			break;
		default:
			break;
		}
		return false;
	}

	bool TitleScreen::_onJoystickMoved(sf::Event::JoystickMoveEvent ev)
	{
		this->_oldStickValues[ev.joystickId][ev.axis] = ev.position;
		if (this->_changeInput && this->_changingInputs) {
			if (this->_latestJoystickId < STICK_ID_PPAD1)
				return false;
			if (std::abs(ev.position) < THRESHOLD)
				return true;

			auto &pair = this->_changingInputs == 1 ? game->menu : (this->_changingInputs == 2 ? game->P1 : game->P2);

			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			pair.second->changeInput(static_cast<InputEnum>(this->_cursorInputs), new ControllerAxis(ev.joystickId, ev.axis, std::copysign(30, ev.position)));
			this->_changeInput = false;
			return true;
		}
		this->_lastInput = &*game->menu.second;
		this->_latestJoystickId = ev.joystickId + STICK_ID_PPAD1;
		return false;
	}

	bool TitleScreen::_onJoystickPressed(sf::Event::JoystickButtonEvent ev)
	{
		if (this->_changeInput && this->_changingInputs) {
			if (this->_latestJoystickId < STICK_ID_PPAD1)
				return false;

			auto &pair = this->_changingInputs == 1 ? game->menu : (this->_changingInputs == 2 ? game->P1 : game->P2);

			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			pair.second->changeInput(static_cast<InputEnum>(this->_cursorInputs), new ControllerButton(ev.joystickId, ev.button));
			this->_changeInput = false;
			return true;
		}
		this->_lastInput = &*game->menu.second;
		this->_latestJoystickId = ev.joystickId + STICK_ID_PPAD1;
		return false;
	}

	void TitleScreen::_showAskInputBox() const
	{
		game->screen->displayElement({540, 180, 600, 300}, sf::Color{0x50, 0x50, 0x50});

		game->screen->fillColor(this->_leftInput ? sf::Color::Green : sf::Color::White);
		game->screen->displayElement("P1", {540 + 120, 190});
		game->screen->fillColor(sf::Color::White);
		if (this->_leftInput)
			game->screen->displayElement(
			#ifdef VIRTUAL_CONTROLLER
				this->_leftInput - 1 == STICK_ID_VPAD ?
				game->virtualController->getName() :
			#endif
				this->_leftInput - 1 == STICK_ID_KEYBOARD ?
				game->P1.first->getName() :
				game->P1.second->getName() + " #" + std::to_string(this->_leftInput - STICK_ID_PPAD1),
				{540, 260},
				300,
				Screen::ALIGN_CENTER
			);
		else
			game->screen->displayElement("Press [Confirm]", {540, 260}, 300, Screen::ALIGN_CENTER);

		if (
			this->_menuObject.getSelectedItem() == PLAY_BUTTON ||
		#ifdef SYNC_TEST_BUTTON
			this->_menuObject.getSelectedItem() == SYNC_TEST_BUTTON ||
		#endif
			this->_menuObject.getSelectedItem() == PRACTICE_BUTTON
		)
			game->screen->fillColor(this->_rightInput ? sf::Color::Green : (this->_leftInput ? sf::Color::White : sf::Color{0xA0, 0xA0, 0xA0}));
		else
			game->screen->fillColor(sf::Color{0x80, 0x80, 0x80});
		game->screen->displayElement("P2", {540 + 420, 190});
		game->screen->fillColor(sf::Color::White);
		if (this->_leftInput && (
			this->_menuObject.getSelectedItem() == PLAY_BUTTON ||
		#ifdef SYNC_TEST_BUTTON
			this->_menuObject.getSelectedItem() == SYNC_TEST_BUTTON ||
		#endif
			this->_menuObject.getSelectedItem() == PRACTICE_BUTTON
		)) {
			if (this->_rightInput)
				game->screen->displayElement(
				#ifdef VIRTUAL_CONTROLLER
					this->_rightInput - 1 == STICK_ID_VPAD ?
					game->virtualController->getName() :
				#endif
					this->_rightInput - 1 == STICK_ID_KEYBOARD ?
					game->P2.first->getName() :
					game->P2.second->getName() + " #" + std::to_string(this->_rightInput - STICK_ID_PPAD1),
					{840, 260},
					300,
					Screen::ALIGN_CENTER
				);
			else
				game->screen->displayElement("Press [Confirm]", {840, 260}, 300, Screen::ALIGN_CENTER);
		}

		if (this->_leftInput && (this->_rightInput || (
			this->_menuObject.getSelectedItem() != PLAY_BUTTON &&
		#ifdef SYNC_TEST_BUTTON
			this->_menuObject.getSelectedItem() != SYNC_TEST_BUTTON &&
		#endif
			this->_menuObject.getSelectedItem() != PRACTICE_BUTTON
		)))
			game->screen->displayElement("Press [Confirm] to confirm", {540, 360}, 600, Screen::ALIGN_CENTER);
	}

#ifdef HAS_NETWORK
	void TitleScreen::_showHostMessage() const
	{
		game->screen->fillColor(sf::Color::White);
		if (this->_remote.empty()) {
			game->screen->displayElement({640, 280, 400, 100}, sf::Color{0x50, 0x50, 0x50});
			game->screen->displayElement("Hosting on port " + std::to_string(this->_hostingPort), {640, 300}, 400, Screen::ALIGN_CENTER);
		} else {
			game->screen->displayElement({620, 280, 440, 200}, sf::Color{0x50, 0x50, 0x50});
			game->screen->displayElement(this->_remote + " joined.", {640, 300}, 400, Screen::ALIGN_CENTER);
			if (this->_spec.first == this->_spec.second)
				game->screen->displayElement("Select delay   < " + std::to_string(this->_delay) + " frame(s) >", {640, 340}, 400, Screen::ALIGN_CENTER);
			else
				game->screen->displayElement("Waiting for spectator(s) (" + std::to_string(this->_spec.first) + "/" + std::to_string(this->_spec.second) + ").", {640, 340}, 400, Screen::ALIGN_CENTER);
			if (this->_nbPings) {
				game->screen->textSize(20);
				game->screen->displayElement("Last ping: " + std::to_string(this->_lastPing) + "ms", {620, 380}, 440, Screen::ALIGN_CENTER);
				game->screen->displayElement("Peak ping: " + std::to_string(this->_peakPing) + "ms", {620, 400}, 440, Screen::ALIGN_CENTER);
				game->screen->displayElement("Average ping: " + std::to_string(static_cast<int>(std::round(this->_totalPing / this->_nbPings))) + "ms", {620, 420}, 440, Screen::ALIGN_CENTER);
				game->screen->textSize(30);
			} else
				game->screen->displayElement("Calculating ping...", {620, 380}, 440, Screen::ALIGN_CENTER);
		}
	}

	void TitleScreen::_showConnectMessage() const
	{
		game->screen->fillColor(sf::Color::White);
		if (this->_remote.empty()) {
			game->screen->displayElement({540, 280, 600, 100}, sf::Color{0x50, 0x50, 0x50});
			game->screen->displayElement("Connecting to " + game->lastIp + " on port " + std::to_string(game->lastPort), {540, 300}, 600, Screen::ALIGN_CENTER);
		} else {
			game->screen->displayElement({540, 280, 600, 130}, sf::Color{0x50, 0x50, 0x50});
			game->screen->displayElement("Connected to " + this->_remote + ".", {540, 300}, 600, Screen::ALIGN_CENTER);
			if (this->_spec.first == this->_spec.second)
				game->screen->displayElement("Waiting for host to select delay.", {540, 330}, 600, Screen::ALIGN_CENTER);
			else
				game->screen->displayElement("Waiting for spectator(s) (" + std::to_string(this->_spec.first) + "/" + std::to_string(this->_spec.second) + ").", {540, 330}, 600, Screen::ALIGN_CENTER);
		}
	}
#endif

	void TitleScreen::_showEditKeysMenu() const
	{
		auto &pair = this->_changingInputs == 1 ? game->menu : (this->_changingInputs == 2 ? game->P1 : game->P2);
		auto input = !this->_latestJoystickId ? static_cast<std::shared_ptr<IInput>>(pair.first) : static_cast<std::shared_ptr<IInput>>(pair.second);
		auto strs = input->getKeyNames();
		const std::string names[]{
			"Left",
			"Right",
			"Up",
			"Down",
			"Confirm",
			"Cancel",
			"Pause"
		};

		game->screen->displayElement({640, 80, 400, 830}, sf::Color{0x50, 0x50, 0x50});

		game->screen->fillColor(sf::Color::White);
		if (this->_changingInputs == 1) {
			game->screen->displayElement("Menu | " + input->getName(), {640, 85}, 400, Screen::ALIGN_CENTER);
			game->screen->fillColor(sf::Color::White);
			for (size_t j = 0; j < sizeof(inputs) / sizeof(*inputs); j++) {
				auto i = inputs[j];

				if (this->_changeInput && this->_cursorInputs == i) {
					game->screen->fillColor(sf::Color{0xFF, 0x80, 0x00});
					game->screen->displayElement(names[j] + ": Press a key", {680, 146 + i * 68.f});
				} else {
					game->screen->fillColor(
						this->_cursorInputs == i ? sf::Color::Red : sf::Color::White);
					game->screen->displayElement(names[j] + ": " + strs[i], {680, 146 + i * 68.f});
				}
			}
		} else {
			game->screen->displayElement((this->_changingInputs == 2 ? "P1 | " : "P2 | ") + input->getName(), {640, 85}, 400, Screen::ALIGN_CENTER);
			game->screen->fillColor(sf::Color::White);
			for (unsigned i = 0; i < this->_inputs.size(); i++) {
				this->_inputs[i].setPosition({680, 135 + i * 68.f});
				game->textureMgr.render(this->_inputs[i]);
				if (this->_changeInput && this->_cursorInputs == i) {
					game->screen->fillColor(sf::Color{0xFF, 0x80, 0x00});
					game->screen->displayElement("Press a key", {760, 146 + i * 68.f});
				} else {
					game->screen->fillColor(this->_cursorInputs == i ? sf::Color::Red : sf::Color::White);
					game->screen->displayElement(strs[i], {760, 146 + i * 68.f});
				}
			}
		}
	}

	void TitleScreen::_onGoUp()
	{
	#ifdef HAS_NETWORK
		if (game->connection)
			return;
		if (this->_chooseSpecCount)
			return;
	#endif
		if (this->_changingInputs) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			do {
				this->_cursorInputs += this->_inputs.size();
				this->_cursorInputs--;
				this->_cursorInputs %= this->_inputs.size();
			} while (std::find(inputs, inputs + 7, this->_cursorInputs) == inputs + 7 && this->_changingInputs == 1);
			return;
		}
		if (this->_askingInputs)
			return;
		game->soundMgr.play(BASICSOUND_MENU_MOVE);
		this->_menuObject.setSelectedItem(this->_menuObject.getSelectedItem() - 1);
	}

	void TitleScreen::_onGoDown()
	{
	#ifdef HAS_NETWORK
		if (game->connection)
			return;
	#endif
		if (this->_changingInputs) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			do {
				this->_cursorInputs++;
				this->_cursorInputs %= this->_inputs.size();
			} while (std::find(inputs, inputs + 7, this->_cursorInputs) == inputs + 7 && this->_changingInputs == 1);
			return;
		}
	#ifdef HAS_NETWORK
		if (this->_chooseSpecCount)
			return;
	#endif
		if (this->_askingInputs)
			return;
		game->soundMgr.play(BASICSOUND_MENU_MOVE);
		this->_menuObject.setSelectedItem(this->_menuObject.getSelectedItem() + 1);
	}

	void TitleScreen::_onGoLeft()
	{
	#ifdef HAS_NETWORK
		if (game->connection)
			return;
		if (this->_chooseSpecCount) {
			this->_specEnabled = !this->_specEnabled;
			return;
		}
	#endif
		if (this->_changingInputs) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			this->_changingInputs--;
			this->_changingInputs += (this->_changingInputs == 0) * 3;
			while (std::find(inputs, inputs + 7, this->_cursorInputs) == inputs + 7 && this->_changingInputs == 1) {
				this->_cursorInputs += this->_inputs.size();
				this->_cursorInputs--;
				this->_cursorInputs %= this->_inputs.size();
			}
			return;
		}
	}

	void TitleScreen::_onGoRight()
	{
	#ifdef HAS_NETWORK
		if (game->connection)
			return;
		if (this->_chooseSpecCount) {
			this->_specEnabled = !this->_specEnabled;
			return;
		}
	#endif
		if (this->_changingInputs) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			this->_changingInputs = (this->_changingInputs + 1) % 4;
			this->_changingInputs += this->_changingInputs == 0;
			while (std::find(inputs, inputs + 7, this->_cursorInputs) == inputs + 7 && this->_changingInputs == 1) {
				this->_cursorInputs += this->_inputs.size();
				this->_cursorInputs--;
				this->_cursorInputs %= this->_inputs.size();
			}
			return;
		}
	}

	void TitleScreen::_onConfirm(unsigned stickId)
	{
	#ifdef HAS_NETWORK
		if (game->connection)
			return;
		if (this->_chooseSpecCount) {
			this->_chooseSpecCount = false;
			this->_host(this->_specEnabled);
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			return;
		}
	#endif
		if (this->_changingInputs) {
			this->_changeInput = true;
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			return;
		}
		if (this->_askingInputs) {
			if (this->_rightInput || (this->_leftInput && (
				this->_menuObject.getSelectedItem() != PLAY_BUTTON &&
			#ifdef SYNC_TEST_BUTTON
				this->_menuObject.getSelectedItem() != SYNC_TEST_BUTTON &&
			#endif
				this->_menuObject.getSelectedItem() != PRACTICE_BUTTON
			)))
				this->_onInputsChosen();
			else if (this->_leftInput) {
				if (stickId >= STICK_ID_PPAD1 && this->_leftInput == stickId + 1)
					return;
				this->_rightInput = stickId + 1;
			} else
				this->_leftInput = stickId + 1;
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			return;
		}
	}

	void TitleScreen::_onCancel()
	{
		game->soundMgr.play(BASICSOUND_MENU_CANCEL);
	#ifdef HAS_NETWORK
		if (game->connection) {
			this->_connecting = false;
			return game->connection.reset();
		}
		if (this->_chooseSpecCount) {
			this->_chooseSpecCount = false;
			return;
		}
	#endif
		if (this->_changingInputs) {
			this->_changingInputs = 0;
			return;
		}
		if (this->_askingInputs) {
			if (this->_rightInput && (
				this->_menuObject.getSelectedItem() == PLAY_BUTTON ||
			#ifdef SYNC_TEST_BUTTON
				this->_menuObject.getSelectedItem() == SYNC_TEST_BUTTON ||
			#endif
				this->_menuObject.getSelectedItem() == PRACTICE_BUTTON
			))
				this->_rightInput = 0;
			else if (this->_leftInput)
				this->_leftInput = 0;
			else
				this->_askingInputs = false;
			return;
		}
		this->_menuObject.setSelectedItem(QUIT_BUTTON);
	}

#ifdef HAS_NETWORK
	void TitleScreen::_onDisconnect(const std::string &address)
	{
		game->logger.info(address + " disconnected");
		if (this->_remote == address) {
			this->_connected = false;
			this->_remote.clear();
			game->connection.reset();
		}
	}

	void TitleScreen::_onConnect(const std::string &address)
	{
		game->logger.info(address + " connected");
		if (this->_remote.empty()) {
			this->_connected = true;
			game->soundMgr.play(this->_netbellSound);
			this->_remote = address;
		}
	}

	void TitleScreen::_pingUpdate(unsigned int ping)
	{
		this->_totalPing += ping;
		this->_nbPings++;
		this->_peakPing = std::max(ping, this->_peakPing);
		this->_lastPing = ping;
	}

	void TitleScreen::_specUpdate(std::pair<unsigned, unsigned> spec)
	{
		this->_spec = spec;
	}

	void TitleScreen::_showChooseSpecCount() const
	{
		game->screen->displayElement({620, 280, 440, 100}, sf::Color{0x50, 0x50, 0x50});
		game->screen->fillColor(sf::Color::White);
		game->screen->displayElement("Enable spectating?", {640, 280}, 400, Screen::ALIGN_CENTER);
		game->screen->displayElement(this->_specEnabled ? "Spectating enabled" : "Spectating disabled", {640, 340}, 400, Screen::ALIGN_CENTER);
	}
#endif

	void TitleScreen::_loadReplay(const std::filesystem::path &path)
	{
		std::vector<StageEntry> stages;
		std::vector<CharacterEntry> entries;
		std::ifstream stream{path, std::ifstream::binary};
		auto data = game->fileMgr.readFull("assets/stages/list.json");
		nlohmann::json json;
		unsigned nb;
		unsigned short P1pos;
		unsigned short P2pos;
		unsigned short P1palette;
		unsigned short P2palette;
		std::deque<ReplayData> P1inputs;
		std::deque<ReplayData> P2inputs;
		char *buffer;
		ReplayData *buffer2;
		InGame::GameParams params;
		unsigned magic;
		unsigned frameCount;
		unsigned expectedMagic = getMagic();
		RandomWrapper::SerializedWrapper random;

		if (!stream)
			throw std::invalid_argument("Cannot load " + path.string() + ": " + strerror(errno));
		game->logger.info("Loading replay " + path.string());
		stream.read(reinterpret_cast<char *>(&magic), sizeof(magic));
		game->logger.debug("Expected magic " + std::to_string(expectedMagic) + " vs Replay magic " + std::to_string(magic));
		if (magic != expectedMagic)
			throw std::invalid_argument("INVALID_MAGIC");

		auto chrList = game->getCharacters();

		entries.reserve(chrList.size());
		for (auto &entry : chrList) {
			auto file = entry + "/chr.json";

			game->logger.debug("Loading character from " + file);

			auto data2 = game->fileMgr.readFull(file);

			json = nlohmann::json::parse(data2);
			entries.emplace_back(json, entry);
		}
		std::sort(entries.begin(), entries.end(), [](CharacterEntry &a, CharacterEntry &b){
			return a.pos < b.pos;
		});

		json = nlohmann::json::parse(data);
		for (auto &elem : json)
			stages.emplace_back(elem);
		stream.read(reinterpret_cast<char *>(&frameCount), sizeof(frameCount));
		stream.read(reinterpret_cast<char *>(&random), sizeof(random));
		stream.read(reinterpret_cast<char *>(&params), sizeof(params));
		game->logger.debug("Params: stageID " + std::to_string(params.stage) + ", platformsID " + std::to_string(params.platforms) + ", musicID " + std::to_string(params.music));
		if (params.stage >= stages.size())
			throw std::invalid_argument("INVALID_STAGE");
		if (params.platforms >= stages[params.stage].platforms.size())
			throw std::invalid_argument("INVALID_PLAT_CONF");
		game->battleRandom.seed(random.seed);
		game->battleRandom.discard(random.invoke_count);


		stream.read(reinterpret_cast<char *>(&P1pos), sizeof(P1pos));
		stream.read(reinterpret_cast<char *>(&P1palette), sizeof(P1palette));
		game->logger.debug("Reading P1 entry: pos " + std::to_string(P1pos) + ", pal " + std::to_string(P1palette));
		if (P1pos >= entries.size())
			throw std::invalid_argument("INVALID_P1POS");
		if (P1palette >= entries[P1pos].palettes.size())
			throw std::invalid_argument("INVALID_P1PAL");

		stream.read(reinterpret_cast<char *>(&nb), sizeof(nb));
		game->logger.debug("P1 has " + std::to_string(nb) + "inputs");
		buffer = new char[nb * sizeof(ReplayData)];
		stream.read(buffer, nb * sizeof(ReplayData));
		buffer2 = reinterpret_cast<ReplayData *>(buffer);
		P1inputs.insert(P1inputs.begin(), buffer2, buffer2 + nb);
		delete[] buffer;


		stream.read(reinterpret_cast<char *>(&P2pos), sizeof(P2pos));
		stream.read(reinterpret_cast<char *>(&P2palette), sizeof(P2palette));
		game->logger.debug("Reading P2 entry: pos " + std::to_string(P2pos) + ", pal " + std::to_string(P2palette));
		if (P2pos >= entries.size())
			throw std::invalid_argument("INVALID_P2POS");
		if (P2palette >= entries[P2pos].palettes.size())
			throw std::invalid_argument("INVALID_P2PAL");

		stream.read(reinterpret_cast<char *>(&nb), sizeof(nb));
		game->logger.debug("P2 has " + std::to_string(nb) + "inputs");
		buffer = new char[nb * sizeof(ReplayData)];
		stream.read(buffer, nb * sizeof(ReplayData));
		buffer2 = reinterpret_cast<ReplayData *>(buffer);
		P2inputs.insert(P2inputs.begin(), buffer2, buffer2 + nb);
		delete[] buffer;

		auto args = new ReplayInGame::Arguments(entries[P1pos], entries[P2pos], stages[params.stage]);

		args->lpos = P1pos;
		args->lpalette = P1palette;
		args->linput = std::make_shared<ReplayInput>(P1inputs);
		args->rpos = P2pos;
		args->rpalette = P2palette;
		args->rinput = std::make_shared<ReplayInput>(P2inputs);
		args->params = params;
		args->frameCount = frameCount;
		args->platforms = stages[params.stage].platforms[params.platforms];
		// TODO: Do this properly
		args->licon = 0;
		args->ricon = 0;
		args->lJson = entries[P1pos].entry;
		args->rJson = entries[P2pos].entry;
		game->scene.switchScene("replay_in_game", args);
	}

	void TitleScreen::_fetchReplayList()
	{
#if 0
		DIR *dir = opendir(("replays/" + this->_basePath).c_str());
		struct dirent *entry;
		struct stat s;

		for (struct dirent *entry = readdir(dir); entry; entry = readdir(dir)) {
			entry->d_name;
		}
#endif
	}

	TitleScreen *TitleScreen::create(SceneArguments *args)
	{
		auto result = new TitleScreen();

		if (args)
			result->_errorMsg = reinterpret_cast<TitleScreenArguments *>(args)->errorMessage;
		return result;
	}

	std::shared_ptr<IInput> TitleScreen::_getInputFromId(unsigned int id, const std::pair<std::shared_ptr<KeyboardInput>, std::shared_ptr<ControllerInput>> &pair)
	{
		if (id == STICK_ID_KEYBOARD)
			return pair.first;
	#ifdef VIRTUAL_CONTROLLER
		if (id == STICK_ID_VPAD)
			return game->virtualController;
	#endif
		pair.second->setJoystickId(id - STICK_ID_PPAD1);
		return pair.second;
	}
}
