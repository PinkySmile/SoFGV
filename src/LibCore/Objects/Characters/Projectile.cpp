//
// Created by PinkySmile on 02/02/2022.
//

#include "Resources/Game.hpp"
#include "Projectile.hpp"
#include "Character.hpp"
#include "Objects/CheckUtils.hpp"

namespace SpiralOfFate
{
	Projectile::Projectile(
		bool owner,
		Character *ownerObj,
		unsigned id,
		const nlohmann::json &json,
		TypeSwitch typeSwitchFlags,
		unsigned debuffDuration
	) :
		SubObject(id, owner, ownerObj),
		_debuffDuration(debuffDuration),
		_typeSwitch(typeSwitchFlags),
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
		Character *ownerObj,
		unsigned id,
		const nlohmann::json &json,
		TypeSwitch typeSwitchFlags,
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

	void Projectile::kill()
	{
		this->_disableObject(this->_onGetHitDieAnim);
	}

	unsigned int Projectile::getBufferSize() const
	{
		return Object::getBufferSize() + sizeof(Data);
	}

	void Projectile::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data) + Object::getBufferSize());

		Object::copyToBuffer(data);
		dat->_animationCtr = this->_animationCtr;
		dat->_disabled = this->_disabled;
		dat->_nbHit = this->_nbHit;
		dat->_animData = this->_animData;
		dat->_animType = this->_animType;
		dat->_typeSwitchFlags = this->_typeSwitch;
		dat->_debuffDuration = this->_debuffDuration;
	}

	void Projectile::restoreFromBuffer(void *data)
	{
		Object::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data) + Object::getBufferSize());

		this->_animationCtr = dat->_animationCtr;
		this->_disabled = dat->_disabled;
		this->_nbHit = dat->_nbHit;
		this->_animData = dat->_animData;
		this->_animType = dat->_animType;
		this->_typeSwitch = dat->_typeSwitchFlags;
		this->_debuffDuration = dat->_debuffDuration;
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

		auto dat1 = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data1) + length);
		auto dat2 = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data2) + length);

		game->logger.info("Projectile @" + std::to_string(startOffset + length));
		OBJECT_CHECK_FIELD("Projectile", "", dat1, dat2, _animationCtr, std::to_string);
		OBJECT_CHECK_FIELD("Projectile", "", dat1, dat2, _disabled, DISP_BOOL);
		OBJECT_CHECK_FIELD("Projectile", "", dat1, dat2, _nbHit, std::to_string);
		OBJECT_CHECK_FIELD("Projectile", "", dat1, dat2, _animData, std::to_string);
		OBJECT_CHECK_FIELD("Projectile", "", dat1, dat2, _animType, std::to_string);
		OBJECT_CHECK_FIELD("Projectile", "", dat1, dat2, _typeSwitchFlags, std::to_string);
		OBJECT_CHECK_FIELD("Projectile", "", dat1, dat2, _debuffDuration, std::to_string);
		return length + sizeof(Data);
	}

	void Projectile::_disableObject(const ProjectileAnimationData &data)
	{
		if (this->_disabled || !data.hasValue)
			return;
		this->_animType = data.type;
		if (this->_animType == ANIMATION_DISAPPEAR) {
			this->_dead = true;
			return;
		}
		this->_animData = data.data;
		this->_disabled = true;
		if (this->_animType == ANIMATION_BLOCK) {
			this->_actionBlock = this->_animData;
			this->_animation = 0;
			this->_newAnim = true;
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
		if (this->_typeSwitch != TYPESWITCH_NONE) {
			int index = LIMIT_NEUTRAL;

			static_assert(TYPESWITCH_NEUTRAL - 1 == LIMIT_NEUTRAL);
			static_assert(TYPESWITCH_VOID - 1    == LIMIT_VOID);
			static_assert(TYPESWITCH_MATTER - 1  == LIMIT_MATTER);
			static_assert(TYPESWITCH_SPIRIT - 1  == LIMIT_SPIRIT);

			if (this->_typeSwitch != TYPESWITCH_NON_TYPED)
				index = this->_typeSwitch - 1;
			this->_fdCache.oFlag.voidElement   = this->_typeSwitch == TYPESWITCH_NEUTRAL || this->_typeSwitch == TYPESWITCH_VOID;
			this->_fdCache.oFlag.matterElement = this->_typeSwitch == TYPESWITCH_NEUTRAL || this->_typeSwitch == TYPESWITCH_MATTER;
			this->_fdCache.oFlag.spiritElement = this->_typeSwitch == TYPESWITCH_NEUTRAL || this->_typeSwitch == TYPESWITCH_SPIRIT;
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

		auto dat = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data) + length);

		game->logger.info("Projectile @" + std::to_string(startOffset + length));
		if (startOffset + length + sizeof(Data) >= dataSize)
			game->logger.warn("Object is " + std::to_string(startOffset + length + sizeof(Data) - dataSize) + " bytes bigger than input");
		DISPLAY_FIELD("Projectile", "", dat, _nbHit, std::to_string);
		DISPLAY_FIELD("Projectile", "", dat, _animationCtr, std::to_string);
		DISPLAY_FIELD("Projectile", "", dat, _debuffDuration, std::to_string);
		DISPLAY_FIELD("Projectile", "", dat, _disabled, DISP_BOOL);
		DISPLAY_FIELD("Projectile", "", dat, _animData, std::to_string);
		DISPLAY_FIELD("Projectile", "", dat, _typeSwitchFlags, std::to_string);
		DISPLAY_FIELD("Projectile", "", dat, _animType, std::to_string);
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
