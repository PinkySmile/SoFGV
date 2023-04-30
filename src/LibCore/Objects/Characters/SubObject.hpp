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

	public:
		SubObject(unsigned id, bool owner);
		bool getOwner() const;
		unsigned int getId() const;
		unsigned int getClassId() const override;

		int getLayer() const override;
	};
}


#endif //SOFGV_SUBOBJECT_HPP
