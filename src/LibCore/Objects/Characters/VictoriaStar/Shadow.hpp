//
// Created by PinkySmile on 29/03/23.
//

#ifndef SOFGV_SHADOW_HPP
#define SOFGV_SHADOW_HPP

#include "Objects/Characters/Projectile.hpp"

namespace SpiralOfFate
{
	class Shadow : public SubObject {
	protected:
		enum AnimationBlock {
			ANIMBLOCK_SPAWNING,
			ANIMBLOCK_IDLE,
			ANIMBLOCK_DYING,
			ANIMBLOCK_RESURRECT,
			ANIMBLOCK_NORMAL_ACTIVATED,
			ANIMBLOCK_NORMAL_ATTACKING,
			ANIMBLOCK_MATTER_ACTIVATED,
			ANIMBLOCK_MATTER_ATTACKING,
			ANIMBLOCK_SPIRIT_ACTIVATED,
			ANIMBLOCK_SPIRIT_ATTACKING,
			ANIMBLOCK_VOID_ACTIVATED,
			ANIMBLOCK_VOID_ATTACKING,
		};

		void _onMoveEnd(const FrameData &lastData) override;
		virtual void _die();

		// Non game state
		unsigned _activateBlock;

		// Game state
		unsigned invincibleTime;

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

		virtual void activate();
		void setInvincible(unsigned time);
		void getHit(IObject &other, const FrameData *data) override;
	};
}

#endif //SOFGV_SHADOW_HPP
