//
// Created by Gegel85 on 07/02/2022.
//

#ifndef BATTLE_NETMANAGER_HPP
#define BATTLE_NETMANAGER_HPP


#include <memory>
#include <SFML/Graphics.hpp>
#include "../Inputs/RemoteInput.hpp"
#include "../Scenes/NetplayScene.hpp"
#include "../Scenes/IScene.hpp"

namespace Battle
{
	class GGPOError : public std::exception {
	private:
		 GGPOErrorCode _code;

	public:
		GGPOError(GGPOErrorCode code);
		const char *what() const noexcept;
	};

	class NetManager {
	private:
		struct Data {
			bool isCharSelect;
			std::array<int, INPUT_NUMBER - 1> leftInputs;
			std::array<int, INPUT_NUMBER - 1> rightInputs;
		};

		unsigned _timer = 0;
		sf::Texture _interruptedLogo;
		sf::Sprite _interruptedSprite;
		bool _interrupted = false;
		NetplayScene *_netScene;
		GGPOSession *_ggpoSession;
		std::shared_ptr<IScene> _scene;
		GGPOPlayerHandle _playerHandles[2];
		std::shared_ptr<IInput> _leftRealInput;
		std::shared_ptr<IInput> _rightRealInput;

		void _initGGPO(unsigned short port, unsigned int spectators);

	public:
		NetManager();

		std::shared_ptr<RemoteInput> _leftInput;
		std::shared_ptr<RemoteInput> _rightInput;

		void consumeEvent(const sf::Event &event);
		void consumeEvent(GGPOEvent *event);
		bool isConnected() const;
		void beginSession();
		void setInputs(std::shared_ptr<IInput> left, std::shared_ptr<IInput> right);
		bool spectate(const std::string &address, unsigned short port);
		bool connect(const std::string &address, unsigned short port);
		void host(unsigned short port, unsigned int spectators);
		void advanceState();
		void nextFrame();
		void endSession();
		void save(void **buffer, int *len);
		void load(void *buffer);
		void update();
		void renderHUD();
		void postRender();
		static void free(void *buffer);
	};
}


#endif //BATTLE_NETMANAGER_HPP
