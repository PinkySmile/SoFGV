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
#include "KeyboardInput.hpp"
#include "ControllerInput.hpp"
#include "Resources/SceneArgument.hpp"
#ifdef HAS_NETWORK
#include "Resources/Network/Connection.hpp"
#endif

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
		std::string _basePath;
		std::vector<std::pair<bool, std::string>> _replays;
		unsigned char _changingInputs = 0;
		unsigned char _cursorInputs = 0;
		unsigned char _errorTimer = 0;
	#ifdef HAS_NETWORK
		unsigned _hostingPort = 10800;
		bool _connected = false;
		bool _chooseSpecCount = false;
		bool _specEnabled = true;
	#endif
		IInput *_lastInput;
		std::string _remote;
		std::string _oldRemote;
		std::string _errorMsg;
		std::function<void ()> onDestruct;

		void _loadReplay(const std::filesystem::path &path);
		void _fetchReplayList();
		void _onInputsChosen();
	#ifdef HAS_NETWORK
		void _host(bool spec);
		void _connect();
	#endif
		bool _onKeyPressed(sf::Event::KeyEvent ev);
		bool _onJoystickMoved(sf::Event::JoystickMoveEvent ev);
		bool _onJoystickPressed(sf::Event::JoystickButtonEvent ev);
		void _showAskInputBox() const;
		void _showEditKeysMenu() const;
	#ifdef HAS_NETWORK
		void _showHostMessage() const;
		void _showConnectMessage() const;
		void _showChooseSpecCount() const;
	#endif
		void _onGoUp();
		void _onGoDown();
		void _onGoLeft();
		void _onGoRight();
		void _onCancel();
		void _onConfirm(unsigned stickId);
#ifdef HAS_NETWORK
		void _onDisconnect(const std::string &address);
		void _onConnect(const std::string &address);
		void _pingUpdate(unsigned ping);
		void _specUpdate(std::pair<unsigned, unsigned> spec);
#endif

		static std::shared_ptr<IInput> _getInputFromId(unsigned id, const std::pair<std::shared_ptr<KeyboardInput>, std::shared_ptr<ControllerInput>> &pair);

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
