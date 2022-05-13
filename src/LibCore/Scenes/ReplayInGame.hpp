//
// Created by PinkySmile on 20/03/2022.
//

#ifndef SOFGV_REPLAYINGAME_HPP
#define SOFGV_REPLAYINGAME_HPP


#include "PracticeInGame.hpp"

namespace SpiralOfFate
{
	class ReplayInGame : public PracticeInGame {
	protected:
		constexpr static const char *_menuStrings[] = {
			"Resume",
			"P1 Move list",
			"P2 Move list",
			"Replay options",
			"Return to title screen"
		};
		constexpr static const char *_practiceMenuStrings[] = {
			"Hitboxes: %s",
			"Debug: %s",
			"Input display: %s",
		};

		Character *_chr;
		size_t _startTime = 0;

		void _pauseUpdate() override;
		void _practiceUpdate() override;
		void _renderPause() const override;
		bool _pauseConfirm() override;
		void _practiceRender() const override;
		bool _practiceConfirm() override;

	public:
		ReplayInGame(const GameParams &params, const std::vector<struct PlatformSkeleton> &platforms, const struct StageEntry &stage, Character *leftChr, Character *rightChr, const nlohmann::json &lJson, const nlohmann::json &rJson);
		~ReplayInGame() override;
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
		void render() const override;
	};
}


#endif //SOFGV_REPLAYINGAME_HPP
