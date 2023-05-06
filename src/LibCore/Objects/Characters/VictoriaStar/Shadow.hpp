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
			ANIMBLOCK_ACTIVATED,
			ANIMBLOCK_ATTACKING,
		};

		void _onMoveEnd(const FrameData &lastData) override;
		virtual void _die();

		unsigned _activateBlock;

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

		void getHit(IObject &other, const FrameData *data) override;
	};
}

#endif //SOFGV_SHADOW_HPP
