//
// Created by Gegel85 on 18/02/2022.
//

#ifndef BATTLE_PRACTICEBATTLEMANAGER_HPP
#define BATTLE_PRACTICEBATTLEMANAGER_HPP


#include "BattleManager.hpp"

namespace Battle
{
	class PracticeBattleManager : public BattleManager {
	public:
		PracticeBattleManager(const CharacterParams &leftCharacter, const CharacterParams &rightCharacter);
		bool update() override;
		void render() override;
		void consumeEvent(const sf::Event &event) override;

		friend class PracticeInGame;
	};
}


#endif //BATTLE_PRACTICEBATTLEMANAGER_HPP
