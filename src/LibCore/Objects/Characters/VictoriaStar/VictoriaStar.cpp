//
// Created by PinkySmile on 26/02/23.
//

#include "VictoriaStar.hpp"
#include "Resources/Game.hpp"
#include "Objects/Characters/VictoriaStar/Shadow.hpp"

#define ACTION_SCREEN_TELEPORT 368
#define ACTION_HAPPY_BUTTERFLY 4
#define ACTION_WEIRD_BUTTERFLY 5
#define WEIRD_BUTTERFLIES_START_ID (1000 + NB_BUTTERFLIES)
#define BUTTERFLIES_END_ID (1000 + NB_BUTTERFLIES * 2)

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
		// Butterflies actions
		my_assert(this->_subObjectsData.find(ACTION_HAPPY_BUTTERFLY) != this->_subObjectsData.end());
		my_assert(this->_subObjectsData.find(ACTION_WEIRD_BUTTERFLY) != this->_subObjectsData.end());
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
		dat->_hitShadow = this->_hitShadow;
	}

	void VictoriaStar::restoreFromBuffer(void *data)
	{
		Character::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());

		this->_hitShadow = dat->_hitShadow;
		game->logger.verbose("Restored VictoriaStar @" + std::to_string((uintptr_t)dat));
	}

	std::pair<unsigned int, std::shared_ptr<IObject>> VictoriaStar::_spawnSubObject(BattleManager &manager, unsigned int id, bool needRegister)
	{
		// Butterflies
		if (BUTTERFLIES_START_ID <= id && id < BUTTERFLIES_END_ID)
			return manager.registerObject<Butterfly>(
				needRegister,
				this,
				this->_opponent,
				this->_team,
				id < WEIRD_BUTTERFLIES_START_ID ? nullptr : this->_happyBufferFlies[id - WEIRD_BUTTERFLIES_START_ID].second,
				this->_subObjectsData.at(id < WEIRD_BUTTERFLIES_START_ID ? ACTION_HAPPY_BUTTERFLY : ACTION_WEIRD_BUTTERFLY),
				id
			);
		my_assert2(this->_projectileData.find(id) != this->_projectileData.end(), "Cannot find subobject " + std::to_string(id));

		auto &pdat = this->_projectileData[id];
		bool dir = this->_getProjectileDirection(pdat);

		if (!pdat.json.contains("shadow"))
			return Character::_spawnSubObject(manager, id, needRegister);
		try {
			std::string shadow = pdat.json["shadow"];

			if (shadow == "matter")
				return manager.registerObject<Shadow>(
					needRegister,
					this->_subObjectsData.at(pdat.action),
					pdat.json["hp"],
					dir,
					this->_calcProjectilePosition(pdat, dir ? 1 : -1),
					this->_team,
					id
				);
			if (shadow == "void")
				return manager.registerObject<Shadow>(
					needRegister,
					this->_subObjectsData.at(pdat.action),
					pdat.json["hp"],
					dir,
					this->_calcProjectilePosition(pdat, dir ? 1 : -1),
					this->_team,
					id
				);
			throw std::invalid_argument("Invalid shadow type " + shadow);
		} catch (std::out_of_range &e) {
			throw std::invalid_argument("Cannot find subobject id " + std::to_string(id));
		}
	}

	void VictoriaStar::getHit(IObject &other, const FrameData *data)
	{
		auto old = this->_hasHit;

		if (reinterpret_cast<VictoriaStar *>(&other)->_team == 4)
			this->_hasHit = false;
		Character::getHit(other, data);
		this->_hasHit = old;
	}

	bool VictoriaStar::hits(const IObject &other) const
	{
		auto old = this->_hasHit;
		auto t = const_cast<VictoriaStar *>(this);
		auto shadow = dynamic_cast<const Shadow *>(&other);

		if (shadow && shadow->getOwner() == this->_team)
			t->_hasHit = false;

		auto result = Character::hits(other);

		t->_hasHit = old;
		return result;
	}

	void VictoriaStar::hit(IObject &other, const FrameData *data)
	{
		auto shadow = dynamic_cast<const Shadow *>(&other);

		if (!shadow || shadow->getOwner() != this->_team)
			return Character::hit(other, data);
		this->_speed.x += data->pushBack * -this->_dir;
		if (!this->_hasHit && this->getCurrentFrameData()->oFlag.nextBlockOnHit) {
			this->_actionBlock++;
			my_assert2(this->_actionBlock != this->_moves.at(this->_action).size(), "Action " + actionToString(this->_action) + " is missing block " + std::to_string(this->_actionBlock));
			this->_animationCtr = 0;
			Object::_onMoveEnd(*data);
		}
		this->_hitShadow = true;
	}

	void VictoriaStar::_forceStartMove(unsigned int action)
	{
		this->_hitShadow = false;
		Character::_forceStartMove(action);
	}

	bool VictoriaStar::_canCancel(unsigned int action)
	{
		auto old = this->_hasHit;

		this->_hasHit |= this->_hitShadow;

		auto result = Character::_canCancel(action);

		this->_hasHit = old;
		return result;
	}

	void VictoriaStar::resolveSubObjects(const BattleManager &manager)
	{
		for (auto &butterfly : this->_happyBufferFlies)
			butterfly.second = reinterpret_cast<Butterfly *>(&*manager.getObjectFromId(butterfly.first));
		for (auto &butterfly : this->_weirdBufferFlies)
			butterfly.second = reinterpret_cast<Butterfly *>(&*manager.getObjectFromId(butterfly.first));
		Character::resolveSubObjects(manager);
	}

	void VictoriaStar::init(BattleManager &manager, const Character::InitData &data)
	{
		Character::init(manager, data);
		for (int i = 0; i < this->_happyBufferFlies.size(); i++) {
			auto result = this->_spawnSubObject(manager, 1000 + i, true);

			this->_happyBufferFlies[i] = {
				result.first,
				reinterpret_cast<Butterfly *>(&*result.second)
			};

			result = this->_spawnSubObject(manager, 1000 + this->_happyBufferFlies.size() + i,true);
			this->_weirdBufferFlies[i] = {
				result.first,
				reinterpret_cast<Butterfly *>(&*result.second)
			};
		}
	}
}