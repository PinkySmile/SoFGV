//
// Created by PinkySmile on 20/03/2022.
//

#include "ReplayInGame.hpp"
#include "Resources/Game.hpp"
#include "Resources/Battle/PracticeBattleManager.hpp"
#include "MainMenu/TitleScreen.hpp"
#include "Inputs/ReplayInput.hpp"
#include "Utils.hpp"

namespace SpiralOfFate
{
	ReplayInGame::ReplayInGame(const InGame::GameParams &params, unsigned frameCount, const std::vector<struct PlatformSkeleton> &platforms, const struct StageEntry &stage, Character *leftChr, Character *rightChr, unsigned licon, unsigned ricon, const nlohmann::json &lJson, const nlohmann::json &rJson) :
		PracticeInGame(params, platforms, stage, leftChr, rightChr, licon, ricon, lJson, rJson, false)
	{
		this->_endScene = "title_screen";
		this->_replay = true;
		this->_startTime = frameCount;
		this->_p1 = reinterpret_cast<ReplayInput *>(&*leftChr->getInput());
		this->_p2 = reinterpret_cast<ReplayInput *>(&*rightChr->getInput());
		this->_startingState = this->ReplayInGame::_saveState();
	}

	void ReplayInGame::consumeEvent(const sf::Event &event)
	{
		PracticeInGame::consumeEvent(event);
		if (auto e = event.getIf<sf::Event::JoystickButtonPressed>())
			game->P1.second->setJoystickId(e->joystickId);
		if (auto e = event.getIf<sf::Event::JoystickButtonReleased>())
			game->P1.second->setJoystickId(e->joystickId);
		if (auto e = event.getIf<sf::Event::JoystickMoved>())
			game->P1.second->setJoystickId(e->joystickId);
		game->P1.first->consumeEvent(event);
		game->P1.second->consumeEvent(event);
	}

	std::vector<unsigned char> ReplayInGame::_saveState()
	{
		size_t mSize = this->_manager->getBufferSize();
		size_t p1Size = this->_p1->getBufferSize();
		size_t p2Size = this->_p2->getBufferSize();
		size_t size = mSize + p1Size + p2Size;
		std::vector<unsigned char> buffer;

		buffer.resize(size);

		unsigned char *ptr = buffer.data();

		game->logger.info("Saving ReplayInGame state: " + std::to_string(size) + " bytes total (" + std::to_string(mSize) + " + " + std::to_string(p1Size) + " + " + std::to_string(p2Size) + ")");
		this->_manager->copyToBuffer(ptr);
		ptr += mSize;
		this->_p1->copyToBuffer(ptr);
		ptr += p1Size;
		this->_p2->copyToBuffer(ptr);
		return buffer;
	}

	void ReplayInGame::_restoreState(unsigned char *buffer)
	{
		this->_manager->restoreFromBuffer(buffer);
		buffer += this->_manager->getBufferSize();
		this->_p1->restoreFromBuffer(buffer);
		buffer += this->_p1->getBufferSize();
		this->_p2->restoreFromBuffer(buffer);
	}

	void ReplayInGame::_pauseUpdate()
	{
		if (this->_paused != 3 && this->_practice)
			return this->_practiceUpdate();

		auto keyboard = game->P1.first->getInputs();
		auto controller = game->P1.second->getInputs();

		Utils::mergeInputs(keyboard, controller);
		if (this->_paused == 3) {
			this->_paused = 0;
			return;
		}
		if (keyboard.pause == 1 || keyboard.s == 1) {
			this->_pauseCursor = 0;
			this->_paused = 3;
			return;
		}
		if (keyboard.verticalAxis == 1 || (keyboard.verticalAxis >= 36 && keyboard.verticalAxis % 6 == 0)) {
			this->_pauseCursor += std::size(InGame::_menuStrings);
			this->_pauseCursor--;
			this->_pauseCursor %= std::size(InGame::_menuStrings);
		} else if (keyboard.verticalAxis == -1 || (keyboard.verticalAxis <= -36 && keyboard.verticalAxis % 6 == 0)) {
			this->_pauseCursor++;
			this->_pauseCursor %= std::size(InGame::_menuStrings);
		}
		if (keyboard.n == 1 && this->_pauseConfirm()) {
			this->_pauseCursor = 0;
			this->_paused = 3;
		}
	}

	void ReplayInGame::_practiceUpdate()
	{
		auto keyboard = game->P1.first->getInputs();
		auto controller = game->P1.second->getInputs();

		Utils::mergeInputs(keyboard, controller);
		if (keyboard.pause == 1 || keyboard.s == 1) {
			this->_practice = false;
			return;
		}
		if (keyboard.verticalAxis == 1 || (keyboard.verticalAxis >= 36 && keyboard.verticalAxis % 6 == 0)) {
			this->_practiceCursor += std::size(ReplayInGame::_practiceMenuStrings);
			this->_practiceCursor--;
			this->_practiceCursor %= std::size(ReplayInGame::_practiceMenuStrings);
		} else if (keyboard.verticalAxis == -1 || (keyboard.verticalAxis <= -36 && keyboard.verticalAxis % 6 == 0)) {
			this->_practiceCursor++;
			this->_practiceCursor %= std::size(ReplayInGame::_practiceMenuStrings);
		}
		if (keyboard.n == 1)
			this->_practiceConfirm();
	}

	void ReplayInGame::update()
	{
		game->P1.first->update();
		game->P1.second->update();

		if (this->_moveList) {
			auto keyboard = game->P1.first->getInputs();
			auto controller = game->P1.second->getInputs();

			Utils::mergeInputs(keyboard, controller);
			this->_moveListUpdate(keyboard);
			return;
		}
		if (this->_paused)
			return this->_pauseUpdate();
		if (this->_step && !this->_next)
			return;
		this->_next = false;

		auto linput = game->battleMgr->getLeftCharacter()->getInput();
		auto rinput = game->battleMgr->getRightCharacter()->getInput();

		if (
			game->P1.first->isPressed(INPUT_PAUSE) ||
			game->P1.second->isPressed(INPUT_PAUSE) ||
			(!this->_p1->hasData() && !this->_p2->hasData())
		) {
			this->_paused = 1;
			return;
		}

		if (game->P1.first->isPressed(INPUT_RIGHT)) {
			if (this->_savedFrames.size() <= this->_manager->getCurrentFrame())
				// TODO: Z-compress the state if it ends up being too big
				this->_savedFrames.push_back(this->_saveState());
			if (!game->battleMgr->update()) {
				this->_paused = 1;
				return;
			}
		}
		if (game->P1.first->isPressed(INPUT_LEFT)) {
			auto update = this->_manager->getCurrentFrame() > 1;

			if (this->_manager->getCurrentFrame() > 1) {
				this->_restoreState(this->_savedFrames[this->_manager->getCurrentFrame() - 2].data());
				if (update && !game->battleMgr->update()) {
					this->_paused = 1;
					return;
				}
			} else
				this->_restoreState(this->_startingState.data());
		} else {
			if (this->_savedFrames.size() <= this->_manager->getCurrentFrame())
				// TODO: Z-compress the state if it ends up being too big
				this->_savedFrames.push_back(this->_saveState());
			if (!game->battleMgr->update()) {
				this->_paused = 1;
				return;
			}
		}
	}

	void ReplayInGame::render() const
	{
		if (this->_moveList) {
			game->battleMgr->render();
			if (this->_inputDisplay & 1)
				game->battleMgr->renderLeftInputs();
			if (this->_inputDisplay & 2)
				game->battleMgr->renderRightInputs();
			this->_renderMoveList(this->_chr, this->_chr->name + L"'s " + this->_moveListName);
			return;
		}
		PracticeInGame::render();

		game->screen->displayElement({400 + STAGE_X_MIN, 0, 200, 20}, sf::Color::White);
		game->screen->displayElement({400 + STAGE_X_MIN, 0, static_cast<int>(200.f * game->battleMgr->getCurrentFrame() / this->_startTime), 20}, sf::Color::Black);
		game->screen->textSize(15);
		game->screen->displayElement(std::to_string(game->battleMgr->getCurrentFrame()) + "/" + std::to_string(this->_startTime) + " frames", {400 + STAGE_X_MIN, 20}, 200, Screen::ALIGN_CENTER);
		game->screen->textSize(30);
	}

	void ReplayInGame::_renderPause() const
	{
		if (this->_paused == 3)
			return;
		if (this->_practice)
			return this->_practiceRender();
		game->screen->displayElement({340 - 50 + STAGE_X_MIN, 240 - 600, 400, 175}, sf::Color{0x50, 0x50, 0x50, 0xC0});

		bool end = !reinterpret_cast<ReplayInput *>(&*this->_manager->getLeftCharacter()->getInput())->hasData() &&
		           !reinterpret_cast<ReplayInput *>(&*this->_manager->getRightCharacter()->getInput())->hasData();

		game->screen->textSize(20);
		game->screen->fillColor(sf::Color::White);
		game->screen->displayElement(end ? "Replay Mode | Replay ended" : "Replay Mode", {340 - 50 + STAGE_X_MIN, 245 - 600}, 400, Screen::ALIGN_CENTER);
		for (size_t i = 0; i < std::size(ReplayInGame::_menuStrings); i++) {
			game->screen->fillColor(i == this->_pauseCursor ? sf::Color::Yellow : sf::Color::White);
			game->screen->displayElement(ReplayInGame::_menuStrings[i], {350 - 50 + STAGE_X_MIN, 285 - 600 + 25.f * i});
		}
		game->screen->fillColor(sf::Color::White);
		game->screen->textSize(30);
	}

	void ReplayInGame::_practiceRender() const
	{
		char const *values[std::size(ReplayInGame::_practiceMenuStrings)];
		std::string delay = std::to_string(this->_inputDelay);
		const char *vals[] = {
			"Hidden",
			"P1",
			"P2",
			"Both"
		};

		values[0] = this->_manager->_showBoxes       ? "Shown"    : "Hidden";
		values[1] = !this->_manager->_showAttributes ? "Disabled" : "Enabled";
		values[2] = vals[this->_inputDisplay];

		game->screen->displayElement({340 - 50 + STAGE_X_MIN, 190 - 600, 400, 50 + 25 * std::size(ReplayInGame::_practiceMenuStrings)}, sf::Color{0x50, 0x50, 0x50, 0xC0});
		game->screen->textSize(20);
		game->screen->fillColor(sf::Color::White);
		game->screen->displayElement("Replay Options", {340 - 50 + STAGE_X_MIN, 195 - 600}, 400, Screen::ALIGN_CENTER);
		for (size_t i = 0; i < std::size(ReplayInGame::_practiceMenuStrings); i++) {
			char buffer[0x400];

			sprintf(buffer, ReplayInGame::_practiceMenuStrings[i], values[i]);
			game->screen->fillColor(i == this->_practiceCursor ? sf::Color::Yellow : sf::Color::White);
			game->screen->displayElement(buffer, {350 - 50 + STAGE_X_MIN, 235 - 600 + 25.f * i});
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
			this->_moveListName = L"Move List";
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
			this->_moveListName = L"Move List";
			this->_calculateMoveListOrder();
			this->_moveListObject = std::make_unique<FakeObject>(*this->_moveList);
			return false;
		case 3:
			this->_practice = true;
			return false;
		case 4:
			game->scene.switchScene("title_screen");
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
			this->_manager->_showAttributes = !this->_manager->_showAttributes;
			break;
		case 2:
			this->_inputDisplay++;
			this->_inputDisplay %= 4;
			break;
		}
		return false;
	}

	ReplayInGame *ReplayInGame::create(SceneArguments *args)
	{
		checked_cast(realArgs, ReplayInGame::Arguments, args);

		auto params = InGame::createParams(
			realArgs->stages,
			realArgs->entries,
			realArgs->reportProgressW,
			realArgs->params,
			realArgs->leftInput,
			realArgs->rightInput
		);

		game->battleRandom.seed(realArgs->params.seed);
		if (args->reportProgressA)
			args->reportProgressA("Creating scene...");
		return new ReplayInGame(
			params.params,
			realArgs->frameCount,
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