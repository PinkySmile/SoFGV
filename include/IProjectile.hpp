#ifndef BATTLE_IPROJECTILE_HPP
#define BATTLE_IPROJECTILE_HPP


#include "Screen.hpp"

class IProjectile {
public:
	virtual void	display(Screen &) = 0;
};


#endif //BATTLE_IPROJECTILE_HPP
