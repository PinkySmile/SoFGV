//
// Created by PinkySmile on 26/02/23.
//

#include "VictoriaStar.hpp"
#include "Resources/Game.hpp"
#include "Objects/Characters/VictoriaStar/Shadow.hpp"
#include "Objects/Characters/VictoriaStar/Shadows/NeutralShadow.hpp"
#include "Objects/Characters/VictoriaStar/Shadows/MatterShadow.hpp"
#include "Objects/Characters/VictoriaStar/Shadows/SpiritShadow.hpp"
#include "Objects/Characters/VictoriaStar/Shadows/VoidShadow.hpp"

#define ACTION_SCREEN_TELEPORT 368
#define ACTION_SHADOW 0
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
		std::shared_ptr<IInput> input,
		const std::string &opName
	) :
		Character(index, folder, palette, std::move(input))
	{
		auto spriteName = "shadow_" + opName + ".png";
		auto opHandle = game->textureMgr.load(folder + "/" + spriteName);

		game->logger.debug("VictoriaStar class created");
		// Butterflies actions
		my_assert(this->_subObjectsData.find(ACTION_HAPPY_BUTTERFLY) != this->_subObjectsData.end());
		my_assert(this->_subObjectsData.find(ACTION_WEIRD_BUTTERFLY) != this->_subObjectsData.end());
		// Shadow action
		my_assert(this->_subObjectsData.find(ACTION_SHADOW) != this->_subObjectsData.end());
		this->_shadowActions = this->_subObjectsData.at(ACTION_SHADOW);
		for (auto &block : this->_shadowActions)
			for (auto &frame : block) {
				frame.setSlave();
				frame.spritePath = spriteName;
				frame.textureHandle = opHandle;
				frame.setSlave(false);
			}
		game->textureMgr.remove(opHandle);
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

	size_t VictoriaStar::printDifference(const char *msgStart, void *data1, void *data2) const
	{
		auto length = Character::printDifference(msgStart, data1, data2);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		if (dat1->_hitShadow != dat2->_hitShadow)
			game->logger.fatal(std::string(msgStart) + "VictoriaStar::_hitShadow: " + std::to_string(dat1->_hitShadow) + " vs " + std::to_string(dat2->_hitShadow));
		return length + sizeof(Data);
	}

	void VictoriaStar::postUpdate()
	{
		Character::postUpdate();
		// Not using std::remove_if because it doesn't work with MSVC for some reasons
		for (unsigned i = 0; i < this->_shadows.size(); i++)
			if (this->_shadows[i].second->isDead())
				this->_shadows.erase(this->_shadows.begin() + i--);
	}

	static std::map<std::string, Shadow *(*)(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		bool tint
	)> shadowConstructors{
		{"neutral", NeutralShadow::create },
		{"matter", MatterShadow::create },
		{"spirit", SpiritShadow::create },
		{"void", VoidShadow::create },
	};

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
			unsigned tint = pdat.json["tint"];
			auto obj = std::shared_ptr<Shadow>(shadowConstructors.at(pdat.json["shadow"])(
				tint ? this->_shadowActions : this->_subObjectsData.at(ACTION_SHADOW),
				pdat.json["hp"],
				dir,
				this->_calcProjectilePosition(pdat, dir ? 1 : -1),
				this->_team,
				id,
				tint
			));

			if (!needRegister)
				return {0, obj};

			auto objectId = manager.registerObject(obj);

			this->_shadows.emplace_back(objectId, obj);
			return {objectId, obj};
		} catch (std::out_of_range &e) {
			throw std::invalid_argument("Invalid shadow type '" + pdat.json["shadow"].get<std::string>() + "'");
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

	void VictoriaStar::_blockMove(Object *other, const FrameData &data)
	{
		this->_target = other;
		Character::_blockMove(other, data);
		this->_target = nullptr;
	}

	void VictoriaStar::_forceStartMove(unsigned int action)
	{
		if (isBlockingAction(action) && this->_target) {
			for (auto &butterfly: this->_happyBufferFlies)
				butterfly.second->defensiveFormation(*this->_target);
			for (auto &butterfly : this->_weirdBufferFlies)
				butterfly.second->defensiveFormation(*this->_target);
		}
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
		for (auto &shadow : this->_shadows) {
			auto obj = manager.getObjectFromId(shadow.first);

			shadow.second = std::shared_ptr<Shadow>(obj, reinterpret_cast<Shadow *>(&*obj));
		}
		Character::resolveSubObjects(manager);
	}

	void VictoriaStar::init(BattleManager &manager, const Character::InitData &data)
	{
		Character::init(manager, data);
		for (unsigned i = 0; i < this->_happyBufferFlies.size(); i++) {
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

	void VictoriaStar::_applyMoveAttributes()
	{
		Character::_applyMoveAttributes();
		if (this->_action == ACTION_5A && this->getCurrentFrameData()->specialMarker)
			for (auto &shadow : this->_shadows)
				shadow.second->activate();
	}
}