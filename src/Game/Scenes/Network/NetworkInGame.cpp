//
// Created by PinkySmile on 11/11/2022.
//

#include "NetworkInGame.hpp"
#include "Resources/Game.hpp"
#include "../CharacterSelect.hpp"
#include "Inputs/DelayInput.hpp"

namespace SpiralOfFate
{
	NetworkInGame::NetworkInGame(
		std::shared_ptr<RemoteInput> input,
		const InGame::GameParams &params,
		const std::vector<struct PlatformSkeleton> &platforms,
		const struct StageEntry &stage,
		Character *leftChr,
		Character *rightChr,
		unsigned int licon,
		unsigned int ricon,
		const nlohmann::json &lJson,
		const nlohmann::json &rJson
	) :
		InGame(params, platforms, stage, leftChr, rightChr, licon, ricon, lJson, rJson, true, ""),
		_leftDInput(reinterpret_cast<DelayInput *>(&*game->battleMgr->getLeftCharacter()->getInput())),
		_rightDInput(reinterpret_cast<DelayInput *>(&*game->battleMgr->getRightCharacter()->getInput())),
		_rMachine(leftChr, rightChr),
		_input(std::move(input))
#ifdef _DEBUG
		,
		_leftChr(leftChr),
		_rightChr(rightChr)
#endif
	{
		game->connection->onDesync = [this](Connection::Remote &remote, unsigned frameId, unsigned cpuSum, unsigned recvSum){
			auto error = (char *)malloc(400);

			sprintf(error, "Invalid checksum on frame %d\n%08X computed but received %08X", frameId, cpuSum, recvSum);
			*strchr(error, '\n') = 0;
			this->_errorMutex.lock();

			auto old = this->_error;

			this->_error = error;
			this->_errorMutex.unlock();
			this->_errorClock.restart();
			free(old);
		};
	}

	NetworkInGame::~NetworkInGame()
	{
		if (game->connection)
			game->connection->onDesync = nullptr;
	}

	bool stepMode = false;
	bool step = false;

	void NetworkInGame::update()
	{
		this->_input->refreshInputs();
		if (stepMode && !step)
			return;
		step = false;

		auto time = this->_errorClock.getElapsedTime().asSeconds();

		if (this->_error && time >= 2) {
			this->_errorMutex.lock();

			auto old = this->_error;

			this->_error = nullptr;
			this->_errorMutex.unlock();
			free(old);
		}

		auto linput = game->battleMgr->getLeftCharacter()->getInput();
		auto rinput = game->battleMgr->getRightCharacter()->getInput();

		this->_leftDInput->fillBuffer();
		this->_rightDInput->fillBuffer();

		auto status = this->_rMachine.update(true, true);

		if (status == RollbackMachine::UPDATESTATUS_NO_INPUTS)
			return;
#ifdef _DEBUG
		this->_currentFrame++;
		my_assert(this->_currentFrame <= game->connection->_currentFrame + game->connection->getCurrentDelay());
#endif

		if (this->_moveList) {
			linput->update();
			rinput->update();
			this->_moveListUpdate((this->_paused == 1 ? linput : rinput)->getInputs());
		} else if (this->_paused)
			this->_pauseUpdate();
		if (status == RollbackMachine::UPDATESTATUS_GAME_ENDED) {
			this->_onGameEnd();
			return;
		}
		if (!this->_paused) {
			if (linput->getInputs().pause == 1)
				this->_paused = 1;
			else if (rinput->getInputs().pause == 1)
				this->_paused = 2;
		}
	}

	void NetworkInGame::render() const
	{
		InGame::render();
		char buffer[500];

		game->screen->borderColor(2, sf::Color::Black);
		game->screen->fillColor(sf::Color::White);
		game->screen->textSize(20);
		sprintf(buffer, "Rand %08llx|%llu", (unsigned long long)game->battleRandom.ser.seed, (unsigned long long)game->battleRandom.ser.invoke_count);
		game->screen->displayElement(buffer, {-50, 25}, 145, Screen::ALIGN_LEFT);
		sprintf(buffer, "Delay %u (%zi:%zi)", this->_leftDInput->getDelay(), this->_leftDInput->getBufferSize(), this->_rightDInput->getBufferSize());
		game->screen->displayElement(buffer, {-50, 50}, 145, Screen::ALIGN_LEFT);
		sprintf(buffer, "Rollback %zu/%zu", this->_rMachine.getBufferSize(), this->_rMachine.getMaxBufferSize());
		game->screen->displayElement(buffer, {-50, 75}, 145, Screen::ALIGN_LEFT);
		game->screen->textSize(15);
		game->screen->displayElement(game->connection->getNames().first, {-50, -592}, 340, Screen::ALIGN_CENTER);
		game->screen->displayElement(game->connection->getNames().second, {710, -592}, 340, Screen::ALIGN_CENTER);
		game->screen->textSize(30);
		game->screen->borderColor(0, sf::Color::Transparent);
		if (this->_error) {
			this->_errorMutex.lock();
			game->screen->fillColor(sf::Color::Red);
			game->screen->displayElement("Desync detected!", {-50, -262}, 1100, Screen::ALIGN_CENTER);
			game->screen->displayElement(this->_error, {-50, -232}, 1100, Screen::ALIGN_CENTER);
			game->screen->displayElement(this->_error + strlen(this->_error) + 1, {-50, -202}, 1100, Screen::ALIGN_CENTER);
			game->screen->fillColor(sf::Color::White);
			this->_errorMutex.unlock();
		}
#ifdef _DEBUG
		if (this->_displayInputs) {
			game->battleMgr->renderLeftInputs();
			game->battleMgr->renderRightInputs();
		}
#endif
	}

	void NetworkInGame::consumeEvent(const sf::Event &event)
	{
		this->_rMachine.consumeEvent(event);
		InGame::consumeEvent(event);
#ifdef _DEBUG
		if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::F11)
				stepMode = true;
			if (event.key.code == sf::Keyboard::F10)
				step = true;
			if (event.key.code == sf::Keyboard::F2)
				this->_leftChr->showAttributes = this->_rightChr->showAttributes = !this->_rightChr->showAttributes;
			if (event.key.code == sf::Keyboard::F3)
				this->_leftChr->showBoxes = this->_rightChr->showBoxes = !this->_rightChr->showBoxes;
			if (event.key.code == sf::Keyboard::F4)
				this->_displayInputs = !this->_displayInputs;
			if (event.key.code == sf::Keyboard::F5)
				game->battleRandom();
		}
#endif
	}
}