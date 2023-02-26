//
// Created by PinkySmile on 26/02/23.
//

#include "VictoriaStar.hpp"
#include "Resources/Game.hpp"

#define ACTION_SCREEN_TELEPORT 368

namespace SpiralOfFate
{
	VictoriaStar::VictoriaStar(
		unsigned int index,
		const std::string &folder,
		const std::pair<std::vector<Color>, std::vector<Color>> &palette,
		std::shared_ptr<IInput> input
	) :
		Character(index, folder, palette, std::move(input))
	{
		game->logger.debug("VictoriaStar class created");
	}

	bool VictoriaStar::_startMove(unsigned int action)
	{
		if (action == ACTION_BACKWARD_DASH && (this->_position.x == 0 || this->_position.x == 1000))
			return Object::_startMove(ACTION_SCREEN_TELEPORT);
		return Object::_startMove(action);
	}

	unsigned int VictoriaStar::getClassId() const
	{
		return 4;
	}

	unsigned int VictoriaStar::getBufferSize() const
	{
		return Character::getBufferSize() + sizeof(Data);
	}

	void VictoriaStar::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());

		Character::copyToBuffer(data);
		game->logger.verbose("Saving VictoriaStar (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
		//dat->_buffTimer = this->_buffTimer;
	}

	void VictoriaStar::restoreFromBuffer(void *data)
	{
		Character::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());

		//this->_buffTimer = dat->_buffTimer;
		game->logger.verbose("Restored VictoriaStar @" + std::to_string((uintptr_t)dat));
	}
}