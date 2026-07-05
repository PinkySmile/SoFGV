//
// Created by PinkySmile on 24/09/2021.
//

#ifdef _WIN32
#include <windows.h>
#undef max
#undef min
#else
#include <arpa/inet.h>
#include <dirent.h>
#endif
#include <memory>
#include <utility>
#include "TitleScreen.hpp"
#include "../InGame.hpp"
#include "../CharacterSelect.hpp"
#include "Resources/Game.hpp"
#include "Logger.hpp"
#include "KeyboardInput.hpp"
#include "ControllerInput.hpp"
#include "Utils.hpp"
#include "Inputs/ReplayInput.hpp"
#include "Resources/version.h"
#include "../ReplayInGame.hpp"
#ifdef HAS_NETWORK
#ifdef __EMSCRIPTEN__
#include <emscripten/websocket.h>
#include <emscripten/val.h>
#include <emscripten_browser_clipboard.h>
#endif
#include "Resources/Network/ServerConnection.hpp"
#include "Resources/Network/ClientConnection.hpp"
#include "Resources/Network/SpectatorConnection.hpp"
#endif
#ifdef VIRTUAL_CONTROLLER
#include "VirtualController.hpp"
#endif

#if defined(_DEBUG) && defined(HAS_NETWORK)
#define HAS_SYNC_TEST
#endif
#define THRESHOLD 50
#ifdef HAS_SYNC_TEST
#define MODE_HAS_2_LOCAL_PLAYERS(item) (item == TITLE_SCREEN_BUTTON(MULTIPLAYER_MODE, OFFLINE) || item == TITLE_SCREEN_BUTTON(SOLO_MODE, PRACTICE) || item == TITLE_SCREEN_BUTTON(DEBUG, SYNC_TEST))
#else
#define MODE_HAS_2_LOCAL_PLAYERS(item) (item == TITLE_SCREEN_BUTTON(MULTIPLAYER_MODE, OFFLINE) || item == TITLE_SCREEN_BUTTON(SOLO_MODE, PRACTICE))
#endif

#define MIN_ERROR_SHOW 15
#define MAX_ERROR_SHOW 180

#define DEFAULT_PORT 1326

enum TitleScreenButtonMainMenu {
	BUTTON_MAIN_MENU_SOLO_MODE,
	BUTTON_MAIN_MENU_MULTIPLAYER,
	BUTTON_MAIN_MENU_SETTINGS,
	BUTTON_MAIN_MENU_EXTRA,
	BUTTON_MAIN_MENU_QUIT
};

enum TitleScreenButtonSoloMode {
	BUTTON_SOLO_MODE_STORY_MODE,
	BUTTON_SOLO_MODE_VS_COM,
	BUTTON_SOLO_MODE_PRACTICE,
	BUTTON_SOLO_MODE_TRIAL_MODE,
	BUTTON_SOLO_MODE_TUTORIAL,
	BUTTON_SOLO_MODE_BACK
};

enum TitleScreenButtonMultiplayerMode {
	BUTTON_MULTIPLAYER_MODE_OFFLINE,
	BUTTON_MULTIPLAYER_MODE_HOST,
	BUTTON_MULTIPLAYER_MODE_CONNECT,
	BUTTON_MULTIPLAYER_MODE_SPECTATE,
	BUTTON_MULTIPLAYER_MODE_BACK
};

enum TitleScreenButtonSettings {
	BUTTON_SETTINGS_COLORS,
	BUTTON_SETTINGS_NETPLAY,
	BUTTON_SETTINGS_SOUNDS,
	BUTTON_SETTINGS_KEY_CONFIGS,
	BUTTON_SETTINGS_BACK
};

enum TitleScreenButtonExtra {
	BUTTON_EXTRA_REPLAYS,
	BUTTON_EXTRA_SOUND_TEST,
	BUTTON_EXTRA_CREDITS,
	BUTTON_EXTRA_COMMUNITY,
	BUTTON_EXTRA_BACK
};

#ifdef _DEBUG
enum TitleScreenButtonDebug {
#ifdef HAS_NETWORK
	BUTTON_DEBUG_SYNC_TEST,
#endif
	BUTTON_DEBUG_BACK
};
#endif

enum TitleScreenChunks {
	CHUNK_MAIN_MENU,
	CHUNK_SOLO_MODE,
	CHUNK_MULTIPLAYER_MODE,
	CHUNK_SETTINGS,
	CHUNK_EXTRA,
#ifdef _DEBUG
	CHUNK_DEBUG,
#endif
};

#define TITLE_SCREEN_BUTTON(index, name) (static_cast<int>(CHUNK_##index) << 8 | BUTTON_##index##_##name)

#define STICK_ID_KEYBOARD 0
#define STICK_ID_VPAD 1
#define STICK_ID_PPAD1 2

// FIXME:
extern std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> loadInputs(const std::string &path);

namespace SpiralOfFate
{
	static constexpr unsigned inputsOrder[]{
		INPUT_LEFT,
		INPUT_RIGHT,
		INPUT_UP,
		INPUT_DOWN,
		INPUT_NEUTRAL,
		INPUT_SPIRIT,
		INPUT_PAUSE
	};

	TitleScreen::TitleScreen() :
		_titleBg{ game->textureMgr.load("assets/ui/titlebackground.png") },
		_titleLogo{ game->textureMgr.load("assets/ui/title.png") },
		_titleSpiral{ game->textureMgr.load("assets/ui/spiral.png") },
		_inputs{
			{ game->textureMgr.load("assets/icons/inputs/4.png") },
			{ game->textureMgr.load("assets/icons/inputs/6.png") },
			{ game->textureMgr.load("assets/icons/inputs/8.png") },
			{ game->textureMgr.load("assets/icons/inputs/2.png") },
			{ game->textureMgr.load("assets/icons/inputs/neutral.png") },
			{ game->textureMgr.load("assets/icons/inputs/spirit.png") },
			{ game->textureMgr.load("assets/icons/inputs/matter.png") },
			{ game->textureMgr.load("assets/icons/inputs/void.png") },
			{ game->textureMgr.load("assets/icons/inputs/ascend.png") },
			{ game->textureMgr.load("assets/icons/inputs/dash.png") },
			{ game->textureMgr.load("assets/icons/inputs/pause.png") }
		}
	{
		std::vector<std::vector<MenuItemSkeleton>> array = {
			{
				{"Solo mode", "Fight by yourself", [this]{
					this->_menuObject->setEnabledMenu(CHUNK_SOLO_MODE, false);
				}},
				{"Multiplayer mode", "Fight a human opponent", [this]{
					this->_menuObject->setEnabledMenu(CHUNK_MULTIPLAYER_MODE, false);
				}},
				{"Settings", "Change settings", [this]{
					this->_menuObject->setEnabledMenu(CHUNK_SETTINGS, false);
				}},
				{"Extra", "Other", [this]{
					this->_menuObject->setEnabledMenu(CHUNK_EXTRA, false);
				}},
			#ifdef _DEBUG
				{"Debug", "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?", [this]{
					this->_menuObject->setEnabledMenu(CHUNK_DEBUG, false);
				}},
			#endif
			#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
				{"Quit", "Quit game", []{
					game->screen->close();
				}},
			#endif
			},
			{
				{"Story Mode", "Discover the universe\n\nComing soon!", nullptr},
				{"VS Computer", "Fight a virtual opponent\n\nComing soon!", nullptr},
				{"Practice", "Free training", [this]{
					this->_askingInputs = true;
				}},
				{"Trial Mode", "Combo training\n\nComing soon!", nullptr},
				{"Tutorial", "Learn the basics\n\nComing soon!", nullptr},
				{"Back", "Go back to the main menu", [this]{
					this->_menuObject->setEnabledMenu(CHUNK_MAIN_MENU);
				}},
			},
			{
				{"Offline", "Play a game against a human opponent on the same computer", [this]{
					this->_askingInputs = true;
				}},
			#ifdef HAS_NETWORK
				{"Host", "Host an online game", [this]{
					this->_askingInputs = true;
				}},
				{"Connect", "Connect to ip from clipboard", [this]{
					this->_askingInputs = true;
				}},
				{"Spectate", "Connect to ip from clipboard", [this]{
				#ifdef __EMSCRIPTEN__
					this->_selectingRoom = true;
					this->_typingCode.clear();
				#else
					this->_spectate(sf::Clipboard::getString().toAnsiString());
				#endif
				}},
			#endif
				{"Back", "Go back to the main menu", [this]{
					this->_menuObject->setEnabledMenu(CHUNK_MAIN_MENU);
				}},
			},
			{
				{"Colors", "Change type colors\n\nComing soon!", nullptr},
				{"Netplay", "Change various online options\n\nComing soon!", nullptr},
				{"Sound", "\n\nComing soon!", nullptr},
				{"Key Config", "Change inputs", [this]{
					this->_changingInputs = 1;
					this->_cursorInputs = 0;
				}},
				{"Back", "Go back to the main menu", [this]{
					this->_menuObject->setEnabledMenu(CHUNK_MAIN_MENU);
				}},
			},
			{
				{"Replays", "Select a replay to watch", [this]{
					auto window = Utils::openFileDialog(game->gui, "Open replay", "replays");

					window->setFileTypeFilters({ {"Replay file (*.replay)", {"*.replay"}}, {"All files", {}} }, 0);
					window->onFileSelect([this](const std::vector<tgui::Filesystem::Path> &arr){
						try {
							this->_loadReplay(arr[0]);
						} catch (std::exception &e) {
							Utils::dispMsg(game->gui, "Replay loading failed", "This replay is invalid, corrupted or was created for a different version of the game: " + std::string(e.what()), MB_ICONERROR);
						}
					});
				}},
				{"Music Room", "Catchy tune!\n\nComing soon!", nullptr},
				{"Credits", "\n\nComing soon!", nullptr},
				{"Back", "Go back to the main menu", [this]{
					this->_menuObject->setEnabledMenu(CHUNK_MAIN_MENU);
				}},
			},
		#ifdef _DEBUG
			{
			#ifdef HAS_SYNC_TEST
				{"Sync Test", "Verify that rollback doesn't desync", [this]{
					this->_askingInputs = true;
				}},
			#endif
				{"Back", "Go back to the main menu", [this]{
					this->_menuObject->setEnabledMenu(CHUNK_MAIN_MENU);
				}},
			}
		#endif
		};

	#ifdef __EMSCRIPTEN__
		if (!emscripten_websocket_is_supported()) {
			array[CHUNK_MULTIPLAYER_MODE][BUTTON_MULTIPLAYER_MODE_HOST].onClick = nullptr;
			array[CHUNK_MULTIPLAYER_MODE][BUTTON_MULTIPLAYER_MODE_CONNECT].onClick = nullptr;
			array[CHUNK_MULTIPLAYER_MODE][BUTTON_MULTIPLAYER_MODE_SPECTATE].onClick = nullptr;
			array[CHUNK_MULTIPLAYER_MODE][BUTTON_MULTIPLAYER_MODE_HOST].desc = "Not supported by your browser!\nOnline requires Websocket support.";
			array[CHUNK_MULTIPLAYER_MODE][BUTTON_MULTIPLAYER_MODE_CONNECT].desc = "Not supported by your browser!\nOnline requires Websocket support.";
			array[CHUNK_MULTIPLAYER_MODE][BUTTON_MULTIPLAYER_MODE_SPECTATE].desc = "Not supported by your browser!\nOnline requires Websocket support.";
		}
	#endif
		this->_menuObject = std::make_unique<Menu>("assets/ui/copperplate-gothic-light.ttf", "assets/ui/gillsansmt.ttf", array);
		game->logger.info("Title scene created");
		this->_titleLogo.setPosition({275, 31});
		this->_titleSpiral.setOrigin({140, 131});
		this->_titleSpiral.setPosition({328, 139});
		this->_netbellSound = game->soundMgr.load("assets/sfxs/se/057.ogg");
		this->_lastInput = &*game->menu.first;
	}

	TitleScreen::~TitleScreen()
	{
		if (this->onDestruct)
			this->onDestruct();
		game->logger.debug("~TitleScreen");
		game->soundMgr.remove(this->_netbellSound);
	}

	void TitleScreen::render() const
	{
		ViewPort view{{{0, 0}, {1680, 960}}};

		game->screen->setView(view);
		game->screen->displayElement(this->_titleBg);
		game->screen->displayElement(this->_titleSpiral);
		game->screen->displayElement(this->_titleLogo);
		this->_menuObject->render();
		if (!this->_errorMsg.empty()) {
			game->screen->fillColor(Color::White);
			game->screen->displayElement({540, 280, 600, 100}, Color{0x50, 0x50, 0x50});
			game->screen->displayElement(this->_errorMsg, {540, 300}, 600, Screen::ALIGN_CENTER);
		}
	#ifdef HAS_NETWORK
		else if (this->_connecting)
			this->_showConnectMessage();
		else if (game->connection)
			this->_showHostMessage();
		else if (this->_chooseSpecCount)
			this->_showChooseSpecCount();
	#ifdef __EMSCRIPTEN__
		else if (this->_selectingRoom)
			this->_showSelectingRoom();
	#endif
	#endif
		else if (this->_askingInputs)
			this->_showAskInputBox();
		if (this->_changingInputs)
			this->_showEditKeysMenu();
	}

	void TitleScreen::update()
	{
		this->_timer++;
		this->_titleSpiral.setRotation(this->_titleSpiral.getRotation() - sf::degrees(0.25));
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
			#ifdef __EMSCRIPTEN__
				this->_selectingRoom = true;
			#endif
			}
			game->connection.reset();
		}
	#endif
		if (!this->_errorMsg.empty())  {
			if (this->_errorTimer == 0)
				game->soundMgr.play(BASICSOUND_MENU_CANCEL);
			this->_errorTimer++;
			if (this->_errorTimer > MAX_ERROR_SHOW) {
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
			this->_menuObject->update({});
		else
			this->_menuObject->update(inputs);
	}

	void TitleScreen::consumeEvent(const sf::Event &event)
	{
		if (auto e = event.getIf<sf::Event::KeyPressed>()) {
			if (this->_onKeyPressed(*e))
				return;
		}
		if (auto e = event.getIf<sf::Event::JoystickButtonPressed>()) {
			if (this->_onJoystickPressed(*e))
				return;
		}
		if (auto e = event.getIf<sf::Event::JoystickMoved>()) {
			if (this->_onJoystickMoved(*e))
				return;
		}
#ifdef VIRTUAL_CONTROLLER
		if (
			event.is<sf::Event::MouseMoved>() ||
			event.is<sf::Event::MouseButtonPressed>() ||
			event.is<sf::Event::TouchBegan>() ||
			event.is<sf::Event::TouchMoved>()
		) {
			if (!this->_changingInputs) {
				this->_latestJoystickId = STICK_ID_VPAD;
				this->_lastInput = &*game->virtualController;
			}
		}
#endif
		game->menu.first->consumeEvent(event);
		game->menu.second->consumeEvent(event);
	}

#ifdef HAS_NETWORK
	void TitleScreen::_host(bool spec)
	{
		game->activeNetInput = TitleScreen::_getInputFromId(this->_leftInput - 1, game->P1);

		// TODO: Handle names
		auto con = new ServerConnection("SpiralOfFate::ServerConnection");
		// TODO: Handle port better
		std::ifstream stream{"hostPort.txt"};

		if (stream)
			stream >> this->_hostingPort;
		else
			this->_hostingPort = DEFAULT_PORT;
		game->connection.reset(con);
		con->onConnection = [this](Connection::Remote &remote, PacketInitRequest &packet){
			std::string name{packet.playerName, strnlen(packet.playerName, sizeof(packet.playerName))};
			std::string vers{packet.gameVersion, strnlen(packet.gameVersion, sizeof(packet.gameVersion))};

			game->logger.info(name + " connected with game version " + vers);
			this->_onConnect(remote.ip.toString() + ":" + std::to_string(remote.port));
		};
		con->onError = [](Connection::Remote &remote, const PacketError &e){
			game->logger.error(remote.ip.toString() + ":" + std::to_string(remote.port) + " -> " + e.toString());
		};
		con->onDisconnect = [this](Connection::Remote &remote){
			this->_onDisconnect("[" + remote.ip.toString() + "]:" + std::to_string(remote.port));
		};
		con->spectatorEnabled = spec;
		con->host(this->_hostingPort);
		this->_hostingPort = con->getLocalPort();
		this->onDestruct = [con]{
			con->onConnection = nullptr;
			con->onDisconnect = nullptr;
		};
	#ifdef __EMSCRIPTEN__
		std::string host = emscripten::val::global("window")["location"]["hostname"].as<std::string>();

		emscripten_browser_clipboard::copy(host + ":" + std::to_string(this->_hostingPort));
	#endif
	}

	std::optional<std::pair<sf::IpAddress, unsigned short>> TitleScreen::_getIPPort(const std::string &ipString) const
	{
		if (ipString.empty()) {
			Utils::dispMsg(game->gui, "Error", "No ip is copied to the clipboard", MB_ICONERROR);
			return {};
		}

		auto isIPV4 = std::ranges::count(ipString, ':') <= 1;
		std::optional<std::vector<sf::IpAddress>> ips;
		unsigned short port = DEFAULT_PORT;
		size_t pos = std::string::npos;
		std::string hostStr;

		if (isIPV4) {
			pos = ipString.find_last_of(':');
			hostStr = ipString.substr(0, pos);
		} else if (ipString[0] == '[') {
			size_t pos2 = ipString.find_last_of(']');

			if (pos2 == std::string::npos) {
				Utils::dispMsg(game->gui, "Error", "Clipboard doesn't contain a valid IP address", MB_ICONERROR);
				return {};
			}
			hostStr = ipString.substr(1, pos2 - 1);
			pos = ipString.find_last_of(':');
		} else
			hostStr = ipString;

		ips = sf::Dns::resolve(hostStr);
		if (!ips) {
			Utils::dispMsg(game->gui, "Error", "Clipboard doesn't contain a valid IP address", MB_ICONERROR);
			return {};
		}
		if (pos != std::string::npos) {
			try {
				auto p = std::stoul(ipString.substr(pos + 1));

				if (p > UINT16_MAX)
					throw std::exception();
				port = p;
			} catch (...) {
			#ifndef __EMSCRIPTEN__
				Utils::dispMsg(game->gui, "Error", "Clipboard doesn't contain a valid IP address (Invalid port)", MB_ICONERROR);
			#else
				Utils::dispMsg(game->gui, "Error", "Invalid room code", MB_ICONERROR);
			#endif
				return {};
			}
		}

		sf::IpAddress final = (*ips)[0];

		for (size_t i = 1; !final.isV6() && i < ips->size(); i++)
			if ((*ips)[i].isV6())
				final = (*ips)[i];
		game->lastIp = final.toString();
		game->lastPort = port;
		return std::make_pair(final, port);
	}

	void TitleScreen::_connect(const std::string &ipString)
	{
		game->activeNetInput = TitleScreen::_getInputFromId(this->_leftInput - 1, game->P1);

		auto pair = this->_getIPPort(ipString);

		if (!pair)
			return;

		// TODO: Handle names
		auto con = new ClientConnection("SpiralOfFate::ClientConnection");

		game->connection.reset(con);
		con->onConnection = [this](Connection::Remote &remote, PacketInitSuccess &packet){
			std::string name{packet.player1Name, strnlen(packet.player1Name, sizeof(packet.player1Name))};
			std::string vers{packet.gameVersion, strnlen(packet.gameVersion, sizeof(packet.gameVersion))};

			game->logger.info("Connected to " + name + " with game version " + vers);
			this->_onConnect(remote.ip.toString() + ":" + std::to_string(remote.port));
		};
		con->onError = [](Connection::Remote &remote, const PacketError &e){
			game->logger.error(remote.ip.toString() + ":" + std::to_string(remote.port) + " -> " + e.toString());
			// TODO: Abort connection and display error on UI
		};
		con->onDisconnect = [this](Connection::Remote &remote){
			this->_onDisconnect("[" + remote.ip.toString() + "]:" + std::to_string(remote.port));
		};
		con->connect(pair->first, pair->second);
		this->_connecting = true;
		this->onDestruct = [con]{
			con->onConnection = nullptr;
			con->onDisconnect = nullptr;
		};
	}

	void TitleScreen::_spectate(const std::string &ipString)
	{
		auto pair = this->_getIPPort(ipString);

		if (!pair)
			return;

		auto con = new SpectatorConnection();

		game->connection.reset(con);
		con->onConnection = [this](Connection::Remote &remote, PacketInitSuccess &packet){
			std::string name{packet.player1Name, strnlen(packet.player1Name, sizeof(packet.player1Name))};
			std::string vers{packet.gameVersion, strnlen(packet.gameVersion, sizeof(packet.gameVersion))};

			game->logger.info("Connected to " + name + " with game version " + vers);
			this->_onConnect(remote.ip.toString() + ":" + std::to_string(remote.port));
		};
		con->onError = [](Connection::Remote &remote, const PacketError &e){
			game->logger.error(remote.ip.toString() + ":" + std::to_string(remote.port) + " -> " + e.toString());
			// TODO: Abort connection and display error on UI
		};
		con->onDisconnect = [this](Connection::Remote &remote){
			this->_onDisconnect("[" + remote.ip.toString() + "]:" + std::to_string(remote.port));
		};
		con->connect(pair->first, pair->second);
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

		switch (this->_menuObject->getEnabledMenu() << 8 | this->_menuObject->getSelectedItem()) {
		case TITLE_SCREEN_BUTTON(MULTIPLAYER_MODE, OFFLINE):
			args = new CharacterSelect::Arguments();
			args->leftInput = TitleScreen::_getInputFromId(this->_leftInput - 1, game->P1);
			args->rightInput = TitleScreen::_getInputFromId(this->_rightInput - 1, game->P2);
			args->inGameName = "in_game";
			game->scene.switchScene("char_select", args);
			break;
		case TITLE_SCREEN_BUTTON(SOLO_MODE, PRACTICE):
			args = new CharacterSelect::Arguments();
			args->leftInput = TitleScreen::_getInputFromId(this->_leftInput - 1, game->P1);
			args->rightInput = TitleScreen::_getInputFromId(this->_rightInput - 1, game->P2);
			args->inGameName = "practice_in_game";
			game->scene.switchScene("char_select", args);
			break;
	#ifdef HAS_NETWORK
		case TITLE_SCREEN_BUTTON(MULTIPLAYER_MODE, HOST):
			this->_chooseSpecCount = true;
			break;
		case TITLE_SCREEN_BUTTON(MULTIPLAYER_MODE, CONNECT):
		#ifdef __EMSCRIPTEN__
			this->_selectingRoom = true;
			this->_typingCode.clear();
		#else
			this->_connect(sf::Clipboard::getString().toAnsiString());
		#endif
			break;
	#endif
	#ifdef HAS_SYNC_TEST
		case TITLE_SCREEN_BUTTON(DEBUG, SYNC_TEST):
			args = new CharacterSelect::Arguments();
			args->leftInput = TitleScreen::_getInputFromId(this->_leftInput - 1, game->P1);
			args->rightInput = TitleScreen::_getInputFromId(this->_rightInput - 1, game->P2);
			args->inGameName = "sync_test_in_game";
			game->scene.switchScene("char_select", args);
			break;
	#endif
		}
	}

	bool TitleScreen::_onKeyPressed(const sf::Event::KeyPressed &ev)
	{
	#if defined(__EMSCRIPTEN__) && defined(HAS_NETWORK)
		if (this->_selectingRoom) {
			if (ev.code >= sf::Keyboard::Key::Num0 && ev.code <= sf::Keyboard::Key::Num9 && this->_typingCode.size() < 5) {
				this->_typingCode += static_cast<char>('0' + (static_cast<int>(ev.code) - static_cast<int>(sf::Keyboard::Key::Num0)));
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
			} else if (ev.code == sf::Keyboard::Key::Backspace && !this->_typingCode.empty()) {
				this->_typingCode.pop_back();
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
			} else if (ev.code == sf::Keyboard::Key::Escape)
				this->_onCancel();
			else if (ev.code == sf::Keyboard::Key::Enter)
				this->_onConfirm(STICK_ID_KEYBOARD);
			return true;
		}
	#endif
		if (ev.code == sf::Keyboard::Key::F1 && (this->_changingInputs || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))) {
			auto &pair = (this->_changingInputs == 2 ? game->P1 : this->_changingInputs == 3 ? game->P2 : game->menu);

			pair.first = std::make_shared<KeyboardInput>();
			pair.second = std::make_shared<ControllerInput>();
			this->_lastInput = &*pair.first;
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			if (this->_changingInputs == 0)
				this->_errorMsg = "Menu keys reset";
			return false;
		}
		if (ev.code == sf::Keyboard::Key::F2 && this->_changingInputs > 1) {
			auto dialog = Utils::saveFileDialog(game->gui, "Save inputs", "./profiles");

			dialog->setFileTypeFilters({ {"Inputs file (*.in)", {"*.in"}}, {"All files", {}} }, 0);
			dialog->onFileSelect([this](const std::vector<tgui::Filesystem::Path> &arr){
				std::ofstream stream{static_cast<std::filesystem::path>(arr[0])};

				if (!stream)
					Utils::dispMsg(game->gui, "Saving error", strerror(errno), MB_ICONERROR);

				auto &pair = (this->_changingInputs == 2 ? game->P1 : game->P2);

				pair.first->save(stream);
				pair.second->save(stream);
				game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			});
			return false;
		}
		if (ev.code == sf::Keyboard::Key::F3 && this->_changingInputs > 1) {
			auto dialog = Utils::openFileDialog(game->gui, "Load inputs", "profiles");

			dialog->setFileTypeFilters({ {"Inputs file (*.in)", {"*.in"}}, {"All files", {}} }, 0);
			dialog->onFileSelect([this](const std::vector<tgui::Filesystem::Path> &arr) {
				auto &pair = (this->_changingInputs == 2 ? game->P1 : game->P2);

				// FIXME: Use profile-like system
				pair = loadInputs(static_cast<std::filesystem::path>(arr[0]).string());
				game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			});
			return false;
		}
		if (this->_changeInput && this->_changingInputs) {
			if (sf::Keyboard::Key::Escape == ev.code) {
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
		case sf::Keyboard::Key::Escape:
			this->_onCancel();
			break;
		default:
			break;
		}
		return false;
	}

	bool TitleScreen::_onJoystickMoved(const sf::Event::JoystickMoved &ev)
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

	bool TitleScreen::_onJoystickPressed(const sf::Event::JoystickButtonPressed &ev)
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
		game->screen->displayElement({540, 180, 600, 300}, Color{0x50, 0x50, 0x50});

		game->screen->fillColor(this->_leftInput ? Color::Green : Color::White);
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

		auto item = this->_menuObject->getEnabledMenu() << 8 | this->_menuObject->getSelectedItem();

		if (MODE_HAS_2_LOCAL_PLAYERS(item))
			game->screen->fillColor(this->_rightInput ? sf::Color::Green : (this->_leftInput ? sf::Color::White : sf::Color{0xA0, 0xA0, 0xA0}));
		else
			game->screen->fillColor(sf::Color{0x80, 0x80, 0x80});
		game->screen->displayElement("P2", {540 + 420, 190});
		game->screen->fillColor(sf::Color::White);
		if (this->_leftInput && MODE_HAS_2_LOCAL_PLAYERS(item)) {
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

		if (this->_leftInput && (this->_rightInput || !MODE_HAS_2_LOCAL_PLAYERS(item)))
			game->screen->displayElement("Press [Confirm] to confirm", {540, 360}, 600, Screen::ALIGN_CENTER);
	}

#ifdef HAS_NETWORK
	void TitleScreen::_showHostMessage() const
	{
		game->screen->fillColor(sf::Color::White);
		if (this->_remote.empty()) {
		#ifndef __EMSCRIPTEN__
			game->screen->displayElement({540, 280, 600, 100}, sf::Color{0x50, 0x50, 0x50});
			game->screen->displayElement("Waiting for opponent...", {640, 300}, 400, Screen::ALIGN_CENTER);
			game->screen->displayElement("Hosting on port " + std::to_string(this->_hostingPort), {640, 330}, 400, Screen::ALIGN_CENTER);
		#else
			game->screen->displayElement({540, 280, 600, 150}, sf::Color{0x50, 0x50, 0x50});
			game->screen->displayElement("Waiting for opponent...", {640, 300}, 400, Screen::ALIGN_CENTER);
			game->screen->displayElement("Room code:" + std::to_string(this->_hostingPort), {640, 330}, 400, Screen::ALIGN_CENTER);
			game->screen->displayElement("IP copied to clipboard", {640, 380}, 400, Screen::ALIGN_CENTER);
		#endif
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
		#ifdef __EMSCRIPTEN__
			game->screen->displayElement({540, 280, 600, 100}, sf::Color{0x50, 0x50, 0x50});
			game->screen->displayElement("Joining room " + std::to_string(game->lastPort), {540, 300}, 600, Screen::ALIGN_CENTER);
		#else
			game->screen->displayElement({340, 280, 1000, 100}, sf::Color{0x50, 0x50, 0x50});
			game->screen->displayElement("Connecting to " + game->lastIp + " on port " + std::to_string(game->lastPort), {540, 300}, 600, Screen::ALIGN_CENTER);
		#endif
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
			for (size_t j = 0; j < std::size(inputsOrder); j++) {
				auto i = inputsOrder[j];

				if (this->_changeInput && this->_cursorInputs == i) {
					game->screen->fillColor(sf::Color{0xFF, 0x80, 0x00});
					game->screen->displayElement(names[j] + ": Press a key", {680, 146 + i * 68.f});
				} else {
					game->screen->fillColor(this->_cursorInputs == i ? sf::Color::Red : sf::Color::White);
					game->screen->displayElement(names[j] + ": " + strs[i], {680, 146 + i * 68.f});
				}
			}
		} else {
			game->screen->displayElement((this->_changingInputs == 2 ? "P1 | " : "P2 | ") + input->getName(), {640, 85}, 400, Screen::ALIGN_CENTER);
			game->screen->fillColor(sf::Color::White);
			for (unsigned i = 0; i < this->_inputs.size(); i++) {
				this->_inputs[i].setPosition({680, 135 + i * 68.f});
				game->screen->displayElement(this->_inputs[i]);
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
			} while (std::find(inputsOrder, std::end(inputsOrder), this->_cursorInputs) == std::end(inputsOrder) && this->_changingInputs == 1);
			return;
		}
		if (this->_askingInputs)
			return;
		game->soundMgr.play(BASICSOUND_MENU_MOVE);
		this->_menuObject->setSelectedItem(this->_menuObject->getSelectedItem() - 1);
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
			} while (std::find(inputsOrder, std::end(inputsOrder), this->_cursorInputs) == std::end(inputsOrder) && this->_changingInputs == 1);
			return;
		}
	#ifdef HAS_NETWORK
		if (this->_chooseSpecCount)
			return;
	#endif
		if (this->_askingInputs)
			return;
		game->soundMgr.play(BASICSOUND_MENU_MOVE);
		this->_menuObject->setSelectedItem(this->_menuObject->getSelectedItem() + 1);
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
			while (std::find(inputsOrder, std::end(inputsOrder), this->_cursorInputs) == std::end(inputsOrder) && this->_changingInputs == 1) {
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
			while (std::find(inputsOrder, std::end(inputsOrder), this->_cursorInputs) == std::end(inputsOrder) && this->_changingInputs == 1) {
				this->_cursorInputs += this->_inputs.size();
				this->_cursorInputs--;
				this->_cursorInputs %= this->_inputs.size();
			}
			return;
		}
	}

	void TitleScreen::_onConfirm(unsigned stickId)
	{
		if (!this->_errorMsg.empty()) {
			if (this->_errorTimer < MIN_ERROR_SHOW)
				return;
			this->_errorMsg.clear();
			this->_errorTimer = 0;
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			return;
		}
	#ifdef HAS_NETWORK
		if (game->connection)
			return;
		if (this->_chooseSpecCount) {
			this->_chooseSpecCount = false;
			this->_host(this->_specEnabled);
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			return;
		}
	#ifdef __EMSCRIPTEN__
		if (this->_selectingRoom) {
			auto item = this->_menuObject->getEnabledMenu() << 8 | this->_menuObject->getSelectedItem();

			this->_selectingRoom = false;
			if (item == TITLE_SCREEN_BUTTON(MULTIPLAYER_MODE, CONNECT))
				this->_connect("0.0.0.0:" + this->_typingCode);
			else
				this->_spectate("0.0.0.0:" + this->_typingCode);
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			return;
		}
	#endif
	#endif
		if (this->_changingInputs) {
			this->_changeInput = true;
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			return;
		}
		if (this->_askingInputs) {
			auto item = this->_menuObject->getEnabledMenu() << 8 | this->_menuObject->getSelectedItem();

			if (this->_rightInput || (this->_leftInput && !MODE_HAS_2_LOCAL_PLAYERS(item)))
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

	static void saveInputs(const std::pair<std::shared_ptr<KeyboardInput>, std::shared_ptr<ControllerInput>> &input, const std::string &path)
	{
		auto parent = std::filesystem::path(path).parent_path();

		if (!parent.empty())
			std::filesystem::create_directories(parent);
		game->logger.debug("Saving inputs to " + path);

		std::ofstream stream{path};

		input.first->save(stream);
		input.second->save(stream);
	}

	void TitleScreen::_onCancel()
	{
		if (!this->_errorMsg.empty()) {
			if (this->_errorTimer < MIN_ERROR_SHOW)
				return;
			this->_errorMsg.clear();
			this->_errorTimer = 0;
			game->soundMgr.play(BASICSOUND_MENU_CANCEL);
			return;
		}
		game->soundMgr.play(BASICSOUND_MENU_CANCEL);
	#ifdef HAS_NETWORK
		if (game->connection) {
			this->_connecting = false;
			return game->connection.reset();
		}
	#ifdef __EMSCRIPTEN__
		if (this->_selectingRoom) {
			this->_selectingRoom = false;
			return;
		}
	#endif
		if (this->_chooseSpecCount) {
			this->_chooseSpecCount = false;
			return;
		}
	#endif
		if (this->_changingInputs) {
			this->_changingInputs = 0;
			saveInputs(game->menu, "settings/menuInputs.in");
			saveInputs(game->P1, game->settings.inputPresetP1);
			saveInputs(game->P2, game->settings.inputPresetP2);
			return;
		}
		if (this->_askingInputs) {
			auto item = this->_menuObject->getEnabledMenu() << 8 | this->_menuObject->getSelectedItem();

			if (this->_rightInput && MODE_HAS_2_LOCAL_PLAYERS(item))
				this->_rightInput = 0;
			else if (this->_leftInput)
				this->_leftInput = 0;
			else
				this->_askingInputs = false;
			return;
		}
	#ifdef __EMSCRIPTEN__
		if (this->_menuObject->getEnabledMenu() != CHUNK_MAIN_MENU)
			this->_menuObject->setSelectedItem(this->_menuObject->getMenuSize() - 1);
	#else
		this->_menuObject->setSelectedItem(this->_menuObject->getMenuSize() - 1);
	#endif
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
		game->screen->displayElement("Enable spectating?", {640, 290}, 400, Screen::ALIGN_CENTER);
		game->screen->displayElement(this->_specEnabled ? "Spectating enabled" : "Spectating disabled", {640, 330}, 400, Screen::ALIGN_CENTER);
	}

#ifdef __EMSCRIPTEN__
	void TitleScreen::_showSelectingRoom() const
	{
		game->screen->displayElement({620, 280, 440, 100}, sf::Color{0x50, 0x50, 0x50});
		game->screen->fillColor(sf::Color::White);
		game->screen->displayElement("Enter room code", {640, 290}, 400, Screen::ALIGN_CENTER);
		game->screen->displayElement(this->_typingCode + (this->_timer % 20 < 10 ? "_" : " "), {640, 330}, 400, Screen::ALIGN_CENTER);
	}
#endif
#endif

	void TitleScreen::_loadReplay(const std::filesystem::path &path)
	{
		std::ifstream stream{path, std::ifstream::binary};
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
		unsigned magic = 0;
		unsigned frameCount;
		unsigned expectedMagic = getMagic();
		uint32_t random;

		if (!stream)
			throw std::invalid_argument("Cannot load " + path.string() + ": " + strerror(errno));
		game->logger.info("Loading replay " + path.string());
		stream.read(reinterpret_cast<char *>(&magic), sizeof(magic));
		game->logger.debug("Expected magic " + std::to_string(expectedMagic) + " vs Replay magic " + std::to_string(magic));
		if (magic != expectedMagic)
			throw std::invalid_argument("INVALID_MAGIC");

		auto characterSelectData = CharacterSelect::loadData();
		auto &stages = characterSelectData.first;
		auto &entries = characterSelectData.second;

		stream.read(reinterpret_cast<char *>(&frameCount), sizeof(frameCount));
		stream.read(reinterpret_cast<char *>(&random), sizeof(random));
		stream.read(reinterpret_cast<char *>(&params), sizeof(params));
		game->logger.debug("Params: stageID " + std::to_string(params.stage) + ", platformsID " + std::to_string(params.platforms) + ", musicID " + std::to_string(params.music));
		if (params.stage >= stages.size())
			throw std::invalid_argument("INVALID_STAGE");
		if (params.platforms >= stages[params.stage].platforms.size())
			throw std::invalid_argument("INVALID_PLAT_CONF");


		stream.read(reinterpret_cast<char *>(&P1pos), sizeof(P1pos));
		stream.read(reinterpret_cast<char *>(&P1palette), sizeof(P1palette));
		game->logger.debug("Reading P1 entry: pos " + std::to_string(P1pos) + ", pal " + std::to_string(P1palette));
		if (P1pos >= entries.size())
			throw std::invalid_argument("INVALID_P1POS");
		if (P1palette >= entries[P1pos].palettes.size())
			throw std::invalid_argument("INVALID_P1PAL");

		stream.read(reinterpret_cast<char *>(&nb), sizeof(nb));
		game->logger.debug("P1 has " + std::to_string(nb) + " inputs");
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
		game->logger.debug("P2 has " + std::to_string(nb) + " inputs");
		buffer = new char[nb * sizeof(ReplayData)];
		stream.read(buffer, nb * sizeof(ReplayData));
		buffer2 = reinterpret_cast<ReplayData *>(buffer);
		P2inputs.insert(P2inputs.begin(), buffer2, buffer2 + nb);
		delete[] buffer;

		auto args = new ReplayInGame::Arguments{};

		args->frameCount = frameCount;
		args->params.seed = random;
		args->params.p1chr = P1pos;
		args->params.p1pal = P1palette;
		args->params.p2chr = P2pos;
		args->params.p2pal = P2palette;
		args->params.stage = params.stage;
		args->params.platformConfig = params.platforms;
		args->stages.swap(stages);
		args->entries.swap(entries);
		args->leftInput = std::make_shared<ReplayInput>(P1inputs);
		args->rightInput = std::make_shared<ReplayInput>(P2inputs);
		args->saveReplay = false;
		args->endScene = game->scene.getCurrentScene().first;
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
