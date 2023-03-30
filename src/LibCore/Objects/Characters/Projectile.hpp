//
// Created by PinkySmile on 02/02/2022.
//

#ifndef SOFGV_PROJECTILE_HPP
#define SOFGV_PROJECTILE_HPP


#include "Objects/Character.hpp"
#include "SubObject.hpp"

namespace SpiralOfFate
{
	class Projectile : public SubObject {
	private:
		enum ProjectileAnimation {
			ANIMATION_DISAPPEAR,
			ANIMATION_FADE,
			ANIMATION_BLOCK
		};

		struct Data {
			unsigned nbHit;
			unsigned animationCtr;
			bool disabled;
		};

		// Game State
		bool _disabled = false;
		unsigned _animationCtr = 0;
		unsigned _nbHit = 0;

		// Non Game state
		unsigned _maxHit;
		unsigned _endBlock;
		bool _loop;
		bool _disableOnHit;
		unsigned _animationData;
		ProjectileAnimation _anim;

	protected:
		void _onMoveEnd(const FrameData &lastData) override;
		void _disableObject();

	public:
		Projectile(
			bool owner,
			unsigned id,
			const nlohmann::json &json
		);
		Projectile(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned team,
			bool direction,
			Vector2f pos,
			bool owner,
			unsigned id,
			const nlohmann::json &json
		);
		bool isDead() const override;
		void update() override;
		void hit(IObject &other, const FrameData *data) override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *pVoid, void *pVoid1) const override;
		void getHit(IObject &other, const FrameData *data) override;
		bool hits(const IObject &other) const override;

		static ProjectileAnimation animationFromString(const std::string &str);
	};
}


#endif //SOFGV_PROJECTILE_HPP
