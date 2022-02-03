//
// Created by Gegel85 on 02/02/2022.
//

#include "AProjectile.hpp"

namespace Battle
{
	AProjectile::AProjectile(const std::vector<std::vector<FrameData>> &frameData, unsigned team)
	{
		this->_team = team;
		this->_moves[0] = frameData;
	}
}
