//
// Created by Gegel85 on 18/02/2022.
//

#include "PracticeInGame.hpp"
#include "../Resources/Game.hpp"
#include "../Resources/PracticeBattleManager.hpp"
#include "../Logger.hpp"
#include "TitleScreen.hpp"

namespace Battle
{
	PracticeInGame::PracticeInGame(Character *leftChr, Character *rightChr, const nlohmann::json &lJson, const nlohmann::json &rJson) :
		InGame()
	{
		this->_manager = new PracticeBattleManager(
			BattleManager::CharacterParams{
				leftChr,
				lJson["hp"],
				{lJson["gravity"]["x"], lJson["gravity"]["y"]},
				lJson["jump_count"],
				lJson["air_dash_count"],
				lJson["void_mana_max"],
				lJson["spirit_mana_max"],
				lJson["matter_mana_max"],
				lJson["mana_regen"],
				lJson["guard_bar"],
				lJson["guard_break_cooldown"],
				lJson["overdrive_cooldown"]
			},
			BattleManager::CharacterParams{
				rightChr,
				rJson["hp"],
				{rJson["gravity"]["x"], rJson["gravity"]["y"]},
				rJson["jump_count"],
				rJson["air_dash_count"],
				rJson["void_mana_max"],
				rJson["spirit_mana_max"],
				rJson["matter_mana_max"],
				rJson["mana_regen"],
				rJson["guard_bar"],
				rJson["guard_break_cooldown"],
				rJson["overdrive_cooldown"]
			}
		);
		game.battleMgr.reset(this->_manager);
		logger.debug("Practice session started");
	}

	void PracticeInGame::_renderPause() const
	{
		if (this->_paused == 3)
			return;
		if (this->_practice)
			return this->_practiceRender();
		game.screen->displayElement({340 - 50, 240 - 600, 400, 175}, sf::Color{0x50, 0x50, 0x50, 0xC0});

		game.screen->textSize(20);
		game.screen->fillColor(sf::Color::White);
		game.screen->displayElement("P" + std::to_string(this->_paused) + " | Practice Mode", {340 - 50, 245 - 600}, 400, Screen::ALIGN_CENTER);
		for (size_t i = 0; i < sizeof(PracticeInGame::_menuStrings) / sizeof(*PracticeInGame::_menuStrings); i++) {
			game.screen->fillColor(i == this->_pauseCursor ? sf::Color::Yellow : sf::Color::White);
			game.screen->displayElement(PracticeInGame::_menuStrings[i], {350 - 50, 285 - 600 + 25.f * i});
		}
		game.screen->textSize(30);
	}

	void PracticeInGame::_pauseUpdate()
	{
		if (this->_paused != 3 && this->_practice)
			return this->_practiceUpdate();
		InGame::_pauseUpdate();
	}

	bool PracticeInGame::_pauseConfirm()
	{
		if (this->_practice)
			return this->_practiceConfirm();
		if (this->_pauseCursor == 2) {
			this->_practice = true;
			return false;
		}
		return InGame::_pauseConfirm();
	}

	char const *PracticeInGame::dummyGroundTechToString() const
	{
		switch (this->_dummyGroundTech) {
		case GROUNDTECH_NONE:
			return "None";
		case GROUNDTECH_FORWARD:
			return "Forward";
		case GROUNDTECH_BACKWARD:
			return "Backward";
		case GROUNDTECH_RANDOM:
			return "Random";
		}
		return nullptr;
	}

	char const *PracticeInGame::dummyAirTechToString() const
	{
		switch (this->_dummyAirTech) {
		case AIRTECH_NONE:
			return "None";
		case AIRTECH_FORWARD:
			return "Forward";
		case AIRTECH_BACKWARD:
			return "Backward";
		case AIRTECH_UP:
			return "Up";
		case AIRTECH_DOWN:
			return "Down";
		case AIRTECH_RANDOM:
			return "Random";
		}
		return nullptr;
	}

	char const *PracticeInGame::dummyStateToString() const
	{
		switch (this->_dummyState) {
		case DUMMYSTATE_STANDING:
			return "Standing";
		case DUMMYSTATE_JUMP:
			return "Jump";
		case DUMMYSTATE_HIGH_JUMP:
			return "High Jump";
		case DUMMYSTATE_CROUCH:
			return "Crouch";
		}
		return nullptr;
	}

	char const *PracticeInGame::blockToString() const
	{
		switch (this->_block) {
		case Character::NO_BLOCK:
			return "No Block";
		case Character::ALL_RIGHT_BLOCK:
			return "All Rightblock";
		case Character::ALL_WRONG_BLOCK:
			return "All Wrongblock";
		case Character::HIGH_BLOCK:
			return "High Block";
		case Character::LOW_BLOCK:
			return "Low Block";
		case Character::RANDOM_HEIGHT_BLOCK:
			return "Random Height Block";
		case Character::BLOCK_1ST_HIT:
			return "Block 1st hit";
		case Character::BLOCK_AFTER_HIT:
			return "Block after hit";
		case Character::RANDOM_BLOCK | Character::ALL_RIGHT_BLOCK:
			return "87.5% | All Rightblock";
		case Character::RANDOM_BLOCK | Character::ALL_WRONG_BLOCK:
			return "87.5% | All Wrongblock";
		case Character::RANDOM_BLOCK | Character::HIGH_BLOCK:
			return "87.5% | High Block";
		case Character::RANDOM_BLOCK | Character::LOW_BLOCK:
			return "87.5% | Low Block";
		case Character::RANDOM_BLOCK | Character::RANDOM_HEIGHT_BLOCK:
			return "87.5% | Random Height Block";
		}
		return nullptr;
	}

	void PracticeInGame::_practiceRender() const
	{
		char const *values[sizeof(PracticeInGame::_practiceMenuStrings) / sizeof(*PracticeInGame::_practiceMenuStrings)];
		std::string delay = std::to_string(this->_inputDelay);

		values[0] = this->dummyGroundTechToString();
		values[1] = this->dummyAirTechToString();
		values[2] = this->dummyStateToString();
		values[3] = this->blockToString();
		values[4] = delay.c_str();
		values[5] = this->_guardBar == 0  ? "Normal"   : (this->_guardBar == 1  ? "Disabled" : "Instant regeneration");
		values[6] = this->_overdrive == 0 ? "Normal"   : (this->_overdrive == 1 ? "Disabled" : "Instant regeneration");
		values[7] = !this->_hitboxes      ? "Hidden"   : "Shown";
		values[8] = !this->_debug         ? "Disabled" : "Enabled";

		game.screen->displayElement({340 - 50, 190 - 600, 400, 275}, sf::Color{0x50, 0x50, 0x50, 0xC0});
		game.screen->textSize(20);
		game.screen->fillColor(sf::Color::White);
		game.screen->displayElement("P" + std::to_string(this->_paused) + " | Practice Options", {340 - 50, 195 - 600}, 400, Screen::ALIGN_CENTER);
		for (size_t i = 0; i < sizeof(PracticeInGame::_practiceMenuStrings) / sizeof(*PracticeInGame::_practiceMenuStrings); i++) {
			char buffer[0x400];

			sprintf(buffer, PracticeInGame::_practiceMenuStrings[i], values[i]);
			game.screen->fillColor(i == this->_practiceCursor ? sf::Color::Yellow : sf::Color::White);
			game.screen->displayElement(buffer, {350 - 50, 235 - 600 + 25.f * i});
		}
		game.screen->textSize(30);
	}

	void PracticeInGame::_practiceUpdate()
	{
		auto linput = game.battleMgr->getLeftCharacter()->getInput();
		auto rinput = game.battleMgr->getRightCharacter()->getInput();

		linput->update();
		rinput->update();

		auto relevent = (this->_paused == 1 ? linput : rinput)->getInputs();

		if (relevent.pause == 1 || relevent.s == 1) {
			this->_practice = false;
			return;
		}
		if (relevent.verticalAxis == 1 || (relevent.verticalAxis >= 36 && relevent.verticalAxis % 6 == 0)) {
			this->_practiceCursor += sizeof(PracticeInGame::_practiceMenuStrings) / sizeof(*PracticeInGame::_practiceMenuStrings);
			this->_practiceCursor--;
			this->_practiceCursor %= sizeof(PracticeInGame::_practiceMenuStrings) / sizeof(*PracticeInGame::_practiceMenuStrings);
		} else if (relevent.verticalAxis == -1 || (relevent.verticalAxis <= -36 && relevent.verticalAxis % 6 == 0)) {
			this->_practiceCursor++;
			this->_practiceCursor %= sizeof(PracticeInGame::_practiceMenuStrings) / sizeof(*PracticeInGame::_practiceMenuStrings);
		}
		if (relevent.n == 1)
			this->_practiceConfirm();
	}

	bool PracticeInGame::_practiceConfirm()
	{
		switch (this->_practiceCursor) {
		case 0:
			this->_dummyGroundTech = static_cast<GroundTech>(this->_dummyGroundTech + 1);
			if (this->_dummyGroundTech == GROUNDTECH_RANDOM + 1)
				this->_dummyGroundTech = GROUNDTECH_NONE;
			break;
		case 1:
			this->_dummyAirTech = static_cast<AirTech>(this->_dummyAirTech + 1);
			if (this->_dummyAirTech == AIRTECH_RANDOM + 1)
				this->_dummyAirTech = AIRTECH_NONE;
			break;
		case 2:
			this->_dummyState = static_cast<DummyState>(this->_dummyState + 1);
			if (this->_dummyState == DUMMYSTATE_CROUCH + 1)
				this->_dummyState = DUMMYSTATE_STANDING;
			break;
		case 3:
			this->_block++;
			if (this->_block == Character::BLOCK_AFTER_HIT + 1)
				this->_block = Character::RANDOM_BLOCK | Character::ALL_RIGHT_BLOCK;
			if (this->_block == (Character::RANDOM_BLOCK | Character::RANDOM_HEIGHT_BLOCK) + 1)
				this->_block = 0;
			this->_manager->_rightCharacter->_forceBlock = this->_block;
			break;
		case 4:
			this->_inputDelay++;
			this->_inputDelay %= 11;
			break;
		case 5:
			this->_guardBar = (this->_guardBar + 1) % 3;
			break;
		case 6:
			this->_overdrive = (this->_overdrive + 1) % 3;
			break;
		case 7:
			this->_hitboxes = !this->_hitboxes;
			this->_manager->_leftCharacter->showBoxes = this->_hitboxes;
			this->_manager->_rightCharacter->showBoxes = this->_hitboxes;
			break;
		case 8:
			this->_debug = !this->_debug;
			this->_manager->_leftCharacter->showAttributes = this->_debug;
			this->_manager->_rightCharacter->showAttributes = this->_debug;
			break;
		}
		return false;
	}

	IScene *PracticeInGame::update()
	{
		auto result = InGame::update();

		if (this->_paused)
			return result;
		if (!this->_manager->_leftCharacter->_comboCtr && !this->_manager->_leftCharacter->_blockStun) {
			if (this->_overdrive == 1)
				this->_manager->_leftCharacter->_odCooldown = this->_manager->_leftCharacter->_maxOdCooldown;
			else if (this->_overdrive == 2)
				this->_manager->_leftCharacter->_odCooldown = 0;
			if (this->_guardBar == 1)
				this->_manager->_leftCharacter->_guardCooldown = this->_manager->_leftCharacter->_maxGuardCooldown;
			else if (this->_guardBar == 2) {
				this->_manager->_leftCharacter->_guardCooldown = 0;
				this->_manager->_leftCharacter->_guardBar = this->_manager->_leftCharacter->_maxGuardBar;
			}
		}
		if (!this->_manager->_rightCharacter->_comboCtr && !this->_manager->_rightCharacter->_blockStun) {
			if (this->_overdrive == 1)
				this->_manager->_rightCharacter->_odCooldown = this->_manager->_rightCharacter->_maxOdCooldown;
			else if (this->_overdrive == 2)
				this->_manager->_rightCharacter->_odCooldown = 0;
			if (this->_guardBar == 1)
				this->_manager->_rightCharacter->_guardCooldown = this->_manager->_rightCharacter->_maxGuardCooldown;
			else if (this->_guardBar == 2) {
				this->_manager->_rightCharacter->_guardCooldown = 0;
				this->_manager->_rightCharacter->_guardBar = this->_manager->_rightCharacter->_maxGuardBar;
			}
		}
		return result;
	}
}
