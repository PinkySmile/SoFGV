//
// Created by Gegel85 on 24/09/2021.
//

#ifndef BATTLE_INGAME_HPP
#define BATTLE_INGAME_HPP


#include <memory>
#include "IScene.hpp"
#include "../Inputs/IInput.hpp"
#include "../Objects/ACharacter.hpp"
#include "../Resources/MoveListData.hpp"

namespace Battle
{
	class InGame : public IScene {
	private:
		constexpr static const char *_menuStrings[] = {
			"Resume",
			"Move list",
			"Return to character select",
			"Return to title screen"
		};
		const std::map<unsigned, std::vector<std::vector<FrameData>>> *_moveList = nullptr;
		unsigned char _moveListCursor = 0;
		unsigned char _moveListTop = 0;
		unsigned char _paused = 0;
		unsigned char _pauseCursor = 0;
		IScene *_nextScene = nullptr;
		std::array<sf::Texture, NB_SPRITES> _moveSprites;
		std::vector<unsigned> _moveOrder;

		void _renderPause() const;
		void _pauseUpdate();
		bool _pauseConfirm();
		void _renderMoveList() const;
		void _moveListUpdate();
		void _calculateMoveListOrder();

	public:
		InGame(ACharacter *leftChr, ACharacter *rightChr, const nlohmann::json &lJson, const nlohmann::json &rJson);
		void render() const override;
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_INGAME_HPP
