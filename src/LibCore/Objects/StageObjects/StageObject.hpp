//
// Created by andgel on 15/08/22
//

#ifndef SOFGV_STAGEOBJECT_HPP
#define SOFGV_STAGEOBJECT_HPP


#include "nlohmann/json.hpp"
#include "../Object.hpp"

namespace SpiralOfFate
{
	class StageObject : public Object {
	public:
		StageObject(nlohmann::json &json);
	};
}


#endif //SOFGV_STAGEOBJECT_HPP
