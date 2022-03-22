//
// Created by Gegel85 on 18/02/2022.
//

#ifndef BATTLE_PRACTICEINGAME_HPP
#define BATTLE_PRACTICEINGAME_HPP


#include "InGame.hpp"

namespace Battle
{
	class PracticeInGame : public InGame {
	protected:
		class PracticeBattleManager *_manager;
		bool _replay = false;
		unsigned _practiceCursor = 0;
		bool _practice = false;
		unsigned char _inputDisplay = 0;
		unsigned char _block = 0;
		unsigned char _inputDelay = 0;
		unsigned char _guardBar = 0;
		unsigned char _overdrive = 0;
		unsigned _rightCounter = 0;
		bool _hitboxes = false;
		bool _debug = false;
		bool _mana = false;

		constexpr static const char *_menuStrings[] = {
			"Resume",
			"Move list",
			"Practice options",
			"Return to character select",
			"Return to title screen"
		};
		constexpr static const char *_practiceMenuStrings[] = {
			"Dummy ground tech: %s",
			"Dummy air tech: %s",
			"Dummy state: %s",
			"Dummy block: %s",
			"Input delay: %s",
			"Counter hit: %s",
			"Guard bar: %s",
			"Overdrive: %s",
			"Hitboxes: %s",
			"Debug: %s",
			"Mana: %s",
			"Input display: %s",
		};
		static bool isOnLastFrame(const Character &chr);
		static bool hasControl(const Character &chr);
		char const *dummyGroundTechToString() const;
		char const *dummyAirTechToString() const;
		char const *dummyStateToString() const;
		char const *blockToString() const;
		void _renderPause() const override;
		void _pauseUpdate() override;
		bool _pauseConfirm() override;

		virtual void _practiceRender() const;
		virtual void _practiceUpdate();
		virtual bool _practiceConfirm();

	public:
		PracticeInGame(const GameParams &params, const std::vector<struct PlatformSkeleton> &platforms, const struct StageEntry &stage, Character *leftChr, Character *rightChr, const nlohmann::json &lJson, const nlohmann::json &rJson);
		IScene *update() override;
		void render() const override;
	};
}


#endif //BATTLE_PRACTICEINGAME_HPP
