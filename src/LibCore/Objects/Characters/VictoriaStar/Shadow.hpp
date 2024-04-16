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
			unsigned _boxSize;
			bool _killedByOwner;
			unsigned char _invincibleTime;
			unsigned short _idleCounter;
			std::pair<unsigned char, unsigned char> _loopInfo;
		};
		static_assert(sizeof(Data) == 10, "Data has wrong size");
#pragma pack(pop)

		enum AnimationBlock {
			ANIMBLOCK_SPAWNING,
			ANIMBLOCK_IDLE,
			ANIMBLOCK_DYING,
			ANIMBLOCK_RESURRECT,
			ANIMBLOCK_NORMAL_ACTIVATED,
			ANIMBLOCK_SPIRIT_ACTIVATED,
			ANIMBLOCK_VOID_ACTIVATED,
			ANIMBLOCK_MATTER_ACTIVATED,
		};

		void _onMoveEnd(const FrameData &lastData) override;
		virtual void _die();

		// Non-game state
		unsigned _activateBlock;

		// Game state
		unsigned _invincibleTime = 0;
		unsigned _boxSize = 0;
		bool _killedByOwner = false;
		unsigned short _idleCounter = 0;
		std::pair<unsigned char, unsigned char> _loopInfo = {0, 0};

		void _applyNewAnimFlags() override;
		void _computeFrameDataCache() override;

	public:
		Shadow(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned hp,
			bool direction,
			Vector2f pos,
			bool owner,
			class Character *ownerObj,
			unsigned id,
			const sf::Color &tint,
			unsigned activateBlock
		);

		virtual void activate();

		bool wasKilledByOwner() const;
		void setInvincible(unsigned time);
		float getCurrentPoints() const;
		void update() override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const override;
		size_t printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const override;
		void getHit(Object &other, const FrameData *data) override;
		int getLayer() const override;
	};
}


#endif //SOFGV_SHADOW_HPP
