//
// Created by PinkySmile on 08/07/23.
//

#ifndef SOFGV_VICTORIAPROJECTILE_HPP
#define SOFGV_VICTORIAPROJECTILE_HPP


#include "Objects/Characters/Projectile.hpp"

namespace SpiralOfFate
{
	class VictoriaProjectile : public Projectile {
	public:
		VictoriaProjectile(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned team,
			bool direction,
			Vector2f pos,
			bool owner,
			class Character *ownerObj,
			unsigned id,
			const nlohmann::json &json,
			unsigned char typeSwitchFlags
		);
		bool hits(const Object &other) const override;
	};
}


#endif //SOFGV_VICTORIAPROJECTILE_HPP
