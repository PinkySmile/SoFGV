//
// Created by Gegel85 on 02/02/2022.
//

#ifndef BATTLE_APROJECTILE_HPP
#define BATTLE_APROJECTILE_HPP


#include "AObject.hpp"

namespace Battle
{
	class AProjectile : public AObject {
	private:
		bool owner;
		unsigned id;

	public:
		AProjectile(bool owner, unsigned id);
		AProjectile(const std::vector<std::vector<FrameData>> &frameData, unsigned team, bool direction, Vector2f pos, bool owner, unsigned id);
		bool isDead() const override;
		void update() override;
		unsigned int getClassId() const override;
		bool getOwner() const;
		unsigned int getId() const;
	};
}


#endif //BATTLE_APROJECTILE_HPP
