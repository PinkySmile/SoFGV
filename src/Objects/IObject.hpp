//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_IOBJECT_HPP
#define BATTLE_IOBJECT_HPP


#include "../Resources/FrameData.hpp"

namespace Battle
{
	class IObject {
	public:
		virtual ~IObject() = default;
		virtual void render() const = 0;
		virtual void update() = 0;
		virtual void reset() = 0;
		virtual bool isDead() const = 0;
		virtual bool hits(IObject &other) const = 0;
		virtual bool collides(IObject &other) const = 0;
		virtual void hit(IObject &other, const FrameData *data) = 0;
		virtual void getHit(IObject &other, const FrameData *data) = 0;
		virtual const FrameData *getCurrentFrameData() const = 0;
		virtual void collide(IObject &other) = 0;
	};
}


#endif //BATTLE_IOBJECT_HPP
