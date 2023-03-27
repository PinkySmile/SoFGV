//
// Created by PinkySmile on 24/09/2021.
//

#ifndef SOFGV_TITLESCREEN_HPP
#define SOFGV_TITLESCREEN_HPP


#include <SFML/Graphics/Font.hpp>
#include <memory>
#include <thread>
#include <filesystem>
#include "Resources/IScene.hpp"
#include "Menu.hpp"
#include "Inputs/KeyboardInput.hpp"
#include "Inputs/ControllerInput.hpp"
#include "Resources/Network/Connection.hpp"
#include "Resources/SceneArgument.hpp"

namespace SpiralOfFate
{
	class TitleScreen : public IScene {
	private:
		mutable Sprite _titleBg;
		mutable Sprite _titleLogo;
		mutable Sprite _titleSpiral;
		mutable std::vector<Sprite> _inputs;
		Menu _menuObject;
		std::thread _thread;
		std::map<unsigned, std::map<sf::Joystick::Axis, int>> _oldStickValues;
		std::pair<unsigned, unsigned> _spec;
		unsigned _latestJoystickId = 0;
		unsigned _leftInput = 0;
		unsigned _rightInput = 0;
		float _totalPing = 0;
		unsigned _nbPings = 0;
		unsigned _lastPing = 0;
		unsigned _peakPing = 0;
		unsigned _delay = 0;
		unsigned _netbellSound;
		bool _connecting = false;
		bool _changeInput = false;
		bool _askingInputs = false;
		bool _chooseSpecCount = false;
		bool _replaySelect = false;
		bool _specEnabled = true;
		std::string _basePath;
		std::vector<std::pair<bool, std::string>> _replays;
		unsigned char _changingInputs = 0;
		unsigned char _cursorInputs = 0;
		unsigned char _errorTimer = 0;
		bool _connected = false;
		std::string _remote;
		std::string _oldRemote;
		std::string _errorMsg;
		std::function<void ()> onDestruct;

		void _loadReplay(const std::filesystem::path &path);
		void _fetchReplayList();
		void _onInputsChosen();
		void _host(bool spec);
		void _connect();
		bool _onKeyPressed(sf::Event::KeyEvent ev);
		bool _onJoystickMoved(sf::Event::JoystickMoveEvent ev);
		bool _onJoystickPressed(sf::Event::JoystickButtonEvent ev);
		void _showAskInputBox() const;
		void _showHostMessage() const;
		void _showEditKeysMenu() const;
		void _showConnectMessage() const;
		void _showChooseSpecCount() const;
		void _onGoUp();
		void _onGoDown();
		void _onGoLeft();
		void _onGoRight();
		void _onCancel();
		void _onConfirm(unsigned stickId);
		void _onDisconnect(const std::string &address);
		void _onConnect(const std::string &address);
		void _pingUpdate(unsigned ping);
		void _specUpdate(std::pair<unsigned, unsigned> spec);

	public:
		TitleScreen();
		~TitleScreen();
		void render() const override;
		void update() override;
		void consumeEvent(const sf::Event &event) override;

		static TitleScreen *create(SceneArguments *);
	};
}


#endif //SOFGV_TITLESCREEN_HPP
