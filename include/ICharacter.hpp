#ifndef BATTLE_ICHARACTER_HPP
#define BATTLE_ICHARACTER_HPP


#include <vector>
#include "Screen.hpp"
#include "Directions.hpp"
#include "AttackType.hpp"
#include "IHitObject.hpp"
#include "Point.hpp"
#include "Action.hpp"

class ICharacter {
public:
	virtual bool	weakAttack() = 0;
	virtual bool	meleeAttack() = 0;
	virtual Point	getPosition() = 0;
	virtual bool	strongAttack() = 0;
	virtual float	getSuper() const = 0;
	virtual float	getHealth() const = 0;
	virtual void	display(Screen &) const = 0;
	virtual void	takeKnockback(float factor) = 0;
	virtual std::vector<IProjectile *> update() = 0;
	virtual bool	setDirection(Directions::Directions) = 0;
	virtual void	setAttackType(AttackType::AttackType) = 0;
	virtual void	takeDamages(float nb, AttackType::AttackType) = 0;
	virtual const	std::vector<std::unique_ptr<IHitObject>> &getHixObjects() const = 0;
};


#endif //BATTLE_ICHARACTER_HPP
