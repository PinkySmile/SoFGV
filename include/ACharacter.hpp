//
// Created by Gegel85 on 04/03/2019.
//

#ifndef BATTLE_ACHARACTER_HPP
#define BATTLE_ACHARACTER_HPP


#include "ICharacter.hpp"

class ACharacter : ICharacter {
protected:


public:
	enum State {
		IDLE,
		START_WALK,
		WALK,
		END_WALK,
		START_RUN,
		RUN,
		END_RUN,
		START_MELEE,
		MELEE,
		END_MELEE,
		START_WEAK,
		WEAK,
		END_WEAK,
		START_STRONG,
		STRONG,
		END_STRONG,
		START_TRANSFORM,
		TRANSFORM,
		END_TRANSFORM,
		STUNNED,
		LONG_STUNNED,
	};

	virtual float	getHealth() const;
	virtual void	display(Screen &) const;
	virtual bool	setDirection(Directions::Directions);
	virtual void	setAttackType(AttackType::AttackType);
	virtual void	takeDamages(float nb, AttackType::AttackType);
	virtual const	std::vector<std::unique_ptr<IHitObject>> &getHixObjects() const;
};


#endif //BATTLE_ACHARACTER_HPP
