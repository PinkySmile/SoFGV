//
// Created by PinkySmile on 30/03/23.
//

#include "SubObject.hpp"
#include "Character.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	SubObject::SubObject(unsigned int id, bool owner, class Character *ownerObj) :
		_id(id),
		_owner(owner),
		_ownerObj(ownerObj)
	{
	}

	unsigned int SubObject::getClassId() const
	{
		return CLASS_ID;
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

	bool SubObject::hits(const Object &other) const
	{
		auto otherChr = dynamic_cast<const Character *>(&other);

		if (otherChr)
			for (auto limit : otherChr->getLimit())
				if (limit >= 100)
					return false;
		return Object::hits(other);
	}

	class Character *SubObject::getOwnerObj() const
	{
		return this->_ownerObj;
	}

	void SubObject::_applyNewAnimFlags()
	{
		if (!this->_newAnim)
			return;

		auto data = this->getCurrentFrameData();

		Object::_applyNewAnimFlags();
		if (data->subObjectSpawn > 0)
			this->_ownerObj->_spawnSubObject(*game->battleMgr, data->subObjectSpawn - 1, true);
	}
}