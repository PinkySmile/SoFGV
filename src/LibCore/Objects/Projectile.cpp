//
// Created by PinkySmile on 02/02/2022.
//

#include "Resources/Game.hpp"
#include "Projectile.hpp"

namespace SpiralOfFate
{
	Projectile::Projectile(bool owner, unsigned int id, unsigned maxHit) :
		maxHit(maxHit),
		owner(owner),
		id(id)
	{
	}

	Projectile::Projectile(const std::vector<std::vector<FrameData>> &frameData, unsigned team, bool direction, Vector2f pos, bool owner, unsigned id, unsigned maxHit) :
		Projectile(owner, id, maxHit)
	{
		this->_position = pos;
		this->_dir = direction ? 1 : -1;
		this->_direction = direction;
		this->_team = team;
		this->_moves[0] = frameData;
	}

	void Projectile::hit(IObject &other, const FrameData *data)
	{
		Object::hit(other, data);

		auto proj = dynamic_cast<Projectile *>(&other);

		if (!proj) {
			this->nbHit++;
			return;
		}

		auto odata = other.getCurrentFrameData();

		if (data->priority) {
			if (!odata->priority || *odata->priority < *data->priority)
				proj->_dead = true;
			else if (odata->priority && *odata->priority > *data->priority)
				this->_dead = true;
			else
				this->nbHit++;
		} else if (odata->priority)
			this->_dead = true;
		else
			this->nbHit++;
	}

	void Projectile::getHit(IObject &other, const FrameData *odata)
	{
		Object::getHit(other, odata);

		auto proj = dynamic_cast<Projectile *>(&other);

		if (!proj) {
			this->nbHit++;
			return;
		}

		auto data = other.getCurrentFrameData();

		if (data->priority) {
			if (!odata->priority || *odata->priority < *data->priority)
				proj->_dead = true;
			else if (odata->priority && *odata->priority > *data->priority)
				this->_dead = true;
			else
				this->nbHit++;
		} else if (odata->priority)
			this->_dead = true;
		else
			this->nbHit++;
	}

	bool Projectile::isDead() const
	{
		return Object::isDead() || this->nbHit >= this->maxHit;
	}

	void Projectile::update()
	{
		Object::update();
		this->_dead |=
			this->_position.x < -300 ||
			this->_position.x > 1300 ||
			this->_position.y < -300 ||
			this->_position.y > 1300;
	}

	unsigned int Projectile::getClassId() const
	{
		return 2;
	}

	bool Projectile::getOwner() const
	{
		return owner;
	}

	unsigned int Projectile::getId() const
	{
		return id;
	}

	unsigned int Projectile::getBufferSize() const
	{
		return Object::getBufferSize() + sizeof(Data);
	}

	void Projectile::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		Object::copyToBuffer(data);
		dat->nbHit = this->nbHit;
		dat->maxHit = this->maxHit;
	}

	void Projectile::restoreFromBuffer(void *data)
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		Object::restoreFromBuffer(data);
		this->nbHit = dat->nbHit;
		this->maxHit = dat->maxHit;
	}

	void Projectile::_onMoveEnd(const FrameData &lastData)
	{
		if (!lastData.specialMarker)
			return Object::_onMoveEnd(lastData);
		this->_actionBlock++;
		my_assert2(this->_moves.at(this->_action).size() != this->_actionBlock, "Subobject " + std::to_string(this->_action) + " is missing block " + std::to_string(this->_actionBlock));
		Object::_onMoveEnd(lastData);
	}

	size_t Projectile::printDifference(const char *msgStart, void *data1, void *data2) const
	{
		auto length = Object::printDifference(msgStart, data1, data2);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		if (dat1->nbHit != dat2->nbHit)
			game->logger.fatal(std::string(msgStart) + "Projectile::nbHit: " + std::to_string(dat1->nbHit) + " vs " + std::to_string(dat2->nbHit));
		if (dat1->maxHit != dat2->maxHit)
			game->logger.fatal(std::string(msgStart) + "Projectile::maxHit: " + std::to_string(dat1->maxHit) + " vs " + std::to_string(dat2->maxHit));
		return length + sizeof(Data);
	}
}
