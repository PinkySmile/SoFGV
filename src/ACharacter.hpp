//
// Created by Gegel85 on 04/03/2019.
//

#ifndef BATTLE_ACHARACTER_HPP
#define BATTLE_ACHARACTER_HPP


#include "ICharacter.hpp"

class ACharacter : public ICharacter {
protected:
	std::vector<std::unique_ptr<IHitObject>>	_hitboxes = {};
	Directions::Directions	_directions = Directions::None;
	AttackType::AttackType	_atkType = AttackType::Neutral;

	//States
	unsigned	_stunnedTime = 0;
	sf::Vector2f	_velocity = {0, 0};
	sf::Vector2f	_pos;
	float	_guardTime = 0;
	float	_super;
	float	_life;

	//Stats
	float	_groundSpeed;
	float	_airSpeed;
	float	_baseSuper;
	float	_baseLife;

	template<typename vector>
	bool	_isBlocking(const vector &pos) const;
	void	_move();

public:
	static const int FLIGHT_TEXTURE_OFFSET = 64;

	enum State {
		IDLE,									//0000
		JUMP,									//0001
		CROUCH,									//0010
		HIT,									//0011
		RUN,									//0100
		MELEE,									//0101
		WEAK,									//0110
		STRONG,									//0111
		TRANSFORM,								//1000
		REALLY_REALLY_SPECIAL_STATE_WOW_SO_SO_GOOD_I_LOVE_IT_SO_MUCH_OMG,	//1001
		STUNNED,								//1010
		LONG_STUNNED,								//1011
		BLOCKING,								//1100
		START = 1 << 4,
		END = 1 << 5,
	};

	ACharacter(float lifeMax, float superMax);
	float	getSuper() const override;
	float	getHealth() const override;
	void	takeDamages(float) override;
	sf::Vector2f getPos() const override;
	void	display(Screen &) const override;
	void	setPos(sf::Vector2f pos) override;
	void	setDirection(Directions::Directions) override;
	void	setAttackType(AttackType::AttackType) override;
	void	takeKnockback(Directions::Directions, sf::Vector2f, unsigned) override;
	const	std::vector<std::unique_ptr<IHitObject>> &getHixObjects() const override;
	bool	isBlockingAttack(AttackType::AttackType type, ICharacter &enemy) const override;
};


#endif //BATTLE_ACHARACTER_HPP
