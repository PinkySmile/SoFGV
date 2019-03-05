#ifndef BATTLE_IHITOBJECT_HPP
#define BATTLE_IHITOBJECT_HPP


#include "Screen.hpp"
#include "Quadra.hpp"

class ICharacter;
class IProjectile;

class IHitObject {
public:
	virtual void	display(Screen &) = 0;
	virtual void	changeBounds(Quadra &) = 0;
	virtual bool	isColliding(IHitObject *) = 0;
	virtual void	setDisabled(bool disabled = true) = 0;
	virtual void	applyEffects(IHitObject *, ICharacter *) = 0;
	virtual void	applyEffects(IHitObject *, IProjectile *) = 0;
};


#endif //BATTLE_IHITOBJECT_HPP
