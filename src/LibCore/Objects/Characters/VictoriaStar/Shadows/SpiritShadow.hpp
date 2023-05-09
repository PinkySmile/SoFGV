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
			unsigned char _idleCounter;
		};
#pragma pack(pop)

		// Game state
		unsigned char _idleCounter = 0;

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

		static Shadow *create(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned int hp,
			bool direction,
			Vector2f pos,
			bool owner,
			unsigned int id,
			bool tint
		);

		void update() override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *pVoid, void *pVoid1) const override;

		void render() const override;
	};
}


#endif //SOFGV_SPIRITSHADOW_HPP
