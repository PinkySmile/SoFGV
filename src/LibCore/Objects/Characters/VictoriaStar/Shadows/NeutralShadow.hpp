//
// Created by PinkySmile on 06/05/23.
//

#ifndef SOFGV_NEUTRALSHADOW_HPP
#define SOFGV_NEUTRALSHADOW_HPP


#include "Objects/Characters/VictoriaStar/Shadow.hpp"

namespace SpiralOfFate
{
	class NeutralShadow : public Shadow {
	private:
#pragma pack(push, 1)
		struct Data {
		};
#pragma pack(pop)

		// Non-game state

		// Game state

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

		void update() override;
		void getHit(IObject &other, const FrameData *data) override;
		void activate() override;

		static Shadow *create(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned int hp,
			bool direction,
			Vector2f pos,
			bool owner,
			unsigned int id,
			bool tint
		);

	protected:
		void _computeFrameDataCache() override;
	};
}


#endif //SOFGV_NEUTRALSHADOW_HPP
