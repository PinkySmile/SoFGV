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
		AProjectile(const std::vector<std::vector<FrameData>> &frameData, unsigned team, bool direction, Vector2f pos);
		bool isDead() const override;
		void update() override;
	};
}


#endif //BATTLE_APROJECTILE_HPP
