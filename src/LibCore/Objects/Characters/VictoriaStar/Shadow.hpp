//
// Created by PinkySmile on 29/03/23.
//

#ifndef SOFGV_SHADOW_HPP
#define SOFGV_SHADOW_HPP

#include "Objects/Characters/Projectile.hpp"

namespace SpiralOfFate
{
	class Shadow : public Projectile {
	public:
		Shadow(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned hp,
			bool direction,
			Vector2f pos,
			bool owner,
			unsigned id,
			const nlohmann::json &json
		);
	};
}

#endif //SOFGV_SHADOW_HPP
