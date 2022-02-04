//
// Created by Gegel85 on 24/09/2021.
//

#include "InGame.hpp"
#include "../Objects/ACharacter.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"
#include "CharacterSelect.hpp"
#include "TitleScreen.hpp"

namespace Battle
{
	static const char * const text[] = {
		"in the air",
		"during transform",
		"+",
		"charge"
	};

	InGame::InGame(ACharacter *leftChr, ACharacter *rightChr, const nlohmann::json &lJson, const nlohmann::json &rJson)
	{
		sf::View view{{-50, -600, 1100, 700}};

		this->_moveSprites[SPRITE_2].loadFromFile("2.png");
		this->_moveSprites[SPRITE_3].loadFromFile("3.png");
		this->_moveSprites[SPRITE_4].loadFromFile("4.png");
		this->_moveSprites[SPRITE_6].loadFromFile("6.png");
		this->_moveSprites[SPRITE_8].loadFromFile("8.png");
		this->_moveSprites[SPRITE_214].loadFromFile("214.png");
		this->_moveSprites[SPRITE_236].loadFromFile("236.png");
		this->_moveSprites[SPRITE_421].loadFromFile("421.png");
		this->_moveSprites[SPRITE_426].loadFromFile("426.png");
		this->_moveSprites[SPRITE_623].loadFromFile("623.png");
		this->_moveSprites[SPRITE_624].loadFromFile("624.png");
		this->_moveSprites[SPRITE_624684].loadFromFile("624684.png");
		this->_moveSprites[SPRITE_N].loadFromFile("neutral.png");
		this->_moveSprites[SPRITE_D].loadFromFile("dash.png");
		this->_moveSprites[SPRITE_M].loadFromFile("matter.png");
		this->_moveSprites[SPRITE_S].loadFromFile("spirit.png");
		this->_moveSprites[SPRITE_V].loadFromFile("void.png");
		this->_moveSprites[SPRITE_A].loadFromFile("ascend.png");
		logger.info("InGame scene created");
		Battle::game.screen->setView(view);
		game.battleMgr = std::make_unique<BattleManager>(
			BattleManager::CharacterParams{
				leftChr,
				lJson["hp"],
				{lJson["gravity"]["x"], lJson["gravity"]["y"]},
				lJson["jump_count"],
				lJson["air_dash_count"],
				lJson["void_mana_max"],
				lJson["spirit_mana_max"],
				lJson["matter_mana_max"],
				lJson["mana_regen"]
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
				rJson["mana_regen"]
			}
		);
	}

	void InGame::render() const
	{
		Battle::game.battleMgr->render();
		if (this->_moveList)
			this->_renderMoveList();
		else if (this->_paused)
			this->_renderPause();
	}

	IScene *InGame::update()
	{
		if (this->_nextScene)
			return this->_nextScene;

		auto linput = game.battleMgr->getLeftCharacter()->getInput();
		auto rinput = game.battleMgr->getRightCharacter()->getInput();

		if (this->_moveList)
			this->_moveListUpdate();
		else if (!this->_paused) {
			Battle::game.battleMgr->update();
			if (linput->getInputs().pause == 1)
				this->_paused = 1;
			else if (rinput->getInputs().pause == 1)
				this->_paused = 2;
		} else
			this->_pauseUpdate();
		return this->_nextScene;
	}

	void InGame::consumeEvent(const sf::Event &event)
	{
		game.battleMgr->consumeEvent(event);
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
			if (this->_paused) {
				this->_paused = 3;
				this->_pauseCursor = 0;
			} else if (game.battleMgr->getLeftCharacter()->getInput()->getName() == "Keyboard")
				this->_paused = 1;
			else if (game.battleMgr->getRightCharacter()->getInput()->getName() == "Keyboard")
				this->_paused = 2;
			else
				this->_paused = 1;
		}
	}

	void InGame::_renderPause() const
	{
		if (this->_paused == 3)
			return;
		game.screen->displayElement({340 - 50, 240 - 600, 400, 150}, sf::Color{0x50, 0x50, 0x50, 0xC0});

		game.screen->textSize(20);
		game.screen->fillColor(sf::Color::White);
		game.screen->displayElement("P" + std::to_string(this->_paused) + " | Paused", {340 - 50, 245 - 600}, 400, Screen::ALIGN_CENTER);
		for (size_t i = 0; i < sizeof(InGame::_menuStrings) / sizeof(*InGame::_menuStrings); i++) {
			game.screen->fillColor(i == this->_pauseCursor ? sf::Color::Red : sf::Color::White);
			game.screen->displayElement(InGame::_menuStrings[i], {350 - 50, 285 - 600 + 25.f * i});
		}
		game.screen->textSize(30);
	}

	void InGame::_pauseUpdate()
	{
		auto linput = game.battleMgr->getLeftCharacter()->getInput();
		auto rinput = game.battleMgr->getRightCharacter()->getInput();

		linput->update();
		rinput->update();

		auto relevent = (this->_paused == 1 ? linput : rinput)->getInputs();

		if (this->_paused == 3) {
			auto l = linput->getInputs();
			auto r = linput->getInputs();

			for (size_t i = 0; i < sizeof(l) / sizeof(int); i++)
				if (((int *)&l)[i])
					return;
			for (size_t i = 0; i < sizeof(r) / sizeof(int); i++)
				if (((int *)&r)[i])
					return;
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

	bool InGame::_pauseConfirm()
	{
		switch (this->_pauseCursor) {
		case 0:
			return true;
		case 1:
			this->_moveList = &(this->_paused == 1 ? game.battleMgr->getLeftCharacter() : game.battleMgr->getRightCharacter())->getFrameData();
			this->_moveListCursor = 0;
			this->_moveOrder = defaultMoveOrder;
			this->_calculateMoveListOrder();
			return false;
		case 2:
			this->_nextScene = new CharacterSelect(
				game.battleMgr->getLeftCharacter()->getInput(),
				game.battleMgr->getRightCharacter()->getInput()
			);
			return false;
		case 3:
			this->_nextScene = new TitleScreen(
				game.P1,
				game.P2
			);
			return false;
		default:
			return false;
		}
	}

	void InGame::_renderMoveList() const
	{

	}

	void InGame::_moveListUpdate()
	{
		this->_moveList = nullptr;
	}

	void InGame::_calculateMoveListOrder()
	{
		std::vector<unsigned> ultimates;
		std::vector<unsigned> supers;

		for (size_t i = 0; i < this->_moveOrder.size(); i++) {
			auto it = this->_moveList->find(this->_moveOrder[i]);

			if (it == this->_moveList->end()) {
				this->_moveOrder.erase(this->_moveOrder.begin() + i);
				i--;
			} else if (it->second.front().front().oFlag.ultimate) {
				ultimates.push_back(this->_moveOrder[i]);
				this->_moveOrder.erase(this->_moveOrder.begin() + i);
				i--;
			} else if (it->second.front().front().oFlag.super) {
				supers.push_back(this->_moveOrder[i]);
				this->_moveOrder.erase(this->_moveOrder.begin() + i);
				i--;
			}
		}
		this->_moveOrder.insert(this->_moveOrder.end(), supers.begin(), supers.end());
		this->_moveOrder.insert(this->_moveOrder.end(), ultimates.begin(), ultimates.end());
	}
}