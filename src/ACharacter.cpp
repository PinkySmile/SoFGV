//
// Created by Gegel85 on 09/03/2019.
//

#include <ACharacter.hpp>
#include <IProjectile.hpp>

ACharacter::ACharacter(float lifeMax, float superMax) :
	_baseLife(lifeMax),
	_baseSuper(superMax),
	_life(lifeMax),
	_super(superMax)
{
}

void	ACharacter::display(Screen &) const
{

}

float	ACharacter::getHealth() const
{
	return this->_life / this->_baseLife;
}

float	ACharacter::getSuper() const
{
	return this->_super / this->_baseSuper;
}

const	std::vector<std::unique_ptr<IHitObject>>& ACharacter::getHixObjects() const
{
	return this->_hitboxes;
}

void	ACharacter::setAttackType(AttackType::AttackType type)
{
	this->_atkType = type;
}

void	ACharacter::setDirection(Directions::Directions dir)
{
	this->_directions = dir;
}

bool	ACharacter::isBlockingAttack(AttackType::AttackType type, ICharacter &enemy) const
{
	return this->_isBlocking<sf::Vector2f>(enemy.getPos()) && (type & this->_atkType);
}

void	ACharacter::takeDamages(float dmg)
{
	this->_life -= dmg;
}

void	ACharacter::takeKnockback(Directions::Directions dir, sf::Vector2f factor, unsigned duration)
{
	this->_stunnedTime = duration;
	if (LeftDir(dir))
		this->_velocity.x = -factor.x;
	else if (RightDir(dir))
		this->_velocity.x = factor.x;
	else
		this->_velocity.x = 0;

	if (UpDir(dir))
		this->_velocity.y = -factor.y;
	else if (DownDir(dir))
		this->_velocity.y = factor.y;
	else
		this->_velocity.y = 0;
}

void	ACharacter::_move()
{
	if (LeftDir(this->_directions))
		this->_pos.x -= this->_groundSpeed;
	else if (RightDir(this->_directions))
		this->_pos.x += this->_groundSpeed;
	if (UpDir(this->_directions))
		this->_pos.y -= this->_groundSpeed;
	else if (DownDir(this->_directions))
		this->_pos.y += this->_groundSpeed;
}

sf::Vector2f ACharacter::getPos() const
{
	return this->_pos;
}

void	ACharacter::setPos(sf::Vector2f pos)
{
	this->_pos = pos;
}

template<typename vector>
bool	ACharacter::_isBlocking(const vector &pos) const
{
	Directions::Directions dir = Directions::None;

	if (this->_pos.x > pos.x)
		dir = static_cast<Directions::Directions>(dir | Directions::Right);
	else if (this->_pos.x < pos.x)
		dir = static_cast<Directions::Directions>(dir | Directions::Left);
	return ((LeftDir(dir) && RightDir(this->_directions)) || (RightDir(dir) && LeftDir(this->_directions)));
}