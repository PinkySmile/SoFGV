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
			class Character *ownerObj,
			unsigned id,
			bool tint
		);

		void update() override;
		void getHit(Object &other, const FrameData *data) override;
		void activate() override;

		static Shadow *create(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned int hp,
			bool direction,
			Vector2f pos,
			bool owner,
			class Character *ownerObj,
			unsigned int id,
			bool tint
		);

	protected:
		void _computeFrameDataCache() override;
	};
}


#endif //SOFGV_NEUTRALSHADOW_HPP
