//
// Created by Gegel85 on 24/09/2021.
//

#ifdef _WIN32
#include <windows.h>

#include <utility>
#else
// TODO: Add proper message boxes on non windows systems
#define MessageBox(...)
#endif
#include "TitleScreen.hpp"
#include "../Resources/Game.hpp"
#include "InGame.hpp"
#include "../Logger.hpp"
#include "../Inputs/KeyboardInput.hpp"
#include "../Inputs/ControllerInput.hpp"
#include "../Inputs/RemoteInput.hpp"
#include "../Inputs/NetworkInput.hpp"
#include "NetplayGame.hpp"
#include "CharacterSelect.hpp"

#define THRESHOLD 50

namespace Battle
{
	TitleScreen::TitleScreen(
		std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> P1,
		std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> P2
	) :
		_P1(std::move(P1)),
		_P2(std::move(P2))
	{
		logger.info("Title scene created");
		this->_font.loadFromFile(getenv("SYSTEMROOT") + std::string("\\Fonts\\comic.ttf"));
	}

	void TitleScreen::render() const
	{
		game.screen->setFont(this->_font);

		game.screen->fillColor(this->_selectedEntry == 0 ? sf::Color::Red : sf::Color::Black);
		game.screen->displayElement("Play", {100, 100});

		game.screen->fillColor(this->_selectedEntry == 1 ? sf::Color::Red : sf::Color::Black);
		game.screen->displayElement("Host", {100, 140});

		game.screen->fillColor(this->_selectedEntry == 2 ? sf::Color::Red : sf::Color::Black);
		game.screen->displayElement("Connect", {100, 180});

		game.screen->fillColor(this->_selectedEntry == 3 ? sf::Color::Red : sf::Color::Black);
		game.screen->displayElement("Settings", {100, 220});

		game.screen->fillColor(this->_selectedEntry == 4 ? sf::Color::Red : sf::Color::Black);
		game.screen->displayElement("Quit", {100, 260});

		if (this->_askingInputs)
			this->_showAskInputBox();
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
		auto remote = new RemoteInput();
		auto networkInput = new NetworkInput(*remote, this->_leftInput ? static_cast<std::shared_ptr<IInput>>(this->_P1.first) : static_cast<std::shared_ptr<IInput>>(this->_P1.second));

		try {
			remote->host(10800);
		} catch (std::exception &e) {
			delete remote;
			delete networkInput;
			MessageBox(nullptr, "Host error", e.what(), MB_ICONERROR);
			return;
		}
		this->_nextScene = new NetplayGame(remote, networkInput, networkInput, remote);
	}

	void TitleScreen::_connect()
	{
		auto remote = new RemoteInput();
		auto networkInput = new NetworkInput(*remote, this->_leftInput ? static_cast<std::shared_ptr<IInput>>(this->_P1.first) : static_cast<std::shared_ptr<IInput>>(this->_P1.second));

		try {
			std::ifstream stream{"ip.txt"};
			std::string line;

			if (stream.fail())
				throw std::invalid_argument("Cannot read ip.txt");
			std::getline(stream, line);
			if (stream.fail())
				throw std::invalid_argument("ip.txt is empty");
			remote->connect(line, 10800);
		} catch (std::exception &e) {
			delete remote;
			delete networkInput;
			MessageBox(nullptr, "Connect error", e.what(), MB_ICONERROR);
			return;
		}
		this->_nextScene = new NetplayGame(remote, networkInput, remote, networkInput);
	}

	void TitleScreen::_onConfirm()
	{
		switch (this->_selectedEntry) {
		case 0:
		case 1:
		case 2:
			this->_askingInputs = true;
			break;
		case 3:
			MessageBox(nullptr, "Not implemented", "Not implemented", MB_ICONERROR);
			break;
		case 4:
			game.screen->close();
			break;
		default:
			break;
		}
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
		}
	}

	void TitleScreen::_onKeyPressed(sf::Event::KeyEvent ev)
	{
		this->_usingKeyboard = true;
		if (this->_askingInputs) {
			if (ev.code == sf::Keyboard::Escape) {
				if (this->_rightInput && this->_selectedEntry == 0)
					this->_rightInput = 0;
				else if (this->_leftInput)
					this->_leftInput = 0;
				else
					this->_askingInputs = false;
			} else if (ev.code == sf::Keyboard::Z) {
				if (this->_rightInput || (this->_leftInput && this->_selectedEntry != 0))
					this->_onInputsChosen();
				else if (this->_leftInput)
					this->_rightInput = 1;
				else
					this->_leftInput = 1;
			}
			return;
		}
		switch (ev.code) {
		case sf::Keyboard::Up:
			this->_onGoUp();
			break;
		case sf::Keyboard::Down:
			this->_onGoDown();
			break;
		case sf::Keyboard::X:
			this->_selectedEntry = 4;
			break;
		case sf::Keyboard::Z:
			this->_onConfirm();
			break;
		default:
			break;
		}
	}

	void TitleScreen::_onJoystickMoved(sf::Event::JoystickMoveEvent ev)
	{
		auto old = this->_oldStickValues[ev.joystickId][ev.axis];

		this->_usingKeyboard = false;
		this->_oldStickValues[ev.joystickId][ev.axis] = ev.position;
		switch (ev.axis) {
		case sf::Joystick::Axis::X:
		case sf::Joystick::Axis::PovX:
			if (ev.position < -THRESHOLD && old > -THRESHOLD)
				this->_onGoLeft();
			else if (ev.position > THRESHOLD && old < THRESHOLD)
				this->_onGoRight();
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
		this->_usingKeyboard = false;
		if (this->_askingInputs) {
			if (ev.button == 1) {
				if (this->_rightInput && this->_selectedEntry == 0)
					this->_rightInput = 0;
				else if (this->_leftInput)
					this->_leftInput = 0;
				else
					this->_askingInputs = false;
			} else if (ev.button == 0) {
				if (this->_rightInput || (this->_leftInput && this->_selectedEntry != 0))
					this->_onInputsChosen();
				else if (this->_leftInput) {
					if (this->_leftInput != ev.joystickId + 2)
						this->_rightInput = ev.joystickId + 2;
				} else
					this->_leftInput = ev.joystickId + 2;
			}
			return;
		}
		if (ev.button == 0)
			this->_onConfirm();
		else if (ev.button == 1)
			this->_selectedEntry = 4;
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
				{540 + 70, 260}
			);
		else
			game.screen->displayElement("Press Z or (A)", {540 + 70, 260});

		if (this->_selectedEntry == 0)
			game.screen->fillColor(this->_rightInput ? sf::Color::Green : (this->_leftInput ? sf::Color::White : sf::Color{0xA0, 0xA0, 0xA0}));
		else
			game.screen->fillColor(sf::Color{0x80, 0x80, 0x80});
		game.screen->displayElement("P2", {540 + 420, 190});
		game.screen->fillColor(sf::Color::White);
		if (this->_leftInput && this->_selectedEntry == 0) {
			if (this->_rightInput)
				game.screen->displayElement(
					this->_rightInput == 1 ?
					this->_P2.first->getName() :
					this->_P2.second->getName() + " #" + std::to_string(this->_rightInput - 1),
					{540 + 370, 260}
				);
			else
				game.screen->displayElement("Press Z or (A)", {540 + 370, 260});
		}

		if (this->_leftInput && (this->_rightInput || this->_selectedEntry != 0))
			game.screen->displayElement("Press Z or (A) to confirm", {540 + 120, 360});
	}

	void TitleScreen::_showEditKeysMenu() const
	{
		game.screen->displayElement({540, 180, 600, 600}, sf::Color{0x50, 0x50, 0x50});

		game.screen->fillColor(this->_leftInput ? sf::Color::Green : sf::Color::White);
		game.screen->displayElement("P1", {540 + 120, 190});
		game.screen->fillColor(sf::Color::White);
		if (this->_leftInput)
			game.screen->displayElement(
				this->_leftInput == 1 ?
				this->_P1.first->getName() :
				this->_P1.second->getName() + " #" + std::to_string(this->_leftInput - 1),
				{540 + 70, 260}
			);
		else
			game.screen->displayElement("Press Z or (A)", {540 + 70, 260});

		if (this->_selectedEntry == 0)
			game.screen->fillColor(this->_rightInput ? sf::Color::Green : (this->_leftInput ? sf::Color::White : sf::Color{0xA0, 0xA0, 0xA0}));
		else
			game.screen->fillColor(sf::Color{0x80, 0x80, 0x80});
		game.screen->displayElement("P2", {540 + 420, 190});
		game.screen->fillColor(sf::Color::White);
		if (this->_leftInput && this->_selectedEntry == 0) {
			if (this->_rightInput)
				game.screen->displayElement(
					this->_rightInput == 1 ?
					this->_P2.first->getName() :
					this->_P2.second->getName() + " #" + std::to_string(this->_rightInput - 1),
					{540 + 370, 260}
				);
			else
				game.screen->displayElement("Press Z or (A)", {540 + 370, 260});
		}

		if (this->_leftInput && (this->_rightInput || this->_selectedEntry != 0))
			game.screen->displayElement("Press Z or (A) to confirm", {540 + 120, 360});
	}

	void TitleScreen::_onGoUp()
	{
		this->_selectedEntry += 5;
		this->_selectedEntry--;
		this->_selectedEntry %= 5;
	}

	void TitleScreen::_onGoDown()
	{
		this->_selectedEntry++;
		this->_selectedEntry %= 5;
	}

	void TitleScreen::_onGoLeft()
	{

	}

	void TitleScreen::_onGoRight()
	{

	}

	void TitleScreen::_onCancel()
	{

	}
}