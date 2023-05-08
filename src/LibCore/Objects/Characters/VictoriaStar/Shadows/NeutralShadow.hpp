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
			unsigned char _idleCounter;
		};
#pragma pack(pop)

		// Non-game state
		mutable FrameData _fakeDat;

		// Game state
		unsigned char _idleCounter = 0;

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

		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *pVoid, void *pVoid1) const override;
		void update() override;
		const FrameData *getCurrentFrameData() const override;

		static Shadow *create(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned int hp,
			bool direction,
			Vector2f pos,
			bool owner,
			unsigned int id,
			bool tint
		);

		void getHit(IObject &other, const FrameData *data) override;

		void activate() override;
	};
}


#endif //SOFGV_NEUTRALSHADOW_HPP
