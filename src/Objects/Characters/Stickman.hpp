//
// Created by Gegel85 on 01/03/2022.
//

#ifndef BATTLE_STICKMAN_HPP
#define BATTLE_STICKMAN_HPP


#include "../Character.hpp"

namespace Battle
{
	class Stickman : public Character {
	public:
		Stickman() = default;
		Stickman(const std::string &frameData, const std::string &subobjFrameData, const std::pair<std::vector<Color>, std::vector<Color>> &palette, std::shared_ptr<IInput> input);

		unsigned int getClassId() const override;
	};
}


#endif //BATTLE_STICKMAN_HPP
