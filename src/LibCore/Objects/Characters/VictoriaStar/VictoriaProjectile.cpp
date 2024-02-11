//
// Created by PinkySmile on 08/07/23.
//

#include "VictoriaProjectile.hpp"
#include "Shadow.hpp"

namespace SpiralOfFate
{
	VictoriaProjectile::VictoriaProjectile(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned team,
		bool direction,
		Vector2f pos,
		bool owner,
		class Character *ownerObj,
		unsigned id,
		const nlohmann::json &json,
		unsigned char typeSwitchFlags,
		unsigned debuffDuration
	) :
		Projectile(frameData, team, direction, pos, owner, ownerObj, id, json, typeSwitchFlags, debuffDuration)
	{
	}

	bool VictoriaProjectile::hits(const Object &other) const
	{
		auto old = this->_hasHit;
		auto t = const_cast<VictoriaProjectile *>(this);
		auto shadow = dynamic_cast<const Shadow *>(&other);

		if (shadow && shadow->getOwner() == this->_team)
			t->_hasHit = false;

		auto result = Object::hits(other);

		t->_hasHit = old;
		return result;
	}
}