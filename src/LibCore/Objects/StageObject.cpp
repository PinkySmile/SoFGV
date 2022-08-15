//
// Created by andgel on 15/08/22
//

#include "StageObject.hpp"

SpiralOfFate::StageObject::StageObject(nlohmann::json &json)
{
	this->_moves = FrameData::loadFile(json["framedata"]);
	this->_action = json["action"];
	this->_position = {json["x"], json["y"]};
	this->_team = json["team"];
}
