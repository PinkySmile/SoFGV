//
// Created by PinkySmile on 02/02/2022.
//

#include "Resources/Game.hpp"
#include "Projectile.hpp"

namespace SpiralOfFate
{
	Projectile::Projectile(
		bool owner,
		unsigned id,
		const nlohmann::json &json
	) :
		SubObject(id, owner),
		_maxHit(json["hits"]),
		_endBlock(json["end_block"]),
		_loop(json["loop"]),
		_disableOnHit(json["disable_on_hit"]),
		_animationData(json.contains("animation_data") ? json["animation_data"].get<int>() : 0),
		_anim(json.contains("disable_animation") ? animationFromString(json["disable_animation"].get<std::string>()) : ANIMATION_DISAPPEAR)
	{
	}

	Projectile::Projectile(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned team,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned id,
		const nlohmann::json &json
	) :
		Projectile(owner, id, json)
	{
		this->_position = pos;
		this->_dir = direction ? 1 : -1;
		this->_direction = direction;
		this->_team = team;
		this->_moves[0] = frameData;
	}

	bool Projectile::hits(const IObject &other) const
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

	void Projectile::hit(IObject &other, const FrameData *data)
	{
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

	void Projectile::getHit(IObject &other, const FrameData *odata)
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
		if (this->_disableOnHit && (this->getOwner() ? game->battleMgr->getRightCharacter() : game->battleMgr->getLeftCharacter())->isHit())
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
		dat->disabled = this->_disabled;
		dat->nbHit = this->_nbHit;
	}

	void Projectile::restoreFromBuffer(void *data)
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		Object::restoreFromBuffer(data);
		this->_animationCtr = dat->animationCtr;
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
		if (this->_loop)
			return Object::_onMoveEnd(lastData);
		this->_dead = true;
	}

	size_t Projectile::printDifference(const char *msgStart, void *data1, void *data2) const
	{
		auto length = Object::printDifference(msgStart, data1, data2);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		if (dat1->animationCtr != dat2->animationCtr)
			game->logger.fatal(std::string(msgStart) + "Projectile::animationCtr: " + std::to_string(dat1->animationCtr) + " vs " + std::to_string(dat2->animationCtr));
		if (dat1->disabled != dat2->disabled)
			game->logger.fatal(std::string(msgStart) + "Projectile::disabled: " + std::to_string(dat1->disabled) + " vs " + std::to_string(dat2->disabled));
		if (dat1->nbHit != dat2->nbHit)
			game->logger.fatal(std::string(msgStart) + "Projectile::nbHit: " + std::to_string(dat1->nbHit) + " vs " + std::to_string(dat2->nbHit));
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
		for (auto &move : this->_moves)
			for (auto &block : move.second)
				for (auto &frame : block) {
					frame.hitBoxes.clear();
					frame.hurtBoxes.clear();
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
}
