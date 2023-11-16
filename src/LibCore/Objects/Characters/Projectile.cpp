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
		const nlohmann::json &json
	) :
		SubObject(id, owner, ownerObj),
		_maxHit(json["hits"]),
		_endBlock(json["end_block"]),
		_animationData(json.contains("animation_data") ? json["animation_data"].get<int>() : 0),
		_anim(json.contains("disable_animation") ? animationFromString(json["disable_animation"].get<std::string>()) : ANIMATION_DISAPPEAR),
		_loop(json["loop"]),
		_disableOnHit(json["disable_on_hit"])
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
		const nlohmann::json &json
	) :
		Projectile(owner, ownerObj, id, json)
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

	void Projectile::hit(Object &other, const FrameData *data)
	{
		// We can't hit someone from our team normally.
		// If it happens, that means that we got reflected.
		// We reflected, we stay active as if we didn't actually hit the target.
		if (other.getTeam() == this->getTeam())
			return;
		if (data->dFlag.canBlock) {
			auto owner = this->getOwnerObj();

			owner->hit(other, data);
			reinterpret_cast<Projectile *>(owner)->_hitStop = this->_hitStop;
		}
		Object::hit(other, data);

		auto proj = dynamic_cast<Projectile *>(&other);

		if (!proj) {
			this->_nbHit++;
			return;
		}

		auto odata = other.getCurrentFrameData();

		if (data->priority) {
			if (!odata->priority || *odata->priority < *data->priority)
				proj->_dead = true;
			else if (odata->priority && *odata->priority > *data->priority)
				this->_dead = true;
			else
				this->_nbHit++;
		} else if (odata->priority)
			this->_dead = true;
		else
			this->_nbHit++;
	}

	void Projectile::getHit(Object &other, const FrameData *odata)
	{
		Object::getHit(other, odata);

		auto proj = dynamic_cast<Projectile *>(&other);

		if (!proj) {
			this->_nbHit++;
			return;
		}

		auto data = other.getCurrentFrameData();

		if (data->priority) {
			if (!odata->priority || *odata->priority < *data->priority)
				proj->_dead = true;
			else if (odata->priority && *odata->priority > *data->priority)
				this->_dead = true;
			else
				this->_nbHit++;
		} else if (odata->priority)
			this->_dead = true;
		else
			this->_nbHit++;
	}

	bool Projectile::isDead() const
	{
		return Object::isDead() || this->_nbHit >= this->_maxHit;
	}

	void Projectile::update()
	{
		if (this->_disableOnHit && this->getOwnerObj()->isHit())
			this->_disableObject();
		if (this->_disabled && this->_anim == ANIMATION_FADE) {
			this->_sprite.setColor({255, 255, 255, static_cast<unsigned char>(255 - 255 * this->_animationCtr / this->_animationData)});
			this->_animationCtr++;
			this->_dead |= this->_animationCtr > this->_animationData;
		}
		Object::update();
		this->_dead |=
			this->_position.x < -300 ||
			this->_position.x > 1300 ||
			this->_position.y < -300 ||
			this->_position.y > 1300;
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
		dat->fadingOut = this->_fadingOut;
		dat->disabled = this->_disabled;
		dat->nbHit = this->_nbHit;
	}

	void Projectile::restoreFromBuffer(void *data)
	{
		Object::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		this->_animationCtr = dat->animationCtr;
		this->_fadingOut = dat->fadingOut;
		this->_disabled = dat->disabled;
		this->_nbHit = dat->nbHit;
	}

	void Projectile::_onMoveEnd(const FrameData &lastData)
	{
		if (this->_actionBlock > this->_endBlock) {
			this->_dead = true;
			return;
		}
		if (this->_actionBlock != this->_endBlock) {
			this->_actionBlock++;
			my_assert2(this->_moves.at(this->_action).size() != this->_actionBlock, "Subobject " + std::to_string(this->_action) + " is missing block " + std::to_string(this->_actionBlock));
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
		if (dat1->fadingOut != dat2->fadingOut)
			game->logger.fatal(std::string(msgStart) + "Projectile::fadingOut: " + std::to_string(dat1->fadingOut) + " vs " + std::to_string(dat2->fadingOut));
		return length + sizeof(Data);
	}

	void Projectile::_disableObject()
	{
		if (this->_disabled)
			return;
		if (this->_anim == ANIMATION_DISAPPEAR) {
			this->_dead = true;
			return;
		}
		this->_disabled = true;
		if (this->_anim == ANIMATION_BLOCK) {
			this->_actionBlock = this->_animationData;
			this->_animation = 0;
			this->_newAnim = true;
			return;
		}
		this->_fadingOut = true;
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
		if (this->_fadingOut) {
			this->_fdCache.hitBoxes.clear();
			this->_fdCache.hurtBoxes.clear();
			this->_fdCache.collisionBox = nullptr;
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
		game->logger.info(std::string(msgStart) + "Projectile::fadingOut: " + std::to_string(dat1->fadingOut));
		if (startOffset + length + sizeof(Data) >= dataSize) {
			game->logger.fatal("Invalid input frame");
			return 0;
		}
		return length + sizeof(Data);
	}
}
