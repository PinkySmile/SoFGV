//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_IOBJECT_HPP
#define BATTLE_IOBJECT_HPP


#include "../Resources/FrameData.hpp"

namespace SpiralOfFate
{
	class BattleManager;

	class IObject {
	public:
		bool showBoxes = false;

		virtual ~IObject() = default;
		virtual void render() const = 0;
		virtual void update() = 0;
		virtual void reset() = 0;
		virtual bool isDead() const = 0;
		virtual bool hits(const IObject &other) const = 0;
		virtual bool collides(const IObject &other) const = 0;
		virtual void hit(IObject &other, const FrameData *data) = 0;
		virtual void getHit(IObject &other, const FrameData *data) = 0;
		virtual const FrameData *getCurrentFrameData() const = 0;
		virtual void collide(IObject &other) = 0;
		virtual void kill() = 0;
		virtual unsigned getBufferSize() const = 0;
		virtual void copyToBuffer(void *data) const = 0;
		virtual void restoreFromBuffer(void *data) = 0;
		virtual unsigned getClassId() const = 0;
	};
}


#endif //BATTLE_IOBJECT_HPP
