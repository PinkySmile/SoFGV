//
// Created by PinkySmile on 29/03/23.
//

#include "Shadow.hpp"

namespace SpiralOfFate
{
	Shadow::Shadow(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		const nlohmann::json &json
	) :
		Projectile(
			frameData,
			2,
			direction,
			pos,
			owner,
			id,
			json
		)
	{
		this->_hp = this->_baseHp = hp;
	}
}