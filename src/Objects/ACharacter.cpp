//
// Created by PinkySmile on 18/09/2021
//

#include "ACharacter.hpp"
#include <iostream>
namespace Battle
{
	ACharacter::ACharacter(const std::string &frameData)
	{
		this->_moves = FrameData::loadFile(frameData);

		nlohmann::json j = nlohmann::json::object();

		for (auto &[key, value] : this->_moves) {
			j["action"] = key;
			j["framedata"] = value;
		}
		std::cout << j.dump(4) << std::endl;
	}

	void ACharacter::render() const
	{
		AObject::render();
	}

	void ACharacter::update()
	{
		AObject::update();
	}
}
