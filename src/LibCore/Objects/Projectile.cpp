//
// Created by PinkySmile on 02/02/2022.
//

#include "Resources/Game.hpp"
#include "Projectile.hpp"

namespace SpiralOfFate
{
	Projectile::Projectile(bool owner, unsigned int id, unsigned maxHit, bool loop, unsigned endBlock, bool disableOnHit) :
		_maxHit(maxHit),
		_id(id),
		_endBlock(endBlock),
		_owner(owner),
		_loop(loop),
		_disableOnHit(disableOnHit)
	{
	}

	Projectile::Projectile(const std::vector<std::vector<FrameData>> &frameData, unsigned team, bool direction, Vector2f pos, bool owner, unsigned id, unsigned maxHit, bool loop, unsigned endBlock, bool disableOnHit) :
		Projectile(owner, id, maxHit, loop, endBlock, disableOnHit)
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
		if (this->_disableOnHit && (this->_owner ? game->battleMgr->getRightCharacter() : game->battleMgr->getLeftCharacter())->isHit())
			this->_disableObject();
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
		return this->_owner;
	}

	unsigned int Projectile::getId() const
	{
		return this->_id;
	}

	unsigned int Projectile::getBufferSize() const
	{
		return Object::getBufferSize() + sizeof(Data);
	}

	void Projectile::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		Object::copyToBuffer(data);
		dat->disabled = this->_disabled;
		dat->maxHit = this->_maxHit;
	}

	void Projectile::restoreFromBuffer(void *data)
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		Object::restoreFromBuffer(data);
		this->_disabled = dat->disabled;
		this->_maxHit = dat->maxHit;
	}

	void Projectile::_onMoveEnd(const FrameData &lastData)
	{
		if (this->_endBlock >= this->_actionBlock) {
			this->_dead = true;
			return;
		}
		if (this->_endBlock != this->_actionBlock) {
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

		if (dat1->disabled != dat2->disabled)
			game->logger.fatal(std::string(msgStart) + "Projectile::disabled: " + std::to_string(dat1->disabled) + " vs " + std::to_string(dat2->disabled));
		if (dat1->maxHit != dat2->maxHit)
			game->logger.fatal(std::string(msgStart) + "Projectile::maxHit: " + std::to_string(dat1->maxHit) + " vs " + std::to_string(dat2->maxHit));
		return length + sizeof(Data);
	}

	void Projectile::_disableObject()
	{
		if (this->_disabled)
			return;
		this->_disabled = true;
		for (auto &move : this->_moves)
			for (auto &block : move.second)
				for (auto &frame : block) {
					frame.hitBoxes.clear();
					frame.hurtBoxes.clear();
				}
	}
}
