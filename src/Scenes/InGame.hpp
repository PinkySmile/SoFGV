//
// Created by Gegel85 on 24/09/2021.
//

#ifndef BATTLE_INGAME_HPP
#define BATTLE_INGAME_HPP


#include <memory>
#include <random>
#include "IScene.hpp"
#include "../Inputs/IInput.hpp"
#include "../Objects/Character.hpp"
#include "../Resources/MoveListData.hpp"
#include "../Objects/FakeObject.hpp"

namespace Battle
{
	class InGame : public IScene {
	public:
		struct GameParams {
			unsigned stage;
			unsigned music;
			unsigned platforms;
		};

	protected:
		constexpr static const char *_menuStrings[] = {
			"Resume",
			"Move list",
			"Command list",
			"Return to character select",
			"Return to title screen"
		};
		const std::map<unsigned, std::vector<std::vector<FrameData>>> *_moveList = nullptr;
		std::map<unsigned, MoveData> _moveData;
		std::map<unsigned, MoveData> _leftMoveData;
		std::map<unsigned, MoveData> _rightMoveData;
		std::unique_ptr<FakeObject> _moveListObject;
		std::string _moveListName;
		unsigned int _moveListCursorMax = 0;
		unsigned int _moveListCursor = 0;
		unsigned int _moveListTop = 0;
		unsigned int _moveListTimer = 0;
		unsigned char _paused = 0;
		unsigned char _pauseCursor = 0;
		IScene *_nextScene = nullptr;
		std::array<sf::Texture, NB_SPRITES> _moveSprites;
		std::vector<unsigned> _moveOrder;
		std::vector<unsigned> _moveDisplayed;
		bool _goBackToTitle;

		std::mt19937 _random;
		struct GameParams _params;

		virtual void _renderPause() const;
		virtual void _pauseUpdate();
		virtual bool _pauseConfirm();
		void _renderMoveList() const;
		void _moveListUpdate();
		void _calculateMoveListOrder();

		InGame(const GameParams &params);

	public:
		InGame(const GameParams &params, const std::vector<struct PlatformSkeleton> &platforms, const struct StageEntry &stage, Character *leftChr, Character *rightChr, const nlohmann::json &lJson, const nlohmann::json &rJson, bool goBackToTitle = false);
		~InGame();
		void render() const override;
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_INGAME_HPP
