//
// Created by Gegel85 on 02/02/2022.
//

#include "AProjectile.hpp"

namespace Battle
{
	AProjectile::AProjectile(const std::vector<std::vector<FrameData>> &frameData, unsigned team, bool direction, Vector2f pos)
	{
		this->_position = pos;
		this->_dir = direction ? 1 : -1;
		this->_direction = direction;
		this->_team = team;
		this->_moves[0] = frameData;
	}

	bool AProjectile::isDead() const
	{
		return AObject::isDead() || this->_hasHit;
	}

	void AProjectile::update()
	{
		AObject::update();
		this->_dead |=
			this->_position.x < -300 ||
			this->_position.x > 1300 ||
			this->_position.y < -300 ||
			this->_position.y > 1300;
	}
}
