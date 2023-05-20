//
// Created by PinkySmile on 06/05/23.
//

#ifndef SOFGV_SPIRITSHADOW_HPP
#define SOFGV_SPIRITSHADOW_HPP


#include "Objects/Characters/VictoriaStar/Shadow.hpp"

namespace SpiralOfFate
{
	class SpiritShadow : public Shadow {
	private:
#pragma pack(push, 1)
		struct Data {
		};
#pragma pack(pop)

		// Game state

	public:
		SpiritShadow(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned hp,
			bool direction,
			Vector2f pos,
			bool owner,
			unsigned id,
			bool tint
		);

		void update() override;
		void render() const override;

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


#endif //SOFGV_SPIRITSHADOW_HPP
