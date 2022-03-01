//
// Created by Gegel85 on 01/03/2022.
//

#include "Stickman.hpp"
#include "../../Resources/Game.hpp"

namespace Battle
{
	Stickman::Stickman(
		const std::string &frameData,
		const std::string &subobjFrameData,
		const std::pair<std::vector<Battle::Color>,
		std::vector<Battle::Color>> &palette,
		std::shared_ptr<IInput> input
	) :
		Character(frameData, subobjFrameData, palette, input)
	{
		game.logger.debug("Stickman class created");
	}

	unsigned int Stickman::getClassId() const
	{
		return 3;
	}
}
