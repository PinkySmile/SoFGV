//
// Created by PinkySmile on 06/05/23.
//

#ifndef SOFGV_NEUTRALSHADOW_HPP
#define SOFGV_NEUTRALSHADOW_HPP


#include "Objects/Characters/VictoriaStar/Shadow.hpp"

namespace SpiralOfFate
{
	class NeutralShadow : public Shadow {
	public:
		NeutralShadow(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned hp,
			bool direction,
			Vector2f pos,
			bool owner,
			unsigned id,
			bool tint
		);

		static Shadow *create(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned int hp,
			bool direction,
			Vector2f pos,
			bool owner,
			unsigned int id,
			bool tint
		);
	};
}


#endif //SOFGV_NEUTRALSHADOW_HPP
