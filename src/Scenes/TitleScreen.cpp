//
// Created by Gegel85 on 24/09/2021.
//

#ifdef _WIN32
#include <windows.h>
#else
// TODO: Add proper message boxes on non windows systems
#define MessageBox(...)
#endif
#include <utility>
#include <clip.h>
#include "TitleScreen.hpp"
#include "InGame.hpp"
#include "NetplayInGame.hpp"
#include "CharacterSelect.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"
#include "../Inputs/KeyboardInput.hpp"
#include "../Inputs/ControllerInput.hpp"
#include "../Inputs/RemoteInput.hpp"

#define THRESHOLD 50

namespace Battle
{
	static const char * const menuItems[] = {
		"Play",
		"Host",
		"Connect",
		"Settings",
		"Quit",
#ifdef _DEBUG
		"GGPO sync test"
#endif
	};

	TitleScreen::TitleScreen(
		std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> P1,
		std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> P2
	) :
		_P1(std::move(P1)),
		_P2(std::move(P2))
	{
		sf::View view{{0, 0, 1680, 960}};

		game.screen->setView(view);
		logger.info("Title scene created");
		this->_inputs.resize(INPUT_NUMBER);
		this->_inputs[INPUT_LEFT].loadFromFile("assets/icons/inputs/4.png");
		this->_inputs[INPUT_RIGHT].loadFromFile("assets/icons/inputs/6.png");
		this->_inputs[INPUT_UP].loadFromFile("assets/icons/inputs/8.png");
		this->_inputs[INPUT_DOWN].loadFromFile("assets/icons/inputs/2.png");
		this->_inputs[INPUT_N].loadFromFile("assets/icons/inputs/neutral.png");
		this->_inputs[INPUT_M].loadFromFile("assets/icons/inputs/matter.png");
		this->_inputs[INPUT_S].loadFromFile("assets/icons/inputs/spirit.png");
		this->_inputs[INPUT_V].loadFromFile("assets/icons/inputs/void.png");
		this->_inputs[INPUT_A].loadFromFile("assets/icons/inputs/ascend.png");
		this->_inputs[INPUT_D].loadFromFile("assets/icons/inputs/dash.png");
		this->_inputs[INPUT_PAUSE].loadFromFile("assets/icons/inputs/pause.png");
	}

	TitleScreen::~TitleScreen()
	{
		logger.debug("~TitleScreen");
		if (this->_thread.joinable())
			this->_thread.join();
	}

	void TitleScreen::render() const
	{
		for (unsigned i = 0; i < sizeof(menuItems) / sizeof(*menuItems); i++) {
			game.screen->fillColor(this->_selectedEntry == i ? sf::Color::Red : sf::Color::Black);
			game.screen->displayElement(menuItems[i], {100, static_cast<float>(100 + i * 40)});
		}

		if (this->_connecting)
			this->_showConnectMessage();
		else if (game.networkMgr.isHosting())
			this->_showHostMessage();
		else if (this->_askingInputs)
			this->_showAskInputBox();
		if (this->_changingInputs)
			this->_showEditKeysMenu();
	}

	IScene *TitleScreen::update()
	{
		game.random();
		return this->_nextScene;
	}

	void TitleScreen::consumeEvent(const sf::Event &event)
	{
		if (this->_nextScene)
			return;
		switch (event.type) {
		case sf::Event::KeyPressed:
			this->_onKeyPressed(event.key);
			break;
		case sf::Event::JoystickButtonPressed:
			this->_onJoystickPressed(event.joystickButton);
			break;
		case sf::Event::JoystickMoved:
			this->_onJoystickMoved(event.joystickMove);
			break;
		default:
			break;
		}
	}

	void TitleScreen::_host()
	{
		game.networkMgr.setInputs(this->_leftInput == 1 ? static_cast<std::shared_ptr<IInput>>(this->_P1.first) : static_cast<std::shared_ptr<IInput>>(this->_P1.second), nullptr);
		if (this->_thread.joinable())
			this->_thread.join();
		this->_thread = std::thread{[] {
			try {
				game.networkMgr.host(10800, 0);
			} catch (std::exception &e) {
				MessageBox(nullptr, e.what(), "Host error", MB_ICONERROR);
			} catch (...) {
				MessageBox(nullptr, "WTF???", "Host error", MB_ICONERROR);
			}
		}};
	}

	void TitleScreen::_connect()
	{
		game.networkMgr.setInputs(nullptr, this->_leftInput == 1 ? static_cast<std::shared_ptr<IInput>>(this->_P1.first) : static_cast<std::shared_ptr<IInput>>(this->_P1.second));
		if (this->_thread.joinable())
			this->_thread.join();
		this->_thread = std::thread{[this]{
			if (clip::has(clip::text_format())) {
				std::string ip;

				clip::get_text(ip);
				if (inet_addr(ip.c_str()) != -1)
					game.lastIp = ip;
			}
			try {
				this->_connecting = true;
				if (!game.networkMgr.connect(game.lastIp, 10800) && this->_connecting)
					throw std::invalid_argument("Failed to connect to " + game.lastIp);
			} catch (std::exception &e) {
				MessageBox(nullptr, e.what(), "Connect error", MB_ICONERROR);
			}
			this->_connecting = false;
		}};
	}

	void TitleScreen::_onInputsChosen()
	{
		if (this->_leftInput > 1)
			this->_P1.second->setJoystickId(this->_leftInput - 2);
		if (this->_rightInput > 1)
			this->_P2.second->setJoystickId(this->_rightInput - 2);
		switch (this->_selectedEntry) {
		case 0:
			this->_nextScene = new CharacterSelect(
				this->_leftInput  == 1 ? static_cast<std::shared_ptr<IInput>>(this->_P1.first) : static_cast<std::shared_ptr<IInput>>(this->_P1.second),
				this->_rightInput == 1 ? static_cast<std::shared_ptr<IInput>>(this->_P2.first) : static_cast<std::shared_ptr<IInput>>(this->_P2.second)
			);
			break;
		case 1:
			this->_host();
			break;
		case 2:
			this->_connect();
			break;
		case 5:
			game.networkMgr.setInputs(
				this->_leftInput  == 1 ? static_cast<std::shared_ptr<IInput>>(this->_P1.first) : static_cast<std::shared_ptr<IInput>>(this->_P1.second),
				this->_rightInput == 1 ? static_cast<std::shared_ptr<IInput>>(this->_P2.first) : static_cast<std::shared_ptr<IInput>>(this->_P2.second)
			);
			game.networkMgr.startSyncTest();
			break;
		}
	}

	void TitleScreen::_onKeyPressed(sf::Event::KeyEvent ev)
	{
		if (this->_changeInput && this->_changingInputs) {
			if (sf::Keyboard::Escape == ev.code) {
				this->_changeInput = false;
				return;
			}
			if (!this->_usingKeyboard)
				return;

			auto &pair = this->_changingInputs == 1 ? this->_P1 : this->_P2;

			pair.first->changeInput(static_cast<InputEnum>(this->_cursorInputs), ev.code);
			this->_changeInput = false;
			return;
		}

		this->_usingKeyboard = true;
		switch (ev.code) {
		case sf::Keyboard::Up:
			if (!this->_connecting && !game.networkMgr.isHosting())
				this->_onGoUp();
			break;
		case sf::Keyboard::Down:
			if (!this->_connecting && !game.networkMgr.isHosting())
				this->_onGoDown();
			break;
		case sf::Keyboard::Left:
			if (!this->_connecting && !game.networkMgr.isHosting())
				this->_onGoLeft();
			break;
		case sf::Keyboard::Right:
			if (!this->_connecting && !game.networkMgr.isHosting())
				this->_onGoRight();
			break;
		case sf::Keyboard::Escape:
		case sf::Keyboard::X:
			this->_onCancel();
			break;
		case sf::Keyboard::Z:
			if (!this->_connecting && !game.networkMgr.isHosting())
				this->_onConfirm();
			break;
		default:
			break;
		}
	}

	void TitleScreen::_onJoystickMoved(sf::Event::JoystickMoveEvent ev)
	{
		auto old = this->_oldStickValues[ev.joystickId][ev.axis];

		this->_oldStickValues[ev.joystickId][ev.axis] = ev.position;
		if (this->_changeInput && this->_changingInputs) {
			if (this->_usingKeyboard)
				return;

			auto &pair = this->_changingInputs == 1 ? this->_P1 : this->_P2;

			pair.second->changeInput(static_cast<InputEnum>(this->_cursorInputs), new ControllerAxis(ev.joystickId, ev.axis, std::copysign(30, ev.position)));
			this->_changeInput = false;
			return;
		}
		this->_usingKeyboard = false;

		switch (ev.axis) {
		case sf::Joystick::Axis::X:
		case sf::Joystick::Axis::PovX:
			if (ev.position < -THRESHOLD && old > -THRESHOLD)
				this->_onGoLeft();
			else if (ev.position > THRESHOLD && old < THRESHOLD)
				this->_onGoRight();
			return;
		case sf::Joystick::Axis::Y:
			if (ev.position < -THRESHOLD && old > -THRESHOLD)
				this->_onGoUp();
			else if (ev.position > THRESHOLD && old < THRESHOLD)
				this->_onGoDown();
			return;
		case sf::Joystick::Axis::PovY:
			if (ev.position < -THRESHOLD && old > -THRESHOLD)
				this->_onGoDown();
			else if (ev.position > THRESHOLD && old < THRESHOLD)
				this->_onGoUp();
			return;
		default:
			return;
		}
	}

	void TitleScreen::_onJoystickPressed(sf::Event::JoystickButtonEvent ev)
	{
		if (this->_changeInput && this->_changingInputs) {
			if (this->_usingKeyboard)
				return;

			auto &pair = this->_changingInputs == 1 ? this->_P1 : this->_P2;

			pair.second->changeInput(static_cast<InputEnum>(this->_cursorInputs), new ControllerButton(ev.joystickId, ev.button));
			this->_changeInput = false;
			return;
		}

		this->_usingKeyboard = false;
		if (this->_askingInputs && ev.button == 0) {
			if (this->_rightInput || (this->_leftInput && this->_selectedEntry != 0 && this->_selectedEntry != 5))
				this->_onInputsChosen();
			else if (this->_leftInput) {
				if (this->_leftInput != ev.joystickId + 2)
					this->_rightInput = ev.joystickId + 2;
			} else
				this->_leftInput = ev.joystickId + 2;
			return;
		}
		if (ev.button == 0)
			this->_onConfirm();
		else if (ev.button == 1)
			this->_onCancel();
	}

	void TitleScreen::_showAskInputBox() const
	{
		game.screen->displayElement({540, 180, 600, 300}, sf::Color{0x50, 0x50, 0x50});

		game.screen->fillColor(this->_leftInput ? sf::Color::Green : sf::Color::White);
		game.screen->displayElement("P1", {540 + 120, 190});
		game.screen->fillColor(sf::Color::White);
		if (this->_leftInput)
			game.screen->displayElement(
				this->_leftInput == 1 ?
				this->_P1.first->getName() :
				this->_P1.second->getName() + " #" + std::to_string(this->_leftInput - 1),
				{540, 260},
				300,
				Screen::ALIGN_CENTER
			);
		else
			game.screen->displayElement("Press Z or (A)", {540, 260}, 300, Screen::ALIGN_CENTER);

		if (this->_selectedEntry == 0 || this->_selectedEntry == 5)
			game.screen->fillColor(this->_rightInput ? sf::Color::Green : (this->_leftInput ? sf::Color::White : sf::Color{0xA0, 0xA0, 0xA0}));
		else
			game.screen->fillColor(sf::Color{0x80, 0x80, 0x80});
		game.screen->displayElement("P2", {540 + 420, 190});
		game.screen->fillColor(sf::Color::White);
		if (this->_leftInput && (this->_selectedEntry == 0 || this->_selectedEntry == 5)) {
			if (this->_rightInput)
				game.screen->displayElement(
					this->_rightInput == 1 ?
					this->_P2.first->getName() :
					this->_P2.second->getName() + " #" + std::to_string(this->_rightInput - 1),
					{840, 260},
					300,
					Screen::ALIGN_CENTER
				);
			else
				game.screen->displayElement("Press Z or (A)", {840, 260}, 300, Screen::ALIGN_CENTER);
		}

		if (this->_leftInput && (this->_rightInput || (this->_selectedEntry != 0 && this->_selectedEntry != 5)))
			game.screen->displayElement("Press Z or (A) to confirm", {540, 360}, 600, Screen::ALIGN_CENTER);
	}

	void TitleScreen::_showHostMessage() const
	{
		game.screen->displayElement({640, 280, 400, 100}, sf::Color{0x50, 0x50, 0x50});

		game.screen->fillColor(sf::Color::White);
		game.screen->displayElement("Hosting on port 10800", {640, 300}, 400, Screen::ALIGN_CENTER);
	}

	void TitleScreen::_showConnectMessage() const
	{
		game.screen->displayElement({540, 280, 600, 100}, sf::Color{0x50, 0x50, 0x50});

		game.screen->fillColor(sf::Color::White);
		game.screen->displayElement("Connecting to " + game.lastIp + " on port 10800", {540, 300}, 600, Screen::ALIGN_CENTER);
	}

	void TitleScreen::_showEditKeysMenu() const
	{
		auto &pair = this->_changingInputs == 1 ? this->_P1 : this->_P2;
		auto input = this->_usingKeyboard ? static_cast<std::shared_ptr<IInput>>(pair.first) : static_cast<std::shared_ptr<IInput>>(pair.second);
		auto strs = input->getKeyNames();

		game.screen->displayElement({640, 80, 400, 830}, sf::Color{0x50, 0x50, 0x50});

		game.screen->fillColor(sf::Color::White);
		game.screen->displayElement((this->_changingInputs == 1 ? "P1 | " : "P2 | ") + input->getName(), {640, 85}, 400, Screen::ALIGN_CENTER);

		game.screen->fillColor(sf::Color::White);

		for (unsigned i = 0; i < this->_inputs.size(); i++) {
			game.screen->displayElement(this->_inputs[i], {680, 135 + i * 68.f});
			if (this->_changeInput && this->_cursorInputs == i) {
				game.screen->fillColor(sf::Color{0xFF, 0x80, 0x00});
				game.screen->displayElement("Press a key", {760, 146 + i * 68.f});
			} else {
				game.screen->fillColor(this->_cursorInputs == i ? sf::Color::Red : sf::Color::White);
				game.screen->displayElement(strs[i], {760, 146 + i * 68.f});
			}
		}
	}

	void TitleScreen::_onGoUp()
	{
		if (this->_changingInputs) {
			this->_cursorInputs += this->_inputs.size();
			this->_cursorInputs--;
			this->_cursorInputs %= this->_inputs.size();
			return;
		}
		if (this->_askingInputs)
			return;
		this->_selectedEntry += sizeof(menuItems) / sizeof(*menuItems);
		this->_selectedEntry--;
		this->_selectedEntry %= sizeof(menuItems) / sizeof(*menuItems);
	}

	void TitleScreen::_onGoDown()
	{
		if (this->_changingInputs) {
			this->_cursorInputs++;
			this->_cursorInputs %= this->_inputs.size();
			return;
		}
		if (this->_askingInputs)
			return;
		this->_selectedEntry++;
		this->_selectedEntry %= sizeof(menuItems) / sizeof(*menuItems);
	}

	void TitleScreen::_onGoLeft()
	{
		if (this->_changingInputs) {
			this->_changingInputs = this->_changingInputs == 1 ? 2 : 1;
			return;
		}
	}

	void TitleScreen::_onGoRight()
	{
		if (this->_changingInputs) {
			this->_changingInputs = this->_changingInputs == 1 ? 2 : 1;
			return;
		}
	}

	void TitleScreen::_onConfirm()
	{
		if (this->_changingInputs) {
			this->_changeInput = true;
			return;
		}
		if (this->_askingInputs) {
			if (this->_rightInput || (this->_leftInput && (this->_selectedEntry != 0 && this->_selectedEntry != 5)))
				this->_onInputsChosen();
			else if (this->_leftInput)
				this->_rightInput = 1;
			else
				this->_leftInput = 1;
			return;
		}

		switch (this->_selectedEntry) {
		case 0:
		case 1:
		case 2:
			this->_askingInputs = true;
			break;
		case 3:
			this->_changingInputs = 1;
			this->_cursorInputs = 0;
			break;
		case 4:
			game.screen->close();
			break;
		case 5:
			this->_askingInputs = true;
			break;
		default:
			break;
		}
	}

	void TitleScreen::_onCancel()
	{
		if (this->_connecting || game.networkMgr.isHosting()) {
			this->_connecting = false;
			game.networkMgr.cancelHost();
			return;
		}
		if (this->_changingInputs) {
			this->_changingInputs = 0;
			return;
		}
		if (this->_askingInputs) {
			if (this->_rightInput && (this->_selectedEntry == 0 || this->_selectedEntry == 5))
				this->_rightInput = 0;
			else if (this->_leftInput)
				this->_leftInput = 0;
			else
				this->_askingInputs = false;
			return;
		}
		this->_selectedEntry = 4;
	}
}