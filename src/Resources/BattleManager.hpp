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
		Sprite _stage;
		std::unique_ptr<ACharacter> _leftCharacter;
		std::unique_ptr<ACharacter> _rightCharacter;
		std::vector<std::unique_ptr<IObject>> _objects;
		unsigned _hitStop = 0;
		bool _step = false;
		bool _next = false;

	public:
		struct CharacterParams {
			ACharacter *character;
			unsigned hp;
			Vector2f gravity;
			unsigned char maxJumps;
			unsigned char maxAirDash;
			unsigned voidManaMax;
			unsigned spiritManaMax;
			unsigned matterManaMax;
			float manaRegen;
		};

		BattleManager(const CharacterParams &leftCharacter, const CharacterParams &rightCharacter);
		void addHitStop(unsigned stop);
		void update();
		void render();
		void registerObject(IObject *object);
		void consumeEvent(const sf::Event &);
		template <typename T, typename ...Args>
		T *registerObject(Args &... args)
		{
			this->registerObject(new T(args...));
		}
	};
}


#endif //BATTLE_BATTLEMANAGER_HPP
