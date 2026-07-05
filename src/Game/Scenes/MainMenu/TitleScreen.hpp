//
// Created by PinkySmile on 24/09/2021.
//

#ifndef SOFGV_TITLESCREEN_HPP
#define SOFGV_TITLESCREEN_HPP


#include <map>
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
		std::unique_ptr<Menu> _menuObject;
		std::map<unsigned, std::map<sf::Joystick::Axis, int>> _oldStickValues;
		std::pair<unsigned, unsigned> _spec;
		unsigned _latestJoystickId = 0;
		unsigned _leftInput = 0;
		unsigned _rightInput = 0;
		unsigned _timer = 0;
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
		unsigned _hostingPort;
		bool _connected = false;
		bool _chooseSpecCount = false;
		bool _specEnabled = true;
	#ifdef __EMSCRIPTEN__
		bool _selectingRoom = false;
		std::string _typingCode;
	#endif
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
		std::optional<std::pair<sf::IpAddress, unsigned short>> _getIPPort(const std::string &ip) const;
		void _host(bool spec);
		void _connect(const std::string &ip);
		void _spectate(const std::string &ip);
	#endif
		bool _onKeyPressed(const sf::Event::KeyPressed &ev);
		bool _onJoystickMoved(const sf::Event::JoystickMoved &ev);
		bool _onJoystickPressed(const sf::Event::JoystickButtonPressed &ev);
		void _showAskInputBox() const;
		void _showEditKeysMenu() const;
	#ifdef HAS_NETWORK
		void _showHostMessage() const;
		void _showConnectMessage() const;
		void _showChooseSpecCount() const;
		void _showSelectingRoom() const;
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
