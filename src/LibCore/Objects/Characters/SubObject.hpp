//
// Created by PinkySmile on 30/03/23.
//

#ifndef SOFGV_SUBOBJECT_HPP
#define SOFGV_SUBOBJECT_HPP


#include "Objects/Object.hpp"

namespace SpiralOfFate
{
	class SubObject : public Object {
	private:
		unsigned _id;
		bool _owner;
		class Character *_ownerObj;

	public:
		SubObject(unsigned id, bool owner, class Character *ownerObj);

		bool hits(const IObject &other) const override;

		bool getOwner() const;
		class Character *getOwnerObj() const;
		unsigned int getId() const;
		unsigned int getClassId() const override;
		int getLayer() const override;

	protected:
		void _tickMove() override;
	};
}


#endif //SOFGV_SUBOBJECT_HPP
