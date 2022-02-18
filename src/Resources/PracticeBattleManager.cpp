//
// Created by Gegel85 on 18/02/2022.
//

#include "PracticeBattleManager.hpp"
#include "game.hpp"

namespace Battle
{
	PracticeBattleManager::PracticeBattleManager(const CharacterParams &leftCharacter, const CharacterParams &rightCharacter) :
		BattleManager(leftCharacter, rightCharacter)
	{
	}

	bool PracticeBattleManager::update()
	{
		auto result = BattleManager::update();

		this->_score = {0, 0};
		if (!this->_leftCharacter->_comboCtr)
			this->_leftCharacter->_hp = this->_leftCharacter->_baseHp;
		if (!this->_rightCharacter->_comboCtr)
			this->_rightCharacter->_hp = this->_rightCharacter->_baseHp;
		return result;
	}

	void PracticeBattleManager::render()
	{
		BattleManager::render();
	}

	void PracticeBattleManager::consumeEvent(const sf::Event &event)
	{
		BattleManager::consumeEvent(event);
		assert(!game.networkMgr.isConnected());
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F11)
			this->_step = !this->_step;
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F12)
			this->_next = true;
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F9)
			this->_speed--;
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F10)
			this->_speed++;
	}
}
