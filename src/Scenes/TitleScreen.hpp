//
// Created by Gegel85 on 24/09/2021.
//

#ifndef BATTLE_TITLESCREEN_HPP
#define BATTLE_TITLESCREEN_HPP


#include <SFML/Graphics/Font.hpp>
#include <memory>
#include <thread>
#include "IScene.hpp"
#include "../Inputs/KeyboardInput.hpp"
#include "../Inputs/ControllerInput.hpp"

namespace Battle
{
	class TitleScreen : public IScene {
	private:
		std::vector<sf::Texture> _inputs;
		std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> _P1;
		std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> _P2;
		std::thread _thread;
		std::map<unsigned, std::map<sf::Joystick::Axis, int>> _oldStickValues;
		unsigned _selectedEntry = 0;
		unsigned _leftInput = 0;
		unsigned _rightInput = 0;
		bool _connecting = false;
		bool _changeInput = false;
		bool _askingInputs = false;
		bool _usingKeyboard = false;
		unsigned char _changingInputs = 0;
		unsigned char _cursorInputs = 0;
		IScene *_nextScene = nullptr;

		void _onInputsChosen();
		void _host();
		void _connect();
		void _onKeyPressed(sf::Event::KeyEvent ev);
		void _onJoystickMoved(sf::Event::JoystickMoveEvent ev);
		void _onJoystickPressed(sf::Event::JoystickButtonEvent ev);
		void _showAskInputBox() const;
		void _showHostMessage() const;
		void _showEditKeysMenu() const;
		void _showConnectMessage() const;
		void _onGoUp();
		void _onGoDown();
		void _onGoLeft();
		void _onGoRight();
		void _onCancel();
		void _onConfirm();

	public:
		TitleScreen(
			std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> P1,
			std::pair<std::shared_ptr<Battle::KeyboardInput>, std::shared_ptr<Battle::ControllerInput>> P2
		);
		~TitleScreen();
		void render() const override;
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_TITLESCREEN_HPP
