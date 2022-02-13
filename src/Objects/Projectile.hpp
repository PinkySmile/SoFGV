//
// Created by Gegel85 on 02/02/2022.
//

#ifndef BATTLE_PROJECTILE_HPP
#define BATTLE_PROJECTILE_HPP


#include "Object.hpp"

namespace Battle
{
	class Projectile : public Object {
	private:
		bool owner;
		unsigned id;

	public:
		Projectile(bool owner, unsigned id);
		Projectile(const std::vector<std::vector<FrameData>> &frameData, unsigned team, bool direction, Vector2f pos, bool owner, unsigned id);
		bool isDead() const override;
		void update() override;
		unsigned int getClassId() const override;
		bool getOwner() const;
		unsigned int getId() const;
	};
}


#endif //BATTLE_PROJECTILE_HPP
