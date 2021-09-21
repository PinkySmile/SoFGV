//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_BATTLEMANAGER_HPP
#define BATTLE_BATTLEMANAGER_HPP


#include "../Objects/ACharacter.hpp"

namespace Battle
{
	class BattleManager {
	private:
		std::unique_ptr<ACharacter> _leftCharacter;
		std::unique_ptr<ACharacter> _rightCharacter;

	public:
		BattleManager(ACharacter *leftCharacter, ACharacter *rightCharacter);
		void update();
		void render();
	};
}


#endif //BATTLE_BATTLEMANAGER_HPP
