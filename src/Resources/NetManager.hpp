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
#include "../Scenes/NetplayInGame.hpp"
#include "../Scenes/NetplayCharacterSelect.hpp"

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
		enum Opcode {
			OPCODE_HELLO,
			OPCODE_OLLEH,
			OPCODE_ERROR,
			OPCODE_WAIT,
			OPCODE_START,
			OPCODE_QUIT,
			OPCODE_GAME_START,
			OPCODE_PING,
			OPCODE_PONG,
		};

		enum ErrorCode {
			ERROR_VERSION_MISMATCH,
			ERROR_BAD_PACKET,
			ERROR_INVALID_OPCODE,
			ERROR_UNEXPECTED_OPCODE,
		};

		union Packet {
			Opcode op;
			struct {
				Opcode _1;
				char versionString[32];
			};
			struct {
				Opcode _2;
				ErrorCode error;
			};
			struct {
				Opcode _3;
				unsigned int currentSpecs;
				unsigned int maxSpec;
			};
			struct {
				Opcode _4;
				unsigned pingId;
			};
			struct {
				Opcode _5;
				bool spectator;
			};
			struct {
				Opcode _6;
				bool delay;
			};
		};

#pragma pack(push, 1)
		struct Data {
			bool isCharSelect;
			int leftInputs[INPUT_NUMBER - 1];
			int rightInputs[INPUT_NUMBER - 1];
		};
#pragma pack(pop)

		unsigned _myPlayer = 0;
		volatile bool _host = false;
		volatile bool _connect = false;
		volatile unsigned _delay = 0;
		unsigned _timer = 0;
		sf::Texture _interruptedLogo;
		sf::Sprite _interruptedSprite;
		bool _interrupted = false;
		NetplayScene *_netScene;
		GGPOSession *_ggpoSession;
		std::shared_ptr<IScene> _scene;
		std::shared_ptr<NetplayInGame> _inGame;
		std::shared_ptr<NetplayCharacterSelect> _characterSelect;
		GGPOPlayerHandle _playerHandles[2];
		std::shared_ptr<IInput> _leftRealInput;
		std::shared_ptr<IInput> _rightRealInput;

		void _initGGPO(unsigned short port, unsigned int spectators);
		void _initGGPOSyncTest();

		static void _checkPacket(const Packet &, size_t size);

	public:
		NetManager();

		std::shared_ptr<RemoteInput> _leftInput;
		std::shared_ptr<RemoteInput> _rightInput;

		void startSyncTest();
		void consumeEvent(const sf::Event &event);
		void consumeEvent(GGPOEvent *event);
		bool isConnected() const;
		void beginSession();
		void setInputs(std::shared_ptr<IInput> left, std::shared_ptr<IInput> right);
		bool spectate(
			const std::string &address,
			unsigned short port,
			const std::function<void (const sf::IpAddress &, unsigned short)> &onConnect,
			const std::function<void (unsigned, unsigned)> &spectatorUpdate
		);
		bool connect(
			const std::string &address,
			unsigned short port,
			const std::function<void (const sf::IpAddress &, unsigned short)> &onConnect,
			const std::function<void (unsigned, unsigned)> &spectatorUpdate
		);
		void host(
			unsigned short port,
			unsigned int spectators,
			const std::function<void (const sf::IpAddress &, unsigned short)> &onDisconnect,
			const std::function<void (const sf::IpAddress &, unsigned short)> &onConnect,
			const std::function<void (unsigned)> &pingUpdate
		);
		void advanceState();
		void nextFrame();
		void endSession();
		void setDelay(unsigned delay);
		void save(void **buffer, int *len);
		void load(void *buffer);
		void update();
		void renderHUD();
		void postRender();
		void cancelHost();
		bool isHosting();
		bool isConnecting();

		static void free(void *buffer);
	};
}


#endif //BATTLE_NETMANAGER_HPP
