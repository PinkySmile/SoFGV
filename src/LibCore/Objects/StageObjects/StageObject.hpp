//
// Created by PinkySmile on 15/08/2022.
//

#ifndef SOFGV_STAGEOBJECT_HPP
#define SOFGV_STAGEOBJECT_HPP


#include "nlohmann/json.hpp"
#include "Objects/Object.hpp"

namespace SpiralOfFate
{
	class StageObject : public Object {
	public:
		StageObject(nlohmann::json &json);
	};
}


#endif //SOFGV_STAGEOBJECT_HPP
