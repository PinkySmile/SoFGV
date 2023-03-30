//
// Created by PinkySmile on 26/02/23.
//

#include "VictoriaStar.hpp"
#include "Resources/Game.hpp"
#include "Objects/Characters/VictoriaStar/Shadow.hpp"

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

	std::pair<unsigned int, std::shared_ptr<IObject>> VictoriaStar::_spawnSubObject(unsigned int id, bool needRegister)
	{
		my_assert2(this->_projectileData.find(id) != this->_projectileData.end(), "Cannot find subobject " + std::to_string(id));

		auto &pdat = this->_projectileData[id];
		bool dir = this->_getProjectileDirection(pdat);

		if (!pdat.json.contains("shadow"))
			return Character::_spawnSubObject(id, needRegister);
		try {
			std::string shadow = pdat.json["shadow"];

			if (shadow == "matter")
				return game->battleMgr->registerObject<Shadow>(
					needRegister,
					this->_subObjectsData.at(pdat.action),
					pdat.json["hp"],
					dir,
					this->_calcProjectilePosition(pdat, dir ? 1 : -1),
					this->_team,
					id,
					pdat.json
				);
			if (shadow == "void")
				return game->battleMgr->registerObject<Shadow>(
					needRegister,
					this->_subObjectsData.at(pdat.action),
					pdat.json["hp"],
					dir,
					this->_calcProjectilePosition(pdat, dir ? 1 : -1),
					this->_team,
					id,
					pdat.json
				);
			throw std::invalid_argument("Invalid shadow type " + shadow);
		} catch (std::out_of_range &e) {
			throw std::invalid_argument("Cannot find subobject id " + std::to_string(id));
		}
	}
}