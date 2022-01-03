//
// Created by Gegel85 on 24/09/2021.
//

#ifdef _WIN32
#include <windows.h>
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

namespace Battle
{
	TitleScreen::TitleScreen()
	{
		logger.info("Title scene created");
		this->_font.loadFromFile("assets/fonts/comicsansms.ttf");
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
			switch (event.key.code) {
			case sf::Keyboard::Up:
				this->_selectedEntry += 5;
				this->_selectedEntry--;
				this->_selectedEntry %= 5;
				break;
			case sf::Keyboard::Down:
				this->_selectedEntry++;
				this->_selectedEntry %= 5;
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
			break;
		default:
			break;
		}
	}

	void TitleScreen::_host()
	{
		auto remote = new RemoteInput();
		auto networkInput = new NetworkInput(*remote, new KeyboardInput({
			{ sf::Keyboard::Left,  INPUT_LEFT },
			{ sf::Keyboard::Right, INPUT_RIGHT },
			{ sf::Keyboard::Up,    INPUT_UP },
			{ sf::Keyboard::Down,  INPUT_DOWN },
			{ sf::Keyboard::W,     INPUT_NEUTRAL },
			{ sf::Keyboard::X,     INPUT_MATTER },
			{ sf::Keyboard::C,     INPUT_SPIRIT },
			{ sf::Keyboard::Q,     INPUT_VOID },
			{ sf::Keyboard::S,     INPUT_ASCEND },
			{ sf::Keyboard::D,     INPUT_DASH }
		}));
		//auto networkInput = new NetworkInput(*remote, new ControllerInput({
		//	{ INPUT_LEFT,    new ControllerAxis(0, sf::Joystick::Axis::X, -30) },
		//	{ INPUT_RIGHT,   new ControllerAxis(0, sf::Joystick::Axis::X, 30) },
		//	{ INPUT_UP,      new ControllerAxis(0, sf::Joystick::Axis::Y, -30) },
		//	{ INPUT_DOWN,    new ControllerAxis(0, sf::Joystick::Axis::Y, 30) },
		//	{ INPUT_NEUTRAL, new ControllerButton(0, 0) },
		//	{ INPUT_MATTER,  new ControllerButton(0, 2) },
		//	{ INPUT_SPIRIT,  new ControllerButton(0, 1) },
		//	{ INPUT_VOID,    new ControllerButton(0, 3) },
		//	{ INPUT_ASCEND,  new ControllerButton(0, 5) },
		//	{ INPUT_DASH,    new ControllerAxis(0, sf::Joystick::Z, -30) }
		//}));

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
		auto networkInput = new NetworkInput(*remote, new ControllerInput({
			{ INPUT_LEFT,    new ControllerAxis(0, sf::Joystick::Axis::X, -30) },
			{ INPUT_RIGHT,   new ControllerAxis(0, sf::Joystick::Axis::X, 30) },
			{ INPUT_UP,      new ControllerAxis(0, sf::Joystick::Axis::Y, -30) },
			{ INPUT_DOWN,    new ControllerAxis(0, sf::Joystick::Axis::Y, 30) },
			{ INPUT_NEUTRAL, new ControllerButton(0, 0) },
			{ INPUT_MATTER,  new ControllerButton(0, 2) },
			{ INPUT_SPIRIT,  new ControllerButton(0, 1) },
			{ INPUT_VOID,    new ControllerButton(0, 3) },
			{ INPUT_ASCEND,  new ControllerButton(0, 5) },
			{ INPUT_DASH,    new ControllerAxis(0, sf::Joystick::Z, -30) }
		}));

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
			this->_nextScene = new CharacterSelect(
				std::make_shared<ControllerInput>(std::map<InputEnum, ControllerKey *>{
					{ INPUT_LEFT,    new ControllerAxis(0, sf::Joystick::Axis::PovX, -30) },
					{ INPUT_RIGHT,   new ControllerAxis(0, sf::Joystick::Axis::PovX, 30) },
					{ INPUT_UP,      new ControllerAxis(0, sf::Joystick::Axis::PovY, 30) },
					{ INPUT_DOWN,    new ControllerAxis(0, sf::Joystick::Axis::PovY, -30) },
					{ INPUT_NEUTRAL, new ControllerButton(0, 0) },
					{ INPUT_MATTER,  new ControllerButton(0, 2) },
					{ INPUT_SPIRIT,  new ControllerButton(0, 1) },
					{ INPUT_VOID,    new ControllerButton(0, 3) },
					{ INPUT_ASCEND,  new ControllerButton(0, 5) },
					{ INPUT_DASH,    new ControllerAxis(0, sf::Joystick::Z, -30) }
				}),
				std::make_shared<ControllerInput>(std::map<InputEnum, ControllerKey *>{
					{ INPUT_LEFT,    new ControllerAxis(1, sf::Joystick::Axis::X, -30) },
					{ INPUT_RIGHT,   new ControllerAxis(1, sf::Joystick::Axis::X, 30) },
					{ INPUT_UP,      new ControllerAxis(1, sf::Joystick::Axis::Y, -30) },
					{ INPUT_DOWN,    new ControllerAxis(1, sf::Joystick::Axis::Y, 30) },
					{ INPUT_NEUTRAL, new ControllerButton(1, 2) },
					{ INPUT_MATTER,  new ControllerButton(1, 3) },
					{ INPUT_SPIRIT,  new ControllerAxis(1, sf::Joystick::Z, 30) },
					{ INPUT_VOID,    new ControllerAxis(1, sf::Joystick::Z, -30) },
					{ INPUT_ASCEND,  new ControllerButton(1, 0) },
					{ INPUT_DASH,    new ControllerButton(1, 1) }
				})
			);
			break;
		case 1:
			this->_host();
			break;
		case 2:
			this->_connect();
			break;
		case 3:
			break;
		case 4:
			game.screen->close();
			break;
		default:
			break;
		}
	}
}