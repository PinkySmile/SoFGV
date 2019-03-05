#ifndef BATTLE_IPROJECTILE_HPP
#define BATTLE_IPROJECTILE_HPP


#include "Screen.hpp"
#include "IHitObject.hpp"

class IProjectile {
public:
	virtual void	die() = 0;
	virtual bool	isDead() = 0;
	virtual void	update() = 0;
	virtual void	display(Screen &) = 0;
	virtual const	std::vector<std::unique_ptr<IHitObject>> &getHoxObjects() const = 0;
};


#endif //BATTLE_IPROJECTILE_HPP
