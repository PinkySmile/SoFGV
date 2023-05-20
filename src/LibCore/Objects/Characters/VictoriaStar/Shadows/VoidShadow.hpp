//
// Created by PinkySmile on 06/05/23.
//

#ifndef SOFGV_VOIDSHADOW_HPP
#define SOFGV_VOIDSHADOW_HPP


#include "Objects/Characters/VictoriaStar/Shadow.hpp"

namespace SpiralOfFate
{
	class VoidShadow : public Shadow {
	protected:
#pragma pack(push, 1)
		struct Data {
			bool _attacking;
		};
#pragma pack(pop)

		bool _attacking = false;

		void _onMoveEnd(const FrameData &lastData) override;

	public:
		VoidShadow(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned hp,
			bool direction,
			Vector2f pos,
			bool owner,
			unsigned id,
			bool tint
		);

		void update() override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *pVoid, void *pVoid1) const override;

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


#endif //SOFGV_VOIDSHADOW_HPP
