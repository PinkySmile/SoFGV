//
// Created by Gegel85 on 02/02/2022.
//

#ifndef BATTLE_APROJECTILE_HPP
#define BATTLE_APROJECTILE_HPP


#include "AObject.hpp"

namespace Battle
{
	class AProjectile : public AObject {
	public:
		AProjectile(const std::vector<std::vector<FrameData>> &frameData, unsigned team);
	};
}


#endif //BATTLE_APROJECTILE_HPP
