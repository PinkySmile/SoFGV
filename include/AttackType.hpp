#ifndef BATTLE_ATTACKTYPE_HPP
#define BATTLE_ATTACKTYPE_HPP


namespace AttackType {
	enum AttackType {
		Neutral	= 0,
		Spirit	= 1 << 0,
		Void	= 1 << 1,
		Matter	= 1 << 2,
	};
}

#endif //BATTLE_ATTACKTYPE_HPP
