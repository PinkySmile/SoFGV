//
// Created by PinkySmile on 30/03/23.
//

#include "SubObject.hpp"
#include "Objects/Character.hpp"

namespace SpiralOfFate
{
	SubObject::SubObject(unsigned int id, bool owner) :
		_id(id),
		_owner(owner)
	{
	}

	unsigned int SubObject::getClassId() const
	{
		return 2;
	}

	bool SubObject::getOwner() const
	{
		return this->_owner;
	}

	unsigned int SubObject::getId() const
	{
		return this->_id;
	}

	int SubObject::getLayer() const
	{
		return 100;
	}

	bool SubObject::hits(const IObject &other) const
	{
		auto otherChr = dynamic_cast<const Character *>(&other);

		if (otherChr)
			for (auto limit : otherChr->getLimit())
				if (limit >= 100)
					return false;
		return Object::hits(other);
	}
}