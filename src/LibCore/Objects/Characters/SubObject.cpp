//
// Created by PinkySmile on 30/03/23.
//

#include "SubObject.hpp"

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
		return 1;
	}
}