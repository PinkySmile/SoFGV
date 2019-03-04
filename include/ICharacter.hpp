#ifndef BATTLE_ICHARACTER_HPP
#define BATTLE_ICHARACTER_HPP


#include <vector>
#include "Screen.hpp"
#include "Directions.hpp"
#include "AttackType.hpp"
#include "IHitObject.hpp"

class ICharacter {
public:
	virtual float	getHealth() = 0;
	virtual void	display(Screen &) = 0;
	virtual bool	weakAttack(AttackType::AttackType) = 0;
	virtual bool	meleeAttack(AttackType::AttackType) = 0;
	virtual bool	setDirection(Directions::Directions) = 0;
	virtual bool	strongAttack(AttackType::AttackType) = 0;
	virtual void	takeDamages(float nb, AttackType::AttackType) = 0;
	virtual bool	checkCollisions(std::vector<IHitObject *> &objects) = 0;
};


#endif //BATTLE_ICHARACTER_HPP
