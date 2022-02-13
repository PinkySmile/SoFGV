//
// Created by Gegel85 on 02/02/2022.
//

#include "Projectile.hpp"

namespace Battle
{
	Projectile::Projectile(bool owner, unsigned int id) :
		owner(owner),
		id(id)
	{
	}

	Projectile::Projectile(const std::vector<std::vector<FrameData>> &frameData, unsigned team, bool direction, Vector2f pos, bool owner, unsigned id) :
		Projectile(owner, id)
	{
		this->_position = pos;
		this->_dir = direction ? 1 : -1;
		this->_direction = direction;
		this->_team = team;
		this->_moves[0] = frameData;
	}

	bool Projectile::isDead() const
	{
		return Object::isDead() || this->_hasHit;
	}

	void Projectile::update()
	{
		Object::update();
		this->_dead |=
			this->_position.x < -300 ||
			this->_position.x > 1300 ||
			this->_position.y < -300 ||
			this->_position.y > 1300;
	}

	unsigned int Projectile::getClassId() const
	{
		return 2;
	}

	bool Projectile::getOwner() const
	{
		return owner;
	}

	unsigned int Projectile::getId() const
	{
		return id;
	}
}
