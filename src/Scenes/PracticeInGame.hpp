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
		enum GroundTech {
			GROUNDTECH_NONE,
			GROUNDTECH_FORWARD,
			GROUNDTECH_BACKWARD,
			GROUNDTECH_RANDOM
		};
		enum AirTech {
			AIRTECH_NONE,
			AIRTECH_FORWARD,
			AIRTECH_BACKWARD,
			AIRTECH_UP,
			AIRTECH_DOWN,
			AIRTECH_RANDOM
		};
		enum DummyState {
			DUMMYSTATE_STANDING,
			DUMMYSTATE_JUMP,
			DUMMYSTATE_HIGH_JUMP,
			DUMMYSTATE_CROUCH
		};

		class PracticeBattleManager *_manager;
		unsigned _practiceCursor = 0;
		bool _practice = false;
		GroundTech _dummyGroundTech = GROUNDTECH_NONE;
		AirTech _dummyAirTech = AIRTECH_NONE;
		DummyState _dummyState = DUMMYSTATE_STANDING;
		unsigned char _block = 0;
		unsigned char _inputDelay = 0;
		bool _guardBar = false;
		bool _overdrive = false;
		bool _hitboxes = false;
		bool _debug = false;

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
			"Guard bar: %s",
			"Overdrive: %s",
			"Hitboxes: %s",
			"Debug: %s"
		};
		char const *dummyGroundTechToString() const;
		char const *dummyAirTechToString() const;
		char const *dummyStateToString() const;
		char const *blockToString() const;
		void _renderPause() const override;
		void _pauseUpdate() override;
		bool _pauseConfirm() override;
		void _practiceRender() const;
		void _practiceUpdate();
		bool _practiceConfirm();

	public:
		PracticeInGame(Character *leftChr, Character *rightChr, const nlohmann::json &lJson, const nlohmann::json &rJson);
	};
}


#endif //BATTLE_PRACTICEINGAME_HPP
