//
// Created by PinkySmile on 18/02/2022.
//

#include "CharacterSelect.hpp"
#include "PracticeInGame.hpp"
#include "Resources/Game.hpp"
#include "Resources/Battle/PracticeBattleManager.hpp"
#include "Logger.hpp"
#include "TitleScreen.hpp"
#include "Objects/StageObjects/StageObject.hpp"

namespace SpiralOfFate
{
	PracticeInGame::PracticeInGame(const GameParams &params, const std::vector<struct PlatformSkeleton> &platforms, const struct StageEntry &stage, Character *leftChr, Character *rightChr, unsigned licon, unsigned ricon, const nlohmann::json &lJson, const nlohmann::json &rJson) :
		InGame(params)
	{
		this->_replaySaved = true;
		this->_endScene = "char_select";
		this->_manager = new PracticeBattleManager(
			BattleManager::StageParams{
				stage.imagePath,
				[&stage, this]{
					return this->_generateStageObjects(stage);
				},
				[&platforms]{
					std::vector<Platform *> objects;

					for (auto &platform : platforms)
						objects.push_back(new Platform(platform.framedata, platform.width, platform.hp, platform.cd, platform.pos));
					return objects;
				}
			},
			BattleManager::CharacterParams{
				leftChr,
				licon,
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
				lJson["overdrive_cooldown"],
				lJson["ground_drag"],
				{lJson["air_drag"]["x"], lJson["air_drag"]["y"]}
			},
			BattleManager::CharacterParams{
				rightChr,
				ricon,
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
				rJson["overdrive_cooldown"],
				rJson["ground_drag"],
				{rJson["air_drag"]["x"], rJson["air_drag"]["y"]}
			}
		);
		game->battleMgr.reset(this->_manager);
		game->logger.debug("Practice session started");
	}

	void PracticeInGame::_renderPause() const
	{
		if (this->_paused == 3)
			return;
		if (this->_practice)
			return this->_practiceRender();
		game->screen->displayElement({340 - 50, 240 - 600, 400, 175}, sf::Color{0x50, 0x50, 0x50, 0xC0});

		game->screen->textSize(20);
		game->screen->fillColor(sf::Color::White);
		game->screen->displayElement("P" + std::to_string(this->_paused) + " | Practice Mode", {340 - 50, 245 - 600}, 400, Screen::ALIGN_CENTER);
		for (size_t i = 0; i < sizeof(PracticeInGame::_menuStrings) / sizeof(*PracticeInGame::_menuStrings); i++) {
			game->screen->fillColor(i == this->_pauseCursor ? sf::Color::Yellow : sf::Color::White);
			game->screen->displayElement(PracticeInGame::_menuStrings[i], {350 - 50, 285 - 600 + 25.f * i});
		}
		game->screen->fillColor(sf::Color::White);
		game->screen->textSize(30);
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
		switch (this->_manager->_rightCharacter->_dummyGroundTech) {
		case Character::GROUNDTECH_NONE:
			return "None";
		case Character::GROUNDTECH_FORWARD:
			return "Forward";
		case Character::GROUNDTECH_BACKWARD:
			return "Backward";
		case Character::GROUNDTECH_RANDOM:
			return "Random";
		}
		return nullptr;
	}

	char const *PracticeInGame::dummyAirTechToString() const
	{
		switch (this->_manager->_rightCharacter->_dummyAirTech) {
		case Character::AIRTECH_NONE:
			return "None";
		case Character::AIRTECH_FORWARD:
			return "Forward";
		case Character::AIRTECH_BACKWARD:
			return "Backward";
		case Character::AIRTECH_UP:
			return "Up";
		case Character::AIRTECH_DOWN:
			return "Down";
		case Character::AIRTECH_RANDOM:
			return "Random";
		}
		return nullptr;
	}

	char const *PracticeInGame::dummyStateToString() const
	{
		switch (this->_manager->_rightCharacter->_dummyState) {
		case Character::DUMMYSTATE_STANDING:
			return "Standing";
		case Character::DUMMYSTATE_JUMP:
			return "Jump";
		case Character::DUMMYSTATE_HIGH_JUMP:
			return "High Jump";
		case Character::DUMMYSTATE_CROUCH:
			return "Crouch";
		}
		return nullptr;
	}

	char const *PracticeInGame::manaStateToString() const
	{
		switch (this->_mana) {
		case 0:
			return "Normal";
		case 1:
			return "0%";
		case 2:
			return "50%";
		case 3:
			return "100%";
		case 4:
			return "Infinite";
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
		auto chVal = this->_manager->_leftCharacter->_counterHit;
		const char *vals[] = {
			"Hidden",
			"P1",
			"P2",
			"Both"
		};

		values[0] = this->dummyGroundTechToString();
		values[1] = this->dummyAirTechToString();
		values[2] = this->dummyStateToString();
		values[3] = this->blockToString();
		values[4] = delay.c_str();
		values[5] = chVal == 0                 ? "Normal"   : (chVal == 1            ? "Always"   : "Never");
		values[6] = this->_guardBar == 0       ? "Normal"   : (this->_guardBar == 1  ? "Disabled" : "Instant regeneration");
		values[7] = this->_overdrive == 0      ? "Normal"   : (this->_overdrive == 1 ? "Disabled" : "Instant regeneration");
		values[8] = this->_manager->_showBoxes ? "Shown"    : "Hidden";
		values[9] = !this->_debug              ? "Disabled" : "Enabled";
		values[10]= this->manaStateToString();
		values[11]= vals[this->_inputDisplay];

		game->screen->displayElement({340 - 50, 190 - 600, 400, 50 + 25 * (sizeof(PracticeInGame::_practiceMenuStrings) / sizeof(*PracticeInGame::_practiceMenuStrings))}, sf::Color{0x50, 0x50, 0x50, 0xC0});
		game->screen->textSize(20);
		game->screen->fillColor(sf::Color::White);
		game->screen->displayElement("P" + std::to_string(this->_paused) + " | Practice Options", {340 - 50, 195 - 600}, 400, Screen::ALIGN_CENTER);
		for (size_t i = 0; i < sizeof(PracticeInGame::_practiceMenuStrings) / sizeof(*PracticeInGame::_practiceMenuStrings); i++) {
			char buffer[0x400];

			sprintf(buffer, PracticeInGame::_practiceMenuStrings[i], values[i]);
			game->screen->fillColor(i == this->_practiceCursor ? sf::Color::Yellow : sf::Color::White);
			game->screen->displayElement(buffer, {350 - 50, 235 - 600 + 25.f * i});
		}
		game->screen->fillColor(sf::Color::White);
		game->screen->textSize(30);
	}

	void PracticeInGame::_practiceUpdate()
	{
		auto linput = game->battleMgr->getLeftCharacter()->getInput();
		auto rinput = game->battleMgr->getRightCharacter()->getInput();

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
			this->_manager->_rightCharacter->_dummyGroundTech = static_cast<Character::GroundTech>(this->_manager->_rightCharacter->_dummyGroundTech + 1);
			if (this->_manager->_rightCharacter->_dummyGroundTech == Character::GROUNDTECH_RANDOM + 1)
				this->_manager->_rightCharacter->_dummyGroundTech = Character::GROUNDTECH_NONE;
			break;
		case 1:
			this->_manager->_rightCharacter->_dummyAirTech = static_cast<Character::AirTech>(this->_manager->_rightCharacter->_dummyAirTech + 1);
			if (this->_manager->_rightCharacter->_dummyAirTech == Character::AIRTECH_RANDOM + 1)
				this->_manager->_rightCharacter->_dummyAirTech = Character::AIRTECH_NONE;
			break;
		case 2:
			this->_manager->_rightCharacter->_dummyState = static_cast<Character::DummyState>(this->_manager->_rightCharacter->_dummyState + 1);
			if (this->_manager->_rightCharacter->_dummyState == Character::DUMMYSTATE_CROUCH + 1)
				this->_manager->_rightCharacter->_dummyState = Character::DUMMYSTATE_STANDING;
			break;
		case 3:
			this->_block++;
			if (this->_block == Character::BLOCK_AFTER_HIT + 1)
				this->_block = Character::RANDOM_BLOCK | Character::ALL_RIGHT_BLOCK;
			if (this->_block == (Character::RANDOM_BLOCK | Character::RANDOM_HEIGHT_BLOCK) + 1)
				this->_block = 0;
			if (this->_block == Character::BLOCK_1ST_HIT)
				this->_manager->_rightCharacter->_forceBlock = Character::ALL_RIGHT_BLOCK;
			else if (this->_block == Character::BLOCK_AFTER_HIT)
				this->_manager->_rightCharacter->_forceBlock = Character::NO_BLOCK;
			else
				this->_manager->_rightCharacter->_forceBlock = this->_block;
			break;
		case 4:
			this->_inputDelay++;
			this->_inputDelay %= 11;
			break;
		case 5:
			this->_manager->_leftCharacter->_counterHit = (this->_manager->_leftCharacter->_counterHit + 1) % 3;
			this->_manager->_rightCharacter->_counterHit = this->_manager->_leftCharacter->_counterHit;
			break;
		case 6:
			this->_guardBar = (this->_guardBar + 1) % 3;
			break;
		case 7:
			this->_overdrive = (this->_overdrive + 1) % 3;
			break;
		case 8:
			this->_manager->_showBoxes = !this->_manager->_showBoxes;
			break;
		case 9:
			this->_debug = !this->_debug;
			this->_manager->_leftCharacter->showAttributes = this->_debug;
			this->_manager->_rightCharacter->showAttributes = this->_debug;
			break;
		case 10:
			this->_mana = (this->_mana + 1) % 5;
			break;
		case 11:
			this->_inputDisplay++;
			this->_inputDisplay %= 4;
			break;
		}
		return false;
	}

	void PracticeInGame::update()
	{
		InGame::update();
		if (this->_paused)
			return;
		if (!this->_replay) {
			this->_manager->_leftHUDData.score = 0;
			this->_manager->_rightHUDData.score = 0;
		}
		if (this->_block == Character::BLOCK_1ST_HIT) {
			if (this->_manager->_rightCharacter->_blockStun) {
				this->_manager->_rightCharacter->_forceBlock = Character::NO_BLOCK;
				this->_rightCounter = 15;
			} else if (this->_rightCounter == 0)
				this->_manager->_rightCharacter->_forceBlock = Character::ALL_RIGHT_BLOCK;
			else
				this->_rightCounter--;
		} else if (this->_block == Character::BLOCK_AFTER_HIT) {
			if (this->_manager->_rightCharacter->_blockStun) {
				this->_manager->_rightCharacter->_forceBlock = Character::ALL_RIGHT_BLOCK;
				this->_rightCounter = 15;
			} else if (this->_rightCounter == 0)
				this->_manager->_rightCharacter->_forceBlock = Character::NO_BLOCK;
			else
				this->_rightCounter--;
		}
		if (this->_mana == 4) {
			this->_manager->_leftCharacter->_voidMana   = this->_manager->_leftCharacter->_voidManaMax;
			this->_manager->_leftCharacter->_matterMana = this->_manager->_leftCharacter->_matterManaMax;
			this->_manager->_leftCharacter->_spiritMana = this->_manager->_leftCharacter->_spiritManaMax;
			this->_manager->_rightCharacter->_voidMana   = this->_manager->_rightCharacter->_voidManaMax;
			this->_manager->_rightCharacter->_matterMana = this->_manager->_rightCharacter->_matterManaMax;
			this->_manager->_rightCharacter->_spiritMana = this->_manager->_rightCharacter->_spiritManaMax;
		}
		if (!this->_manager->_leftCharacter->_comboCtr && !this->_manager->_leftCharacter->_blockStun && !this->_manager->_rightCharacter->_comboCtr && !this->_manager->_rightCharacter->_blockStun) {
			switch (this->_mana) {
			case 0:
				break;
			case 1:
				this->_manager->_leftCharacter->_voidMana   = 0;
				this->_manager->_leftCharacter->_matterMana = 0;
				this->_manager->_leftCharacter->_spiritMana = 0;
				this->_manager->_rightCharacter->_voidMana   = 0;
				this->_manager->_rightCharacter->_matterMana = 0;
				this->_manager->_rightCharacter->_spiritMana = 0;
				break;
			case 2:
				this->_manager->_leftCharacter->_voidMana   = this->_manager->_leftCharacter->_voidManaMax / 2.f;
				this->_manager->_leftCharacter->_matterMana = this->_manager->_leftCharacter->_matterManaMax / 2.f;
				this->_manager->_leftCharacter->_spiritMana = this->_manager->_leftCharacter->_spiritManaMax / 2.f;
				this->_manager->_rightCharacter->_voidMana   = this->_manager->_rightCharacter->_voidManaMax / 2.f;
				this->_manager->_rightCharacter->_matterMana = this->_manager->_rightCharacter->_matterManaMax / 2.f;
				this->_manager->_rightCharacter->_spiritMana = this->_manager->_rightCharacter->_spiritManaMax / 2.f;
				break;
			case 3:
				this->_manager->_leftCharacter->_voidMana   = this->_manager->_leftCharacter->_voidManaMax;
				this->_manager->_leftCharacter->_matterMana = this->_manager->_leftCharacter->_matterManaMax;
				this->_manager->_leftCharacter->_spiritMana = this->_manager->_leftCharacter->_spiritManaMax;
				this->_manager->_rightCharacter->_voidMana   = this->_manager->_rightCharacter->_voidManaMax;
				this->_manager->_rightCharacter->_matterMana = this->_manager->_rightCharacter->_matterManaMax;
				this->_manager->_rightCharacter->_spiritMana = this->_manager->_rightCharacter->_spiritManaMax;
				break;
			}
			if (!this->_replay) {
				this->_manager->_leftCharacter->_hp = this->_manager->_leftCharacter->_baseHp;
				this->_manager->_rightCharacter->_hp = this->_manager->_rightCharacter->_baseHp;
				this->_manager->_leftCharacter->_ultimateUsed &= this->_manager->_leftCharacter->getCurrentFrameData()->oFlag.ultimate;
				this->_manager->_rightCharacter->_ultimateUsed &= this->_manager->_rightCharacter->getCurrentFrameData()->oFlag.ultimate;
			}
			if (this->_overdrive == 1) {
				this->_manager->_leftCharacter->_odCooldown = this->_manager->_leftCharacter->_barMaxOdCooldown = this->_manager->_leftCharacter->_maxOdCooldown;
				this->_manager->_rightCharacter->_odCooldown = this->_manager->_rightCharacter->_barMaxOdCooldown = this->_manager->_rightCharacter->_maxOdCooldown;
			} else if (this->_overdrive == 2) {
				this->_manager->_leftCharacter->_odCooldown = 0;
				this->_manager->_rightCharacter->_odCooldown = 0;
			}
			if (this->_guardBar == 1) {
				this->_manager->_leftCharacter->_guardCooldown = this->_manager->_leftCharacter->_maxGuardCooldown;
				this->_manager->_rightCharacter->_guardCooldown = this->_manager->_rightCharacter->_maxGuardCooldown;
			} else if (this->_guardBar == 2) {
				this->_manager->_leftCharacter->_guardCooldown = 0;
				this->_manager->_leftCharacter->_guardBar = this->_manager->_leftCharacter->_maxGuardBar;
				this->_manager->_rightCharacter->_guardCooldown = 0;
				this->_manager->_rightCharacter->_guardBar = this->_manager->_rightCharacter->_maxGuardBar;
			}
		}
	}

	bool PracticeInGame::hasControl(const Character &chr)
	{
		if (chr._blockStun)
			return chr._blockStun == 1;
		if (chr._action == ACTION_BEING_KNOCKED_DOWN)
			return isOnLastFrame(chr);
		if (chr._action == ACTION_KNOCKED_DOWN)
			return isOnLastFrame(chr);
		if (chr._action == ACTION_NEUTRAL_TECH)
			return isOnLastFrame(chr);
		if (chr._action == ACTION_FORWARD_TECH)
			return isOnLastFrame(chr);
		if (chr._action == ACTION_BACKWARD_TECH)
			return isOnLastFrame(chr);
		if (chr._action == ACTION_UP_AIR_TECH)
			return isOnLastFrame(chr);
		if (chr._action == ACTION_DOWN_AIR_TECH)
			return isOnLastFrame(chr);
		if (chr._action == ACTION_FORWARD_AIR_TECH)
			return isOnLastFrame(chr);
		if (chr._action == ACTION_BACKWARD_AIR_TECH)
			return isOnLastFrame(chr);
		return true;
	}

	bool PracticeInGame::isOnLastFrame(const Character &chr)
	{
		return chr._animationCtr == chr._moves.at(chr._action).at(chr._actionBlock).at(chr._animation).duration - 1;
	}

	void PracticeInGame::render() const
	{
		sf::View view{{-50, -600, 1100, 700}};

		game->screen->setView(view);
		game->battleMgr->render();
		if (this->_inputDisplay & 1)
			game->battleMgr->renderLeftInputs();
		if (this->_inputDisplay & 2)
			game->battleMgr->renderRightInputs();
		if (this->_moveList) {
			auto linput = game->battleMgr->getLeftCharacter();
			auto rinput = game->battleMgr->getRightCharacter();
			auto relevent = (this->_paused == 1 ? linput : rinput);

			this->_renderMoveList(relevent, L"P" + sf::String(std::to_string(this->_paused)) + L" | " + relevent->name + L"'s " + this->_moveListName);
		} else if (this->_paused)
			this->_renderPause();
	}

	PracticeInGame *PracticeInGame::create(SceneArguments *args)
	{
		checked_cast(realArgs, InGame::Arguments, args);

		auto params = realArgs->characterSelectScene->createParams(args);

		if (args->reportProgressA)
			args->reportProgressA("Creating scene...");
		return new PracticeInGame(
			params.params,
			params.platforms,
			params.stage,
			params.leftChr,
			params.rightChr,
			params.licon,
			params.ricon,
			params.lJson,
			params.rJson
		);
	}
}
