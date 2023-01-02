//
// Created by PinkySmile on 02/02/2022.
//

#ifndef SOFGV_PROJECTILE_HPP
#define SOFGV_PROJECTILE_HPP


#include "Object.hpp"
#include "Character.hpp"

namespace SpiralOfFate
{
	class Projectile : public Object {
	private:
		struct Data {
			unsigned maxHit;
			unsigned animationCtr;
			bool disabled;
		};

		// Game State
		bool _disabled = false;
		unsigned _animationCtr = 0;
		unsigned _nbHit = 0;

		// Non Game state
		unsigned _maxHit;
		unsigned _id;
		unsigned _endBlock;
		bool _owner;
		bool _loop;
		bool _disableOnHit;
		unsigned _animationData;
		Character::ProjectileAnimation _anim;

	protected:
		void _onMoveEnd(const FrameData &lastData) override;
		void _disableObject();

	public:
		Projectile(
			bool owner,
			unsigned id,
			unsigned maxHit,
			bool loop,
			unsigned endBlock,
			bool disableOnHit,
			unsigned animationData,
			Character::ProjectileAnimation anim
		);
		Projectile(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned team,
			bool direction,
			Vector2f pos,
			bool owner,
			unsigned id,
			unsigned maxHit,
			bool loop,
			unsigned endBlock,
			bool disableOnHit,
			unsigned animationData,
			Character::ProjectileAnimation anim
		);
		bool isDead() const override;
		void update() override;
		unsigned int getClassId() const override;
		bool getOwner() const;
		unsigned int getId() const;
		void hit(IObject &other, const FrameData *data) override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *pVoid, void *pVoid1) const override;
		void getHit(IObject &other, const FrameData *data) override;
		bool hits(const IObject &other) const override;
	};
}


#endif //SOFGV_PROJECTILE_HPP
