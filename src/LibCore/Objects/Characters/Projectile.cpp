//
// Created by PinkySmile on 02/02/2022.
//

#include "Resources/Game.hpp"
#include "Projectile.hpp"

namespace SpiralOfFate
{
	Projectile::Projectile(
		bool owner,
		class Character *ownerObj,
		unsigned id,
		const nlohmann::json &json,
		unsigned char typeSwitchFlags,
		unsigned debuffDuration
	) :
		SubObject(id, owner, ownerObj),
		_debuffDuration(debuffDuration),
		_typeSwitchFlags(typeSwitchFlags),
		_maxHit(json["hits"]),
		_endBlock(json["end_block"]),
		_onHitDieAnim(json["on_hit_die_animation"]),
		_onBlockDieAnim(json["on_block_die_animation"]),
		_onGetHitDieAnim(json["on_get_hit_die_animation"]),
		_onOwnerHitDieAnim(json["on_owner_hit_die_animation"]),
		_loop(json["loop"])
	{
	}

	Projectile::Projectile(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned team,
		bool direction,
		Vector2f pos,
		bool owner,
		class Character *ownerObj,
		unsigned id,
		const nlohmann::json &json,
		unsigned char typeSwitchFlags,
		unsigned debuffDuration
	) :
		Projectile(owner, ownerObj, id, json, typeSwitchFlags, debuffDuration)
	{
		this->_position = pos;
		this->_dir = direction ? 1 : -1;
		this->_direction = direction;
		this->_team = team;
		this->_moves[0] = frameData;
	}

	bool Projectile::hits(const Object &other) const
	{
		if (this->_disabled)
			return false;

		auto otherChr = dynamic_cast<const Character *>(&other);

		if (otherChr)
			for (auto limit : otherChr->getLimit())
				if (limit >= 100)
					return false;
		return Object::hits(other);
	}

	unsigned int Projectile::getDebuffDuration() const
	{
		return this->_debuffDuration;
	}

	void Projectile::hit(Object &other, const FrameData *data)
	{
		// We can't hit someone from our team normally.
		// If it happens, that means that we got reflected.
		// When reflected, we stay active as if we didn't actually hit the target.
		if (other.getTeam() == this->getTeam())
			return;
		if (data->dFlag.canBlock) {
			auto owner = this->getOwnerObj();

			owner->hit(other, data);
			this->_hitStop = reinterpret_cast<Projectile *>(owner)->_hitStop;
		}
		Object::hit(other, data);

		auto chr = dynamic_cast<Character *>(&other);

		if (chr) {
			this->_nbHit++;
			if (this->_nbHit >= this->_maxHit) {
				auto d = chr->getCurrentFrameData();

				if (
					chr->isHit() ||
					d->dFlag.superarmor ||
					d->dFlag.invulnerableArmor ||
					d->dFlag.neutralArmor ||
					d->dFlag.matterArmor ||
					d->dFlag.spiritArmor ||
					d->dFlag.voidArmor
				)
					this->_disableObject(this->_onHitDieAnim);
				else
					this->_disableObject(this->_onBlockDieAnim);
			}
			return;
		}

		auto proj = dynamic_cast<Projectile *>(&other);

		if (!proj) {
			this->_nbHit++;
			if (this->_nbHit >= this->_maxHit)
				this->_disableObject(this->_onHitDieAnim);
			return;
		}

		auto odata = other.getCurrentFrameData();

		if (data->priority) {
			if (!odata->priority || *odata->priority < *data->priority)
				proj->_nbHit = proj->_maxHit;
			else if (odata->priority && *odata->priority > *data->priority)
				this->_nbHit = this->_maxHit;
			else
				this->_nbHit++;
		} else if (odata->priority)
			this->_nbHit = this->_maxHit;
		else
			this->_nbHit++;
		if (this->_nbHit >= this->_maxHit)
			this->_disableObject(this->_onHitDieAnim);
	}

	void Projectile::getHit(Object &other, const FrameData *odata)
	{
		Object::getHit(other, odata);

		auto proj = dynamic_cast<Projectile *>(&other);

		if (!proj) {
			this->_nbHit++;
			if (this->_nbHit >= this->_maxHit)
				this->_disableObject(this->_onGetHitDieAnim);
			return;
		}

		auto data = other.getCurrentFrameData();

		if (data->priority) {
			if (!odata->priority || *odata->priority < *data->priority)
				proj->_nbHit = proj->_maxHit;
			else if (odata->priority && *odata->priority > *data->priority)
				this->_nbHit = this->_maxHit;
			else
				this->_nbHit++;
		} else if (odata->priority)
			this->_nbHit = this->_maxHit;
		else
			this->_nbHit++;
		if (this->_nbHit >= this->_maxHit)
			this->_disableObject(this->_onGetHitDieAnim);
	}

	void Projectile::update()
	{
		if (this->getOwnerObj()->isHit())
			this->_disableObject(this->_onOwnerHitDieAnim);
		if (this->_disabled && this->_animType == ANIMATION_FADE) {
			this->_sprite.setColor({255, 255, 255, static_cast<unsigned char>(255 - 255 * this->_animationCtr / this->_animData)});
			this->_animationCtr++;
			this->_dead |= this->_animationCtr > this->_animData;
		}
		Object::update();
		this->_dead |=
			this->_position.x < STAGE_X_MIN - 300 ||
			this->_position.x > STAGE_X_MAX + 300 ||
			this->_position.y > STAGE_Y_MAX + 300 ||
			this->_position.y < STAGE_Y_MIN - 300;
	}

	unsigned int Projectile::getBufferSize() const
	{
		return Object::getBufferSize() + sizeof(Data);
	}

	void Projectile::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		Object::copyToBuffer(data);
		dat->animationCtr = this->_animationCtr;
		dat->disabled = this->_disabled;
		dat->nbHit = this->_nbHit;
		dat->typeSwitchFlags = this->_typeSwitchFlags;
		dat->debuffDuration = this->_debuffDuration;
	}

	void Projectile::restoreFromBuffer(void *data)
	{
		Object::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		this->_animationCtr = dat->animationCtr;
		this->_disabled = dat->disabled;
		this->_nbHit = dat->nbHit;
		this->_typeSwitchFlags = dat->typeSwitchFlags;
		this->_debuffDuration = dat->debuffDuration;
	}

	void Projectile::_onMoveEnd(const FrameData &lastData)
	{
		if (this->_actionBlock > this->_endBlock) {
			this->_dead = true;
			Object::_onMoveEnd(lastData);
			return;
		}
		if (this->_actionBlock != this->_endBlock) {
			this->_actionBlock++;
			assert_msg(this->_moves.at(this->_action).size() != this->_actionBlock, "Projectile " + std::to_string(this->_action) + " is missing block " + std::to_string(this->_actionBlock));
			return Object::_onMoveEnd(lastData);
		}
		if (!this->_loop)
			this->_dead = true;
		Object::_onMoveEnd(lastData);
	}

	size_t Projectile::printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const
	{
		auto length = Object::printDifference(msgStart, data1, data2, startOffset);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		game->logger.info("Projectile @" + std::to_string(startOffset + length));
		if (dat1->animationCtr != dat2->animationCtr)
			game->logger.fatal(std::string(msgStart) + "Projectile::animationCtr: " + std::to_string(dat1->animationCtr) + " vs " + std::to_string(dat2->animationCtr));
		if (dat1->disabled != dat2->disabled)
			game->logger.fatal(std::string(msgStart) + "Projectile::disabled: " + std::to_string(dat1->disabled) + " vs " + std::to_string(dat2->disabled));
		if (dat1->nbHit != dat2->nbHit)
			game->logger.fatal(std::string(msgStart) + "Projectile::nbHit: " + std::to_string(dat1->nbHit) + " vs " + std::to_string(dat2->nbHit));
		if (dat1->typeSwitchFlags != dat2->typeSwitchFlags)
			game->logger.fatal(std::string(msgStart) + "Projectile::typeSwitchFlags: " + std::to_string(dat1->typeSwitchFlags) + " vs " + std::to_string(dat2->typeSwitchFlags));
		if (dat1->debuffDuration != dat2->debuffDuration)
			game->logger.fatal(std::string(msgStart) + "Projectile::debuffDuration: " + std::to_string(dat1->debuffDuration) + " vs " + std::to_string(dat2->debuffDuration));
		return length + sizeof(Data);
	}

	void Projectile::_disableObject(const ProjectileAnimationData &data)
	{
		if (this->_disabled || !data.hasValue)
			return;
		if (this->_animType == ANIMATION_DISAPPEAR) {
			this->_dead = true;
			return;
		}
		this->_animType = data.type;
		this->_animData = data.data;
		this->_disabled = true;
		if (this->_animType == ANIMATION_BLOCK) {
			this->_actionBlock = this->_animData;
			this->_animation = 0;
			this->_newAnim = true;
			return;
		}
	}

	Projectile::ProjectileAnimation Projectile::animationFromString(const std::string &str)
	{
		if (str == "fade")
			return ANIMATION_FADE;
		if (str == "disappear")
			return ANIMATION_DISAPPEAR;
		if (str == "block")
			return ANIMATION_BLOCK;
		throw std::invalid_argument("Invalid animation '" + str + "'");
	}

	void Projectile::_computeFrameDataCache()
	{
		Object::_computeFrameDataCache();
		if (this->_disabled && this->_animType == ANIMATION_FADE) {
			this->_fdCache.hitBoxes.clear();
			this->_fdCache.hurtBoxes.clear();
			this->_fdCache.collisionBox = nullptr;
		}
		if (this->_typeSwitchFlags) {
			auto index = (this->_typeSwitchFlags & TYPESWITCH_VOID)   ? LIMIT_VOID :
			             (this->_typeSwitchFlags & TYPESWITCH_MATTER) ? LIMIT_MATTER :
			             (this->_typeSwitchFlags & TYPESWITCH_SPIRIT) ? LIMIT_SPIRIT : LIMIT_NEUTRAL;

			this->_fdCache.oFlag.voidElement   = (this->_typeSwitchFlags & (TYPESWITCH_NEUTRAL | TYPESWITCH_VOID))   != 0;
			this->_fdCache.oFlag.matterElement = (this->_typeSwitchFlags & (TYPESWITCH_NEUTRAL | TYPESWITCH_MATTER)) != 0;
			this->_fdCache.oFlag.spiritElement = (this->_typeSwitchFlags & (TYPESWITCH_NEUTRAL | TYPESWITCH_SPIRIT)) != 0;
			for (int i = 0; i < 4; i++) {
				if (i == index)
					continue;
				(&this->_fdCache.neutralLimit)[index] += (&this->_fdCache.neutralLimit)[i];
				(&this->_fdCache.neutralLimit)[i] = 0;
			}
		}
	}

	size_t Projectile::printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const
	{
		auto length = Object::printContent(msgStart, data, startOffset, dataSize);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data + length);

		game->logger.info("Projectile @" + std::to_string(startOffset + length));
		if (startOffset + length + sizeof(Data) >= dataSize)
			game->logger.warn("Object is " + std::to_string(startOffset + length + sizeof(Data) - dataSize) + " bytes bigger than input");
		game->logger.info(std::string(msgStart) + "Projectile::animationCtr: " + std::to_string(dat1->animationCtr));
		game->logger.info(std::string(msgStart) + "Projectile::disabled: " + std::to_string(dat1->disabled));
		game->logger.info(std::string(msgStart) + "Projectile::nbHit: " + std::to_string(dat1->nbHit));
		game->logger.info(std::string(msgStart) + "Projectile::typeSwitchFlags: " + std::to_string(dat1->typeSwitchFlags));
		game->logger.info(std::string(msgStart) + "Projectile::debuffDuration: " + std::to_string(dat1->debuffDuration));
		if (startOffset + length + sizeof(Data) >= dataSize) {
			game->logger.fatal("Invalid input frame");
			return 0;
		}
		return length + sizeof(Data);
	}

	Projectile::ProjectileAnimationData::ProjectileAnimationData(const nlohmann::json &json) :
		hasValue(!json.is_null())
	{
		if (!this->hasValue)
			return;
		this->type = animationFromString(json["type"]);
		this->data = json["data"];
	}
}
