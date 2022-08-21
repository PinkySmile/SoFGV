//
// Created by PinkySmile on 15/08/2022.
//

#include "StageObject.hpp"

SpiralOfFate::StageObject::StageObject(nlohmann::json &json)
{
	this->_moves = FrameData::loadFile(json["framedata"]);
	this->_action = json["action"];
	this->_position = {json["x"], json["y"]};
	this->_team = json["team"];
	this->_direction = json["dir"];
	this->_dir = this->_direction ? 1 : -1;
}
