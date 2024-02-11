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
#include "VictoriaProjectile.hpp"

#define ACTION_SCREEN_TELEPORT 368
#define ACTION_SHADOW 0
#define ACTION_FLOWER 1
#define ACTION_HAPPY_BUTTERFLY 4
#define ACTION_WEIRD_BUTTERFLY 5
#define WEIRD_BUTTERFLIES_START_ID (1000 + NB_BUTTERFLIES)
#define BUTTERFLIES_END_ID (1000 + NB_BUTTERFLIES * 2)
#define FLOWER_ID BUTTERFLIES_END_ID

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
		this->_hudBack.textureHandle = game->textureMgr.load(folder + "/hud.png");
		this->_hudFull.textureHandle = this->_hudBack.textureHandle;
		this->_hudPart.textureHandle = this->_hudBack.textureHandle;
		game->textureMgr.setTexture(this->_hudBack);
		game->textureMgr.setTexture(this->_hudFull);
		game->textureMgr.setTexture(this->_hudPart);

		auto size = game->textureMgr.getTextureSize(this->_hudBack.textureHandle);

		this->_hudBack.setScale(0.25, 0.25);
		this->_hudFull.setScale(0.25, 0.25);
		this->_hudPart.setScale(0.25, 0.25);
		this->_hudBack.setTextureRect({
			0, 0,
			static_cast<int>(size.x) / 2,
			static_cast<int>(size.y) / 2
		});
		this->_hudFull.setTextureRect({
			static_cast<int>(size.x) / 2,
			0,
			static_cast<int>(size.x) / 2,
			static_cast<int>(size.y) / 2
		});
		this->_hudPart.setTextureRect({
			0,
			static_cast<int>(size.y) / 2,
			static_cast<int>(size.x) / 2,
			static_cast<int>(size.y) / 2
		});
	}

	VictoriaStar::~VictoriaStar()
	{
		game->textureMgr.addRef(this->_hudBack.textureHandle);
	}

	bool VictoriaStar::_startMove(unsigned int action)
	{
		if (action == ACTION_BACKWARD_DASH && (this->_position.x == 0 || this->_position.x == 1000))
			return Object::_startMove(ACTION_SCREEN_TELEPORT);
		return Object::_startMove(action);
	}

	void VictoriaStar::_tickMove()
	{
		Character::_tickMove();
		if ((this->_action == ACTION_CROUCHING || this->_action == ACTION_CROUCH) && !this->_flower) {
			auto result = this->_spawnSubObject(*game->battleMgr, FLOWER_ID, true);

			this->_flower = {
				result.first,
				std::shared_ptr<Flower>(result.second, reinterpret_cast<Flower *>(&*result.second))
			};
		}
		if (this->_flower && this->_flower->second->isDead())
			this->_flower.reset();
	}

	unsigned int VictoriaStar::getClassId() const
	{
		return CLASS_ID;
	}

	unsigned int VictoriaStar::getBufferSize() const
	{
		return Character::getBufferSize() + sizeof(Data) + sizeof(unsigned) * this->_shadows.size();
	}

	void VictoriaStar::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());
		unsigned i = 0;

		Character::copyToBuffer(data);
		game->logger.verbose("Saving VictoriaStar (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
		dat->_hitShadow = this->_hitShadow;
		dat->_stacks = this->_stacks;
		dat->_flower = this->_flower && !this->_flower->second->isDead() ? this->_flower->first : 0;
		dat->_nbShadows = this->_shadows.size();
		for (auto &shadow : this->_shadows)
			dat->_objects[i++] = shadow.first;
	}

	void VictoriaStar::restoreFromBuffer(void *data)
	{
		Character::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());
		unsigned i = 0;

		this->_hitShadow = dat->_hitShadow;
		this->_stacks = dat->_stacks;
		this->_shadows.clear();
		if (dat->_flower == 0)
			this->_flower.reset();
		else
			this->_flower = {dat->_flower, nullptr};
		this->_shadows.reserve(dat->_nbShadows);
		for (; i < dat->_nbShadows; i++)
			this->_shadows.emplace_back(dat->_objects[i], nullptr);
		game->logger.verbose("Restored VictoriaStar @" + std::to_string((uintptr_t)dat));
	}

	size_t VictoriaStar::printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const
	{
		auto length = Character::printDifference(msgStart, data1, data2, startOffset);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		game->logger.info("VictoriaStar @" + std::to_string(startOffset + length));
		if (dat1->_hitShadow != dat2->_hitShadow)
			game->logger.fatal(std::string(msgStart) + "VictoriaStar::_hitShadow: " + std::to_string(dat1->_hitShadow) + " vs " + std::to_string(dat2->_hitShadow));
		if (dat1->_stacks != dat2->_stacks)
			game->logger.fatal(std::string(msgStart) + "VictoriaStar::_stacks: " + std::to_string(dat1->_stacks) + " vs " + std::to_string(dat2->_stacks));
		if (dat1->_nbShadows != dat2->_nbShadows)
			game->logger.fatal(std::string(msgStart) + "VictoriaStar::_nbShadows: " + std::to_string(dat1->_nbShadows) + " vs " + std::to_string(dat2->_nbShadows));
		if (dat1->_flower != dat2->_flower)
			game->logger.fatal(std::string(msgStart) + "VictoriaStar::_flower: " + std::to_string(dat1->_flower) + " vs " + std::to_string(dat2->_flower));
		if (dat1->_nbShadows != dat2->_nbShadows)
			return 0;
		for (unsigned i = 0; i < dat1->_nbShadows; i++)
			if (dat1->_objects[i] != dat2->_objects[i])
				game->logger.fatal(std::string(msgStart) + "VictoriaStar::shadows[" + std::to_string(i) + "]: " + std::to_string(dat1->_objects[i]) + " vs " + std::to_string(dat2->_objects[i]));
		return length + sizeof(Data) + sizeof(unsigned) * dat1->_nbShadows;
	}

	void VictoriaStar::postUpdate()
	{
		Character::postUpdate();
		// Not using std::remove_if because it doesn't work with MSVC for some reasons
		for (unsigned i = 0; i < this->_shadows.size(); i++)
			if (this->_shadows[i].second->isDead()) {
				if (this->_stacks < MAX_STACKS * SHADOW_PER_STACK)
					this->_stacks += this->_shadows[i].second->wasOwnerKilled();
				this->_shadows.erase(this->_shadows.begin() + i--);
			}
	}

	static std::map<std::string, Shadow *(*)(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		Character *ownerObj,
		unsigned int id,
		bool tint
	)> shadowConstructors{
		{"neutral", NeutralShadow::create },
		{"matter", MatterShadow::create },
		{"spirit", SpiritShadow::create },
		{"void", VoidShadow::create },
	};

	std::pair<unsigned int, std::shared_ptr<Object>> VictoriaStar::_spawnSubObject(BattleManager &manager, unsigned int id, bool needRegister)
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
		if (id == FLOWER_ID)
			return manager.registerObject<Flower>(
				needRegister,
				this,
				this->_subObjectsData.at(ACTION_FLOWER),
				this->_direction,
				this->_position + Vector2f{30 * this->_dir, 0},
				this->_team,
				id
			);
		my_assert2(this->_projectileData.find(id) != this->_projectileData.end(), "Cannot find subobject " + std::to_string(id));

		auto &pdat = this->_projectileData[id];
		bool dir = this->_getProjectileDirection(pdat);

		if (!pdat.json.contains("shadow")) {
			unsigned char flags = 0;

			if (this->_installMoveStarted) {
				flags |= this->_hasVoidInstall   * Projectile::TYPESWITCH_VOID;
				flags |= this->_hasMatterInstall * Projectile::TYPESWITCH_MATTER;
				flags |= this->_hasSpiritInstall * Projectile::TYPESWITCH_SPIRIT;
			}
			try {
				return manager.registerObject<VictoriaProjectile>(
					needRegister,
					this->_subObjectsData.at(pdat.action),
					this->_team,
					dir,
					this->_calcProjectilePosition(pdat, dir ? 1 : -1),
					this->_team,
					this,
					id,
					pdat.json,
					flags
				);
			} catch (std::out_of_range &e) {
				throw std::invalid_argument("Cannot find subobject action id " + std::to_string(id));
			}
		}

		try {
			unsigned tint = pdat.json["tint"];
			auto obj = std::shared_ptr<Shadow>(shadowConstructors.at(pdat.json["shadow"])(
				tint ? this->_shadowActions : this->_subObjectsData.at(ACTION_SHADOW),
				pdat.json["hp"],
				dir,
				this->_calcProjectilePosition(pdat, dir ? 1 : -1),
				this->_team,
				this,
				id,
				tint
			));

			if (!needRegister)
				return {0, obj};

			auto objectId = manager.registerObject(static_cast<std::shared_ptr<Object>>(obj));

			this->_shadows.emplace_back(objectId, obj);
			return {objectId, obj};
		} catch (std::out_of_range &e) {
			throw std::invalid_argument("Invalid shadow type '" + pdat.json["shadow"].get<std::string>() + "'");
		}
	}

	void VictoriaStar::getHit(Object &other, const FrameData *data)
	{
		auto old = this->_hasHit;

		if (reinterpret_cast<VictoriaStar *>(&other)->_team == 4)
			this->_hasHit = false;
		Character::getHit(other, data);
		this->_hasHit = old;
	}

	bool VictoriaStar::_canStartMove(unsigned int action, const FrameData &data)
	{
		if ((action == ACTION_5A || action == ACTION_j5A) && this->_stacks < SHADOW_PER_STACK)
			return false;
		return Character::_canStartMove(action, data);
	}

	bool VictoriaStar::hits(const Object &other) const
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

	void VictoriaStar::hit(Object &other, const FrameData *data)
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
		if (action == ACTION_FORWARD_DASH)
			for (auto &shadow : this->_shadows)
				shadow.second->setInvincible(40);
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
		for (unsigned i = 0; i < this->_weirdBufferFlies.size(); i++) {
			auto &butterfly = this->_weirdBufferFlies[i];

			butterfly.second = reinterpret_cast<Butterfly *>(&*manager.getObjectFromId(butterfly.first));
			butterfly.second->_copy = &*this->_happyBufferFlies[i].second;
		}
		for (auto &shadow : this->_shadows) {
			auto obj = manager.getObjectFromId(shadow.first);

			shadow.second = std::shared_ptr<Shadow>(obj, reinterpret_cast<Shadow *>(&*obj));
		}
		if (this->_flower) {
			auto obj = manager.getObjectFromId(this->_flower->first);

			this->_flower->second = std::shared_ptr<Flower>(obj, reinterpret_cast<Flower *>(&*obj));
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

	void VictoriaStar::_applyNewAnimFlags()
	{
		if (!this->_newAnim)
			return;

		auto data = this->getCurrentFrameData();

		Character::_applyNewAnimFlags();
		if (data->specialMarker == 15) {
			for (size_t i = 0; i < this->_happyBufferFlies.size(); i++)
				this->_happyBufferFlies[i].second->startAttack(
					this->_getButterflyAttackPos(i),
					data->blockPlayerHitStop,
					data->blockOpponentHitStop,
					data->hitOpponentHitStop,
					data->hitPlayerHitStop
				);
			return;
		}
		if ((this->_action == ACTION_5A || this->_action == ACTION_j5A) && data->specialMarker) {
			for (auto &shadow: this->_shadows)
				shadow.second->activate();
			this->_stacks -= SHADOW_PER_STACK;
		}
	}

	void VictoriaStar::drawSpecialHUD(sf::RenderTarget &texture)
	{
		for (unsigned i = 0; i < MAX_STACKS; i++) {
			if (this->_stacks >= (i + 1) * SHADOW_PER_STACK) {
				this->_hudFull.setPosition(20 + 10 * i, 600);
				texture.draw(this->_hudFull);
				continue;
			}
			this->_hudBack.setPosition(20 + 10 * i, 600);
			texture.draw(this->_hudBack);
			if (this->_stacks <= i * SHADOW_PER_STACK)
				continue;

			auto size = game->textureMgr.getTextureSize(this->_hudPart.textureHandle);

			this->_hudPart.setTextureRect({
				0,
				static_cast<int>(size.y) / 2,
				static_cast<int>((size.x / 2) * (this->_stacks - i * SHADOW_PER_STACK)) / SHADOW_PER_STACK,
				static_cast<int>(size.y)
			});
			this->_hudPart.setPosition(20 + 10 * i, 600);
			texture.draw(this->_hudPart);
		}
		Character::drawSpecialHUD(texture);
	}

	Vector2f VictoriaStar::_getButterflyAttackPos(unsigned int id)
	{
		if (this->_action == ACTION_6S)
			return {
				840.f * this->_direction + id * (160 / NB_BUTTERFLIES),
				-20
			};
		return {0, 0};
	}

	size_t VictoriaStar::printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const
	{
		auto length = Character::printContent(msgStart, data, startOffset, dataSize);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data + length);

		game->logger.info("VictoriaStar @" + std::to_string(startOffset + length));
		if (startOffset + length + sizeof(Data) + sizeof(unsigned) * dat1->_nbShadows >= dataSize)
			game->logger.warn("Object is " + std::to_string(startOffset + length + sizeof(Data) + sizeof(unsigned) * dat1->_nbShadows - dataSize) + " bytes bigger than input");
		game->logger.info(std::string(msgStart) + "VictoriaStar::_hitShadow: " + std::to_string(dat1->_hitShadow));
		game->logger.info(std::string(msgStart) + "VictoriaStar::_stacks: " + std::to_string(dat1->_stacks));
		game->logger.info(std::string(msgStart) + "VictoriaStar::_nbShadows: " + std::to_string(dat1->_nbShadows));
		game->logger.info(std::string(msgStart) + "VictoriaStar::_flower: " + std::to_string(dat1->_flower));
		for (unsigned i = 0; i < dat1->_nbShadows; i++)
			game->logger.info(std::string(msgStart) + "VictoriaStar::shadows[" + std::to_string(i) + "]: " + std::to_string(dat1->_objects[i]));
		if (startOffset + length + sizeof(Data) + sizeof(unsigned) * dat1->_nbShadows >= dataSize) {
			game->logger.fatal("Invalid input frame");
			return 0;
		}
		return length + sizeof(Data) + sizeof(unsigned) * dat1->_nbShadows;
	}
}