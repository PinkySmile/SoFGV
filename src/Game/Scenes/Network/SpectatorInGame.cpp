//
// Created by PinkySmile on 20/03/2022.
//

#include "SpectatorInGame.hpp"
#include "Resources/Game.hpp"
#include "Resources/Network/SpectatorConnection.hpp"
#include "Resources/Battle/PracticeBattleManager.hpp"
#include "../MainMenu/TitleScreen.hpp"
#include "Inputs/ReplayInput.hpp"
#include "Utils.hpp"

namespace SpiralOfFate
{
	SpectatorInGame::SpectatorInGame(const InGame::GameParams &params, const std::vector<struct PlatformSkeleton> &platforms, const struct StageEntry &stage, Character *leftChr, Character *rightChr, unsigned licon, unsigned ricon, const nlohmann::json &lJson, const nlohmann::json &rJson, std::shared_ptr<SpectatorInputManager> inputManager, SpectatorConnection *connection) :
		PracticeInGame(params, platforms, stage, leftChr, rightChr, licon, ricon, lJson, rJson),
		_connection(connection),
		_inputManager(std::move(inputManager))
	{
		this->_replaySaved = false;
		this->_endScene = "title_screen";
		this->_replay = true;
		this->_manager->replay = true;
		game->battleMgr->onFrameSkipped = [this]{
			auto inputs = this->_inputManager->getInputs();

			inputs.first->skipInput();
			inputs.second->skipInput();
		};
	}

	void SpectatorInGame::consumeEvent(const sf::Event &event)
	{
		InGame::consumeEvent(event);
		if (event.type == sf::Event::JoystickButtonPressed || event.type == sf::Event::JoystickButtonReleased)
			game->P1.second->setJoystickId(event.joystickButton.joystickId);
		if (event.type == sf::Event::JoystickMoved)
			game->P1.second->setJoystickId(event.joystickMove.joystickId);
		game->P1.first->consumeEvent(event);
		game->P1.second->consumeEvent(event);
	}

	void SpectatorInGame::_pauseUpdate()
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

	void SpectatorInGame::_practiceUpdate()
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

	void SpectatorInGame::update()
	{
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
			return;
		}

		if ((this->_inputManager->getLastReceivedFrame() != this->_inputManager->getEnd() || !this->_inputManager->getEnd()))
			if (this->_inputManager->getCurrentFrame() % 60 == 0 || !this->_inputManager->hasInputs())
				this->_connection->requestInputs(this->_inputManager->getLastReceivedFrame());

		auto isEnd = this->_inputManager->hasInputs() && !SpiralOfFate::game->battleMgr->update();

		this->_inputManager->update();
		if (isEnd || (this->_inputManager->getCurrentFrame() == this->_inputManager->getEnd() && this->_inputManager->getEnd())) {
			auto args = new SpectatorArguments();

			args->connection = reinterpret_cast<SpectatorConnection *>(&*game->connection);
			game->scene.switchScene("spectator_char_select", args);
			return;
		}
		if (!this->_paused) {
			game->P1.first->update();
			game->P1.second->update();
			if (game->P1.first->isPressed(INPUT_PAUSE) || game->P1.second->isPressed(INPUT_PAUSE))
				this->_paused = 1;
		} else
			this->_pauseUpdate();
	}

	void SpectatorInGame::render() const
	{
		if (this->_moveList) {
			game->battleMgr->render();
			this->_renderMoveList(this->_chr, this->_chr->name + L"'s " + this->_moveListName);
			return;
		}
		PracticeInGame::render();

		game->screen->textSize(15);
		game->screen->displayElement(
			std::to_string(game->battleMgr->getCurrentFrame()) + "/" +
			(this->_inputManager->getEnd() ? std::to_string(this->_inputManager->getEnd()) : std::to_string(this->_inputManager->getLastReceivedFrame()) + "??") + " frames",
			{400, 20},
			200,
			Screen::ALIGN_CENTER
		);
		game->screen->textSize(30);
	}

	void SpectatorInGame::_renderPause() const
	{
		if (this->_paused == 3)
			return;
		if (this->_practice)
			return this->_practiceRender();
		game->screen->displayElement({340 - 50, 240 - 600, 400, 175}, sf::Color{0x50, 0x50, 0x50, 0xC0});

		game->screen->textSize(20);
		game->screen->fillColor(sf::Color::White);
		game->screen->displayElement("Spectator Mode", {340 - 50, 245 - 600}, 400, Screen::ALIGN_CENTER);
		for (size_t i = 0; i < sizeof(SpectatorInGame::_menuStrings) / sizeof(*SpectatorInGame::_menuStrings); i++) {
			game->screen->fillColor(i == this->_pauseCursor ? sf::Color::Yellow : sf::Color::White);
			game->screen->displayElement(SpectatorInGame::_menuStrings[i], {350 - 50, 285 - 600 + 25.f * i});
		}
		game->screen->fillColor(sf::Color::White);
		game->screen->textSize(30);
	}

	void SpectatorInGame::_practiceRender() const
	{
		char const *values[sizeof(SpectatorInGame::_practiceMenuStrings) / sizeof(*SpectatorInGame::_practiceMenuStrings)];
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

		game->screen->displayElement({340 - 50, 190 - 600, 400, 50 + 25 * (sizeof(SpectatorInGame::_practiceMenuStrings) / sizeof(*SpectatorInGame::_practiceMenuStrings))}, sf::Color{0x50, 0x50, 0x50, 0xC0});
		game->screen->textSize(20);
		game->screen->fillColor(sf::Color::White);
		game->screen->displayElement("Replay Options", {340 - 50, 195 - 600}, 400, Screen::ALIGN_CENTER);
		for (size_t i = 0; i < sizeof(SpectatorInGame::_practiceMenuStrings) / sizeof(*SpectatorInGame::_practiceMenuStrings); i++) {
			char buffer[0x400];

			sprintf(buffer, SpectatorInGame::_practiceMenuStrings[i], values[i]);
			game->screen->fillColor(i == this->_practiceCursor ? sf::Color::Yellow : sf::Color::White);
			game->screen->displayElement(buffer, {350 - 50, 235 - 600 + 25.f * i});
		}
		game->screen->fillColor(sf::Color::White);
		game->screen->textSize(30);
	}

	bool SpectatorInGame::_pauseConfirm()
	{
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
			game->connection->terminate();
			game->scene.switchScene("title_screen");
			return false;
		default:
			return false;
		}
	}

	bool SpectatorInGame::_practiceConfirm()
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

	SpectatorInGame *SpectatorInGame::create(SceneArguments *args)
	{
		checked_cast(realArgs, InGameArguments, args);
		checked_cast(con, SpectatorConnection, realArgs->connection);

		if (args->reportProgressA)
			args->reportProgressA("Generating data...");

		auto data = CharacterSelect::loadData();
		auto manager = std::make_shared<SpectatorInputManager>();
		auto inputs = manager->getInputs();
		auto params = CharacterSelect::staticCreateParams(data.first, data.second, realArgs, inputs.first, inputs.second);

		con->onReplayData = [manager](PacketReplay &r){
			manager->parseReplayPacket(r);
		};
		game->battleRandom.seed(realArgs->startParams.seed);
		if (args->reportProgressA)
			args->reportProgressA("Creating scene...");
		return new SpectatorInGame(
			params.params,
			params.platforms, params.stage,
			params.leftChr, params.rightChr,
			params.licon, params.ricon,
			params.lJson, params.rJson,
			manager, con
		);
	}

	SpectatorInGame::Arguments::Arguments(CharacterEntry lentry, CharacterEntry rentry, StageEntry stage) :
		lentry(lentry),
		rentry(rentry),
		stage(stage)
	{
	}
}