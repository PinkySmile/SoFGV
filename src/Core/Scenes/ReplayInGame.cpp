//
// Created by Gegel85 on 20/03/2022.
//

#include "ReplayInGame.hpp"
#include "../Resources/Game.hpp"
#include "../Resources/PracticeBattleManager.hpp"
#include "TitleScreen.hpp"
#include "../Inputs/ReplayInput.hpp"
#include "../Utils.hpp"

namespace Battle
{
	ReplayInGame::ReplayInGame(const InGame::GameParams &params, const std::vector<struct PlatformSkeleton> &platforms, const struct StageEntry &stage, Character *leftChr, Character *rightChr, const nlohmann::json &lJson, const nlohmann::json &rJson) :
		PracticeInGame(params, platforms, stage, leftChr, rightChr, lJson, rJson)
	{
		this->_replay = true;
		this->_manager->replay = true;

		auto left = reinterpret_cast<ReplayInput *>(&*this->_manager->getLeftCharacter()->getInput());
		auto right= reinterpret_cast<ReplayInput *>(&*this->_manager->getRightCharacter()->getInput());

		if (left->getRemainingTime() != right->getRemainingTime())
			Utils::dispMsg("Corrupted replay?", "Replay file contains " + std::to_string(left->getRemainingTime()) + " frames of data for P1 but " + std::to_string(right->getRemainingTime()) + " frames for P2.\nIs it corrupted?", MB_ICONWARNING, &*game->screen);
		this->_startTime = std::max(left->getRemainingTime(), right->getRemainingTime());
	}

	ReplayInGame::~ReplayInGame()
	{
		this->_goBackToTitle = true;
	}

	void ReplayInGame::consumeEvent(const sf::Event &event)
	{
		InGame::consumeEvent(event);
		if (event.type == sf::Event::JoystickButtonPressed || event.type == sf::Event::JoystickButtonReleased)
			game->P1.second->setJoystickId(event.joystickButton.joystickId);
		if (event.type == sf::Event::JoystickMoved)
			game->P1.second->setJoystickId(event.joystickMove.joystickId);
		game->P1.first->consumeEvent(event);
		game->P1.second->consumeEvent(event);
	}

	void ReplayInGame::_pauseUpdate()
	{
		if (this->_paused != 3 && this->_practice)
			return this->_practiceUpdate();

		game->P1.first->update();
		game->P1.second->update();

		auto relevent = game->P1.first->getInputs();
		auto other = game->P1.second->getInputs();

		if (std::abs(((int *)&relevent)[0]) < std::abs(((int *)&other)[0]))
			((int *)&relevent)[0] = ((int *)&other)[0];
		if (std::abs(((int *)&relevent)[1]) < std::abs(((int *)&other)[1]))
			((int *)&relevent)[1] = ((int *)&other)[1];
		for (size_t i = 2; i < sizeof(relevent) / sizeof(int); i++)
			((int *)&relevent)[i] = std::max(((int *)&relevent)[i], ((int *)&other)[i]);

		if (this->_paused == 3) {
			this->_paused = 0;
			return;
		}
		if (relevent.pause == 1 || relevent.s == 1) {
			this->_pauseCursor = 0;
			this->_paused = 3;
			return;
		}
		if (relevent.verticalAxis == 1 || (relevent.verticalAxis >= 36 && relevent.verticalAxis % 6 == 0)) {
			this->_pauseCursor += sizeof(InGame::_menuStrings) / sizeof(*InGame::_menuStrings);
			this->_pauseCursor--;
			this->_pauseCursor %= sizeof(InGame::_menuStrings) / sizeof(*InGame::_menuStrings);
		} else if (relevent.verticalAxis == -1 || (relevent.verticalAxis <= -36 && relevent.verticalAxis % 6 == 0)) {
			this->_pauseCursor++;
			this->_pauseCursor %= sizeof(InGame::_menuStrings) / sizeof(*InGame::_menuStrings);
		}
		if (relevent.n == 1 && this->_pauseConfirm()) {
			this->_pauseCursor = 0;
			this->_paused = 3;
		}
	}

	void ReplayInGame::_practiceUpdate()
	{
		game->P1.first->update();
		game->P1.second->update();

		auto relevent = game->P1.first->getInputs();
		auto other = game->P1.second->getInputs();

		if (std::abs(((int *)&relevent)[0]) < std::abs(((int *)&other)[0]))
			((int *)&relevent)[0] = ((int *)&other)[0];
		if (std::abs(((int *)&relevent)[1]) < std::abs(((int *)&other)[1]))
			((int *)&relevent)[1] = ((int *)&other)[1];
		for (size_t i = 2; i < sizeof(relevent) / sizeof(int); i++)
			((int *)&relevent)[i] = std::max(((int *)&relevent)[i], ((int *)&other)[i]);

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

	IScene *ReplayInGame::update()
	{
		if (this->_nextScene)
			return this->_nextScene;
		if (this->_moveList) {
			game->P1.first->update();
			game->P1.second->update();

			auto relevent = game->P1.first->getInputs();
			auto other = game->P1.second->getInputs();

			if (std::abs(((int *)&relevent)[0]) < std::abs(((int *)&other)[0]))
				((int *)&relevent)[0] = ((int *)&other)[0];
			if (std::abs(((int *)&relevent)[1]) < std::abs(((int *)&other)[1]))
				((int *)&relevent)[1] = ((int *)&other)[1];
			for (size_t i = 2; i < sizeof(relevent) / sizeof(int); i++)
				((int *)&relevent)[i] = std::max(((int *)&relevent)[i], ((int *)&other)[i]);
			this->_moveListUpdate(relevent);
			return this->_nextScene;
		}

		auto linput = game->battleMgr->getLeftCharacter()->getInput();
		auto rinput = game->battleMgr->getRightCharacter()->getInput();

		if (!this->_paused) {
			game->P1.first->update();
			game->P1.second->update();
			if (game->P1.first->isPressed(INPUT_PAUSE) || game->P1.second->isPressed(INPUT_PAUSE) || (
				!reinterpret_cast<ReplayInput *>(&*this->_manager->getLeftCharacter()->getInput())->hasData() &&
				!reinterpret_cast<ReplayInput *>(&*this->_manager->getRightCharacter()->getInput())->hasData()
			)) {
				this->_paused = 1;
				return nullptr;
			}
			if (!Battle::game->battleMgr->update()) {
				this->_paused = 1;
				return nullptr;
			}
			if (linput->getInputs().pause == 1)
				this->_paused = 1;
			else if (rinput->getInputs().pause == 1)
				this->_paused = 2;
		} else
			this->_pauseUpdate();
		return nullptr;
	}

	void ReplayInGame::render() const
	{
		if (this->_moveList) {
			game->battleMgr->render();
			this->_renderMoveList(this->_chr, this->_chr->name + "'s " + this->_moveListName);
			return;
		}
		PracticeInGame::render();

		auto left = reinterpret_cast<ReplayInput *>(&*this->_manager->getLeftCharacter()->getInput());
		auto right= reinterpret_cast<ReplayInput *>(&*this->_manager->getRightCharacter()->getInput());
		auto current = this->_startTime - std::max(left->getRemainingTime(), right->getRemainingTime());

		game->screen->displayElement({400, 0, 200, 20}, sf::Color::White);
		game->screen->displayElement({400, 0, static_cast<int>(200.f * current / this->_startTime), 20}, sf::Color::Black);
		game->screen->textSize(15);
		game->screen->displayElement(std::to_string(current) + "/" + std::to_string(this->_startTime) + " frames", {400, 20}, 200, Screen::ALIGN_CENTER);
		game->screen->textSize(30);
	}

	void ReplayInGame::_renderPause() const
	{
		if (this->_paused == 3)
			return;
		if (this->_practice)
			return this->_practiceRender();
		game->screen->displayElement({340 - 50, 240 - 600, 400, 175}, sf::Color{0x50, 0x50, 0x50, 0xC0});

		bool end = !reinterpret_cast<ReplayInput *>(&*this->_manager->getLeftCharacter()->getInput())->hasData() &&
			   !reinterpret_cast<ReplayInput *>(&*this->_manager->getRightCharacter()->getInput())->hasData();

		game->screen->textSize(20);
		game->screen->fillColor(sf::Color::White);
		game->screen->displayElement(end ? "Replay Mode | Replay ended" : "Replay Mode", {340 - 50, 245 - 600}, 400, Screen::ALIGN_CENTER);
		for (size_t i = 0; i < sizeof(ReplayInGame::_menuStrings) / sizeof(*ReplayInGame::_menuStrings); i++) {
			game->screen->fillColor(i == this->_pauseCursor ? sf::Color::Yellow : sf::Color::White);
			game->screen->displayElement(ReplayInGame::_menuStrings[i], {350 - 50, 285 - 600 + 25.f * i});
		}
		game->screen->fillColor(sf::Color::White);
		game->screen->textSize(30);
	}

	void ReplayInGame::_practiceRender() const
	{
		char const *values[sizeof(ReplayInGame::_practiceMenuStrings) / sizeof(*ReplayInGame::_practiceMenuStrings)];
		std::string delay = std::to_string(this->_inputDelay);
		const char *vals[] = {
			"Hidden",
			"P1",
			"P2",
			"Both"
		};

		values[0] = this->_manager->_showBoxes ? "Shown"    : "Hidden";
		values[1] = !this->_debug              ? "Disabled" : "Enabled";
		values[2] = vals[this->_inputDisplay];

		game->screen->displayElement({340 - 50, 190 - 600, 400, 50 + 25 * (sizeof(ReplayInGame::_practiceMenuStrings) / sizeof(*ReplayInGame::_practiceMenuStrings))}, sf::Color{0x50, 0x50, 0x50, 0xC0});
		game->screen->textSize(20);
		game->screen->fillColor(sf::Color::White);
		game->screen->displayElement("Replay Options", {340 - 50, 195 - 600}, 400, Screen::ALIGN_CENTER);
		for (size_t i = 0; i < sizeof(ReplayInGame::_practiceMenuStrings) / sizeof(*ReplayInGame::_practiceMenuStrings); i++) {
			char buffer[0x400];

			sprintf(buffer, ReplayInGame::_practiceMenuStrings[i], values[i]);
			game->screen->fillColor(i == this->_practiceCursor ? sf::Color::Yellow : sf::Color::White);
			game->screen->displayElement(buffer, {350 - 50, 235 - 600 + 25.f * i});
		}
		game->screen->fillColor(sf::Color::White);
		game->screen->textSize(30);
	}

	bool ReplayInGame::_pauseConfirm()
	{
		if (this->_practice)
			return this->_practiceConfirm();
		switch (this->_pauseCursor) {
		case 0:
			return true;
		case 1:
			this->_chr = game->battleMgr->getLeftCharacter();
			this->_moveList = &this->_chr->getFrameData();
			this->_moveListCursor = 0;
			this->_moveListTop = 0;
			this->_moveOrder = defaultMoveOrder;
			this->_moveData = defaultMoveData;
			this->_moveListName = "Move List";
			this->_calculateMoveListOrder();
			this->_moveListObject = std::make_unique<FakeObject>(*this->_moveList);
			return false;
		case 2:
			this->_chr = game->battleMgr->getRightCharacter();
			this->_moveList = &this->_chr->getFrameData();
			this->_moveListCursor = 0;
			this->_moveListTop = 0;
			this->_moveOrder = defaultMoveOrder;
			this->_moveData = defaultMoveData;
			this->_moveListName = "Move List";
			this->_calculateMoveListOrder();
			this->_moveListObject = std::make_unique<FakeObject>(*this->_moveList);
			return false;
		case 3:
			this->_practice = true;
			return false;
		case 4:
			this->_nextScene = new TitleScreen(
				game->P1,
				game->P2
			);
			return false;
		default:
			return false;
		}
	}

	bool ReplayInGame::_practiceConfirm()
	{
		switch (this->_practiceCursor) {
		case 0:
			this->_manager->_showBoxes = !this->_manager->_showBoxes;
			break;
		case 1:
			this->_debug = !this->_debug;
			this->_manager->_leftCharacter->showAttributes = this->_debug;
			this->_manager->_rightCharacter->showAttributes = this->_debug;
			break;
		case 2:
			this->_inputDisplay++;
			this->_inputDisplay %= 4;
			break;
		}
		return false;
	}
}