//
// Created by PinkySmile on 18/09/2021
//

#include "AObject.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"

namespace Battle
{
	bool Rectangle::intersect(const Rectangle &other)
	{
		return Rectangle::intersect(this->pt1, this->pt2, other.pt1, other.pt2) ||
		       Rectangle::intersect(this->pt2, this->pt3, other.pt1, other.pt2) ||
		       Rectangle::intersect(this->pt3, this->pt4, other.pt1, other.pt2) ||
		       Rectangle::intersect(this->pt4, this->pt1, other.pt1, other.pt2) ||

		       Rectangle::intersect(this->pt1, this->pt2, other.pt2, other.pt3) ||
		       Rectangle::intersect(this->pt2, this->pt3, other.pt2, other.pt3) ||
		       Rectangle::intersect(this->pt3, this->pt4, other.pt2, other.pt3) ||
		       Rectangle::intersect(this->pt4, this->pt1, other.pt2, other.pt3) ||

		       Rectangle::intersect(this->pt1, this->pt2, other.pt3, other.pt4) ||
		       Rectangle::intersect(this->pt2, this->pt3, other.pt3, other.pt4) ||
		       Rectangle::intersect(this->pt3, this->pt4, other.pt3, other.pt4) ||
		       Rectangle::intersect(this->pt4, this->pt1, other.pt3, other.pt4) ||

		       Rectangle::intersect(this->pt1, this->pt2, other.pt4, other.pt1) ||
		       Rectangle::intersect(this->pt2, this->pt3, other.pt4, other.pt1) ||
		       Rectangle::intersect(this->pt3, this->pt4, other.pt4, other.pt1) ||
		       Rectangle::intersect(this->pt4, this->pt1, other.pt4, other.pt1);
	}

	bool Rectangle::intersect(const Vector2f &A, const Vector2f &B, const Vector2f &C, const Vector2f &D)
	{
		auto AB = B - A;
		auto CD = D - C;

		if (CD.y * AB.x == CD.x * AB.y)
			return false;

		auto u = ((A.y - C.y) * AB.x + (C.x - A.x) * AB.y) / (CD.y * AB.x - CD.x * AB.y);
		auto t = AB.x == 0 ? (C.y + u * CD.y - A.y) / AB.y : (C.x + u * CD.x - A.x) / AB.x;

		return u >= 0 && u <= 1 && t >= 0 && t <= 1;
	}

	std::vector<std::vector<Vector2f>> Rectangle::getIntersectionPoints(const Rectangle &other)
	{
		std::vector<std::vector<Vector2f>> result;
		std::vector<Vector2f> tmp;

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				auto A = (&this->pt1)[j];
				auto B = (&this->pt1)[(j + 1) % 4];
				auto C = (&other.pt1)[i];
				auto D = (&other.pt1)[(i + 1) % 4];
				auto AB = B - A;
				auto CD = D - C;

				if (CD.y * AB.x == CD.x * AB.y)
					continue;

				auto u = ((A.y - C.y) * AB.x + (C.x - A.x) * AB.y) / (CD.y * AB.x - CD.x * AB.y);
				auto t = AB.x == 0 ? (C.y + u * CD.y - A.y) / AB.y : (C.x + u * CD.x - A.x) / AB.x;

				if (u < 0 || u > 1 || t < 0 || t > 1)
					continue;

				auto r = C + CD * u;

				tmp.emplace_back(r);
			}
			if (!tmp.empty())
				result.push_back(tmp);
			tmp.clear();
		}
		return result;
	}

	bool Rectangle::isIn(const Rectangle &other)
	{
		Vector2f maxPt1{
			std::max(this->pt1.x, std::max(this->pt2.x, std::max(this->pt3.x, this->pt4.x))),
			std::max(this->pt1.y, std::max(this->pt2.y, std::max(this->pt3.y, this->pt4.y)))
		};
		Vector2f maxPt2{
			std::max(other.pt1.x, std::max(other.pt2.x, std::max(other.pt3.x, other.pt4.x))),
			std::max(other.pt1.y, std::max(other.pt2.y, std::max(other.pt3.y, other.pt4.y)))
		};
		Vector2f minPt1{
			std::min(this->pt1.x, std::min(this->pt2.x, std::min(this->pt3.x, this->pt4.x))),
			std::min(this->pt1.y, std::min(this->pt2.y, std::min(this->pt3.y, this->pt4.y)))
		};
		Vector2f minPt2{
			std::min(other.pt1.x, std::min(other.pt2.x, std::min(other.pt3.x, other.pt4.x))),
			std::min(other.pt1.y, std::min(other.pt2.y, std::min(other.pt3.y, other.pt4.y)))
		};

		return maxPt1.x < maxPt2.x && maxPt1.y < maxPt2.y && minPt1.x > minPt2.x && minPt1.y > minPt2.y;
	}

	void AObject::render() const
	{
		auto &data = *this->getCurrentFrameData();
		auto scale = Vector2f{
			static_cast<float>(data.size.x) / data.textureBounds.size.x,
			static_cast<float>(data.size.y) / data.textureBounds.size.y
		};
		auto result = this->_position;
		auto realPos = this->_position;
		auto center = data.textureBounds.size / 2.f + Vector2f{-data.offset.x / 2.f * this->_dir, static_cast<float>(data.offset.y)};

		realPos.y *= -1;
		result.y *= -1;
		result += Vector2f{
			data.size.x / -2.f - data.offset.x * !this->_direction * 2.f,
			-static_cast<float>(data.size.y) + data.offset.y
		};
		result += Vector2f{
			data.textureBounds.size.x * scale.x / 2,
			data.textureBounds.size.y * scale.y / 2
		};
		this->_sprite.setOrigin(center);
		this->_sprite.setRotation(this->_rotation * 180 / M_PI);
		this->_sprite.setPosition(result);
		this->_sprite.setScale(this->_dir * scale.x, scale.y);
		this->_sprite.textureHandle = data.textureHandle;
		this->_sprite.setTextureRect(data.textureBounds);
		game.textureMgr.render(this->_sprite);
		if (this->showBoxes) {
			sf::RectangleShape rect;
			auto angleDeg = this->_rotation * 180 / M_PI;

			rect.setOutlineThickness(1);
			rect.setOutlineColor(sf::Color{0x00, 0xFF, 0x00, 0xFF});
			rect.setFillColor(sf::Color{0x00, 0xFF, 0x00, 0x60});
			for (auto &hurtBox : data.hurtBoxes) {
				auto box = this->_applyModifiers(hurtBox);

				rect.setRotation(angleDeg);
				rect.setOrigin(box.size / 2);
				rect.setPosition((box.pos + realPos + box.size / 2).rotation(this->_rotation, result));
				rect.setSize(box.size);
				game.screen->draw(rect);
			}

			rect.setOutlineColor(sf::Color{0xFF, 0x00, 0x00, 0xFF});
			rect.setFillColor(sf::Color{0xFF, 0x00, 0x00, 0x60});
			for (auto &hitBox : data.hitBoxes) {
				auto box = this->_applyModifiers(hitBox);

				rect.setOrigin(box.size / 2);
				rect.setRotation(angleDeg);
				rect.setPosition((box.pos + realPos + box.size / 2).rotation(this->_rotation, result));
				rect.setSize(box.size);
				game.screen->draw(rect);
			}

			rect.setRotation(0);
			if (data.collisionBox) {
				auto box = this->_applyModifiers(*data.collisionBox);

				rect.setOrigin(box.size / 2);
				rect.setOutlineColor(sf::Color{0xFF, 0xFF, 0x00, 0xFF});
				rect.setFillColor(sf::Color{0xFF, 0xFF, 0x00, 0x60});
				rect.setRotation(0);
				rect.setPosition(box.pos + realPos + box.size / 2);
				rect.setSize(box.size);
				game.screen->draw(rect);
			}

			rect.setOrigin(4.5, 4.5);
			rect.setOutlineThickness(2);
			rect.setOutlineColor(sf::Color::White);
			rect.setFillColor(sf::Color::Black);
			rect.setPosition(realPos);
			rect.setSize({9, 9});
			game.screen->draw(rect);
		}
	}

	void AObject::update()
	{
		this->_tickMove();
		this->_applyMoveAttributes();
	}

	void AObject::reset()
	{
		this->_rotation = this->_baseRotation;
		this->_gravity = this->_baseGravity;
		this->_hp = this->_baseHp;
		this->_airDrag = this->_baseAirDrag;
		this->_groundDrag = this->_baseGroundDrag;
	}

	bool AObject::isDead() const
	{
		return this->_dead;
	}

	void AObject::hit(IObject &other, const FrameData *)
	{
		char buffer[36];

		sprintf(buffer, "0x%08llX has hit 0x%08llX", (unsigned long long)this, (unsigned long long)&other);
		logger.debug(buffer);
		this->_hasHit = true;
	}

	void AObject::getHit(IObject &other, const FrameData *)
	{
		char buffer[38];

		sprintf(buffer, "0x%08llX is hit by 0x%08llX", (unsigned long long)this, (unsigned long long)&other);
		logger.debug(buffer);
	}

	bool AObject::hits(IObject &other) const
	{
		auto *oData = other.getCurrentFrameData();
		auto *mData = this->getCurrentFrameData();

		if (!mData || !oData || this->_hasHit)
			return false;

		auto asAObject = dynamic_cast<AObject *>(&other);

		if (asAObject && asAObject->_team == this->_team)
			return false;

		if (oData->dFlag.invulnerable && !mData->oFlag.grab && !mData->dFlag.projectile)
			return false;
		if (oData->dFlag.projectileInvul && mData->dFlag.projectile)
			return false;
		if (oData->dFlag.grabInvulnerable && mData->oFlag.grab)
			return false;

		auto mCenter = this->_position;
		auto oCenter = asAObject->_position;
		auto mScale = Vector2f{
			static_cast<float>(mData->size.x) / mData->textureBounds.size.x,
			static_cast<float>(mData->size.y) / mData->textureBounds.size.y
		};
		auto oScale = Vector2f{
			static_cast<float>(oData->size.x) / oData->textureBounds.size.x,
			static_cast<float>(oData->size.y) / oData->textureBounds.size.y
		};

		mCenter.y *= -1;
		mCenter += Vector2f{
			mData->size.x / -2.f - mData->offset.x * !this->_direction * 2.f,
			-static_cast<float>(mData->size.y) + mData->offset.y
		};
		mCenter += Vector2f{
			mData->textureBounds.size.x * mScale.x / 2,
			mData->textureBounds.size.y * mScale.y / 2
		};
		oCenter.y *= -1;
		oCenter += Vector2f{
			oData->size.x / -2.f - oData->offset.x * !asAObject->_direction * 2.f,
			-static_cast<float>(oData->size.y) + oData->offset.y
		};
		oCenter += Vector2f{
			oData->textureBounds.size.x * oScale.x / 2,
			oData->textureBounds.size.y * oScale.y / 2
		};

		for (auto &hurtBox : oData->hurtBoxes) {
			auto _hurtBox = asAObject->_applyModifiers(hurtBox);
			Rectangle __hurtBox;

			__hurtBox.pt1 = _hurtBox.pos.rotation(asAObject->_rotation, oCenter)                                                      + Vector2f{asAObject->_position.x, -asAObject->_position.y};
			__hurtBox.pt2 = (_hurtBox.pos + Vector2f{0, static_cast<float>(_hurtBox.size.y)}).rotation(asAObject->_rotation, oCenter) + Vector2f{asAObject->_position.x, -asAObject->_position.y};
			__hurtBox.pt3 = (_hurtBox.pos + _hurtBox.size).rotation(asAObject->_rotation, oCenter)                                    + Vector2f{asAObject->_position.x, -asAObject->_position.y};
			__hurtBox.pt4 = (_hurtBox.pos + Vector2f{static_cast<float>(_hurtBox.size.x), 0}).rotation(asAObject->_rotation, oCenter) + Vector2f{asAObject->_position.x, -asAObject->_position.y};
			for (auto &hitBox : mData->hitBoxes) {
				auto _hitBox = this->_applyModifiers(hitBox);
				Rectangle __hitBox;

				__hitBox.pt1 = _hitBox.pos.rotation(this->_rotation, mCenter)                                                     + Vector2f{this->_position.x, -this->_position.y};
				__hitBox.pt2 = (_hitBox.pos + Vector2f{0, static_cast<float>(_hitBox.size.y)}).rotation(this->_rotation, mCenter) + Vector2f{this->_position.x, -this->_position.y};
				__hitBox.pt3 = (_hitBox.pos + _hitBox.size).rotation(this->_rotation, mCenter)                                    + Vector2f{this->_position.x, -this->_position.y};
				__hitBox.pt4 = (_hitBox.pos + Vector2f{static_cast<float>(_hitBox.size.x), 0}).rotation(this->_rotation, mCenter) + Vector2f{this->_position.x, -this->_position.y};
				if (__hurtBox.intersect(__hitBox) || __hurtBox.isIn(__hitBox) || __hitBox.isIn(__hurtBox))
					return true;
			}
		}
		return false;
	}

	const FrameData *AObject::getCurrentFrameData() const
	{
		try {
			return &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
		} catch (std::out_of_range &) {
			//TODO : Add proper exceptions
			throw std::invalid_argument("Invalid action: Action " + std::to_string(this->_action) + " was not found.");
		}
	}

	Box AObject::_applyModifiers(Box box) const
	{
		if (this->_direction)
			return box;

		return Box{
			{-box.pos.x - static_cast<int>(box.size.x), box.pos.y},
			box.size
		};
	}

	void AObject::_applyNewAnimFlags()
	{
		auto data = this->getCurrentFrameData();

		if (!data)
			return;
		this->_hasHit &= data->oFlag.resetHits;
		if (data->dFlag.resetRotation)
			this->_rotation = 0;
	}

	bool AObject::_hasMove(unsigned action) const
	{
		return this->_moves.find(action) != this->_moves.end();
	}

	bool AObject::_startMove(unsigned int action)
	{
		if (!this->_hasMove(action)) {
			logger.debug("Cannot start action " + std::to_string(action));
			return false;
		}

		auto &data = this->_moves.at(action)[0][0];

		if (!this->_canStartMove(action, data))
			return false;
		this->_forceStartMove(action);
		return true;
	}

	bool AObject::_canStartMove(unsigned, const FrameData &)
	{
		return true;
	}

	void AObject::_forceStartMove(unsigned int action)
	{
		this->_action = action;
		this->_actionBlock = 0;
		this->_animationCtr = 0;
		this->_animation = 0;
		this->_hasHit = false;
		this->_applyNewAnimFlags();
	}

	void AObject::_onMoveEnd(const FrameData &)
	{
		this->_animation = 0;
		this->_applyNewAnimFlags();
	}

	bool AObject::_isGrounded() const
	{
		auto data = this->getCurrentFrameData();

		return !data || !data->dFlag.airborne;
	}

	void AObject::collide(IObject &other)
	{
		auto myData = this->getCurrentFrameData();
		auto data = other.getCurrentFrameData();
		auto asAObject = dynamic_cast<AObject *>(&other);

		if (!asAObject)
			return;

		auto myBox = this->_applyModifiers(*myData->collisionBox);
		auto opBox = asAObject->_applyModifiers(*data->collisionBox);
		float myDiff;
		float opDiff;

		if (this->_position.x > asAObject->_position.x || (this->_position.x == asAObject->_position.x && this->_cornerPriority > asAObject->_cornerPriority)) {
			opDiff = (this->_position.x      + myBox.pos.to<float>().x - opBox.pos.to<float>().x  - opBox.size.to<float>().x) - asAObject->_position.x;
			myDiff = (asAObject->_position.x + opBox.pos.to<float>().x + opBox.size.to<float>().x - myBox.pos.to<float>().x)  - this->_position.x;
		} else {
			opDiff = (this->_position.x      + myBox.pos.to<float>().x + myBox.size.to<float>().x - opBox.pos.to<float>().x)  - asAObject->_position.x;
			myDiff = (asAObject->_position.x + opBox.pos.to<float>().x - myBox.pos.to<float>().x  - myBox.size.to<float>().x) - this->_position.x;
		}
		this->_position.x += myDiff * 0.5f;
		asAObject->_position.x += opDiff * 0.5f;
	}

	bool AObject::collides(IObject &other) const
	{
		auto myData = this->getCurrentFrameData();

		if (!myData || !myData->collisionBox)
			return false;

		auto data = other.getCurrentFrameData();

		if (!data || !data->collisionBox)
			return false;

		auto asAObject = dynamic_cast<AObject *>(&other);

		if (!asAObject)
			return false;

		auto _hitBox = this->_applyModifiers(*myData->collisionBox);
		auto _hurtBox = asAObject->_applyModifiers(*data->collisionBox);

		_hitBox.pos.x += this->_position.x;
		_hitBox.pos.y -= this->_position.y;
		_hurtBox.pos.x += asAObject->_position.x;
		_hurtBox.pos.y -= asAObject->_position.y;
		return static_cast<float>(_hurtBox.pos.x)                   < static_cast<float>(_hitBox.pos.x) + _hitBox.size.x &&
		       static_cast<float>(_hurtBox.pos.y)                   < static_cast<float>(_hitBox.pos.y) + _hitBox.size.y &&
		       static_cast<float>(_hurtBox.pos.x) + _hurtBox.size.x > static_cast<float>(_hitBox.pos.x)                  &&
		       static_cast<float>(_hurtBox.pos.y) + _hurtBox.size.y > static_cast<float>(_hitBox.pos.y);
	}

	void AObject::_applyMoveAttributes()
	{
		auto data = this->getCurrentFrameData();

		if (data->dFlag.resetSpeed)
			this->_speed = {0, 0};
		if (data->dFlag.resetRotation)
			this->_rotation = this->_baseRotation;
		this->_rotation += data->rotation;
		this->_speed += Vector2f{this->_dir * data->speed.x, static_cast<float>(data->speed.y)};
		this->_position += this->_speed;
		if (!this->_isGrounded()) {
			this->_speed *= 0.99;
			this->_speed += this->_gravity;
		} else
			this->_speed *= 0.75;
	}

	void AObject::_tickMove()
	{
		auto data = this->getCurrentFrameData();

		this->_animationCtr++;
		while (this->_animationCtr >= data->duration) {
			this->_animationCtr = 0;
			this->_animation++;
			this->_hasHit &= this->_animation < this->_moves.at(this->_action)[this->_actionBlock].size();
			if (this->_animation == this->_moves.at(this->_action)[this->_actionBlock].size())
				this->_onMoveEnd(this->_moves.at(this->_action)[this->_actionBlock].back());
			data = &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
			this->_gravity = data->gravity ? *data->gravity : this->_baseGravity;
			this->_hasHit &= !data->oFlag.resetHits;
		}
	}
}
