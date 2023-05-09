//
// Created by PinkySmile on 29/03/23.
//

#ifndef SOFGV_SHADOW_HPP
#define SOFGV_SHADOW_HPP


#include "Objects/Characters/SubObject.hpp"

namespace SpiralOfFate
{
	class Shadow : public SubObject {
	protected:
#pragma pack(push, 1)
		struct Data {
			unsigned char _invincibleTime;
			unsigned _boxSize;
			std::pair<unsigned char, unsigned char> _loopInfo;
		};
#pragma pack(pop)

		enum AnimationBlock {
			ANIMBLOCK_SPAWNING,
			ANIMBLOCK_IDLE,
			ANIMBLOCK_DYING,
			ANIMBLOCK_RESURRECT,
			ANIMBLOCK_NORMAL_ACTIVATED,
			ANIMBLOCK_SPIRIT_ACTIVATED,
			ANIMBLOCK_MATTER_ACTIVATED,
			ANIMBLOCK_MATTER_ATTACKING,
			ANIMBLOCK_VOID_ACTIVATED,
			ANIMBLOCK_VOID_ATTACKING,
		};

		void _onMoveEnd(const FrameData &lastData) override;
		virtual void _die();

		// Non game state
		unsigned _activateBlock;
		mutable FrameData _fakeData;

		// Game state
		unsigned _invincibleTime = 0;
		unsigned _boxSize = 0;
		std::pair<unsigned char, unsigned char> _loopInfo = {0, 0};

		void _applyNewAnimFlags() override;

	public:
		Shadow(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned hp,
			bool direction,
			Vector2f pos,
			bool owner,
			unsigned id,
			const sf::Color &tint,
			unsigned activateBlock
		);

		void update() override;
		const FrameData *getCurrentFrameData() const override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *pVoid, void *pVoid1) const override;
		virtual void activate();
		void setInvincible(unsigned time);
		void getHit(IObject &other, const FrameData *data) override;
		int getLayer() const override;
	};
}


#endif //SOFGV_SHADOW_HPP
