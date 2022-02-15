//
// Created by PinkySmile on 18/09/2021
//

#include "Object.hpp"
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

	void Object::render() const
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
			for (auto &hurtBox : this->_getModifiedHurtBoxes())
				this->_drawBox(hurtBox, sf::Color::Green);
			for (auto &hitBox : this->_getModifiedHitBoxes())
				this->_drawBox(hitBox, sf::Color::Red);

			if (data.collisionBox) {
				auto box = this->_applyModifiers(*data.collisionBox);

				this->_drawBox({
					realPos + box.pos,
					realPos + Vector2f{
						static_cast<float>(box.pos.x),
						static_cast<float>(box.pos.y) + box.size.y
					},
					realPos + box.pos + box.size,
					realPos + Vector2f{
						static_cast<float>(box.pos.x) + box.size.x,
						static_cast<float>(box.pos.y)
					}
				}, sf::Color::Yellow);
			}

			this->_drawBox({
				{realPos.x - 4.5f, realPos.y - 4.5f},
				{realPos.x + 4.5f, realPos.y - 4.5f},
				{realPos.x + 4.5f, realPos.y + 4.5f},
				{realPos.x - 4.5f, realPos.y + 4.5f},
			}, sf::Color::Black);
		}
	}

	void Object::update()
	{
		this->_tickMove();
		this->_applyMoveAttributes();
	}

	void Object::reset()
	{
		this->_rotation = this->_baseRotation;
		this->_gravity = this->_baseGravity;
		this->_hp = this->_baseHp;
		this->_airDrag = this->_baseAirDrag;
		this->_groundDrag = this->_baseGroundDrag;
	}

	bool Object::isDead() const
	{
		return this->_dead;
	}

	void Object::hit(IObject &other, const FrameData *)
	{
		char buffer[36];

		sprintf(buffer, "0x%08llX has hit 0x%08llX", (unsigned long long)this, (unsigned long long)&other);
		logger.debug(buffer);
		this->_hasHit = true;
	}

	void Object::getHit(IObject &other, const FrameData *)
	{
		char buffer[38];

		sprintf(buffer, "0x%08llX is hit by 0x%08llX", (unsigned long long)this, (unsigned long long)&other);
		logger.debug(buffer);
	}

	bool Object::hits(const IObject &other) const
	{
		auto *oData = other.getCurrentFrameData();
		auto *mData = this->getCurrentFrameData();

		if (!mData || !oData || this->_hasHit)
			return false;

		auto asAObject = dynamic_cast<const Object *>(&other);

		if (!asAObject || asAObject->_team == this->_team)
			return false;

		if (oData->dFlag.invulnerable && !mData->oFlag.grab && !mData->dFlag.projectile)
			return false;
		if (oData->dFlag.projectileInvul && mData->dFlag.projectile)
			return false;
		if (oData->dFlag.grabInvulnerable && mData->oFlag.grab)
			return false;
		if (oData->dFlag.airborne != mData->dFlag.airborne && mData->oFlag.grab)
			return false;
		if (mData->dFlag.spiritInvul && oData->oFlag.spiritElement)
			return false;
		if (mData->dFlag.voidInvul && oData->oFlag.voidElement)
			return false;
		if (mData->dFlag.matterInvul && oData->oFlag.matterElement)
			return false;
		if (mData->dFlag.neutralInvul && (oData->oFlag.matterElement == oData->oFlag.voidElement && oData->oFlag.voidElement == oData->oFlag.spiritElement))
			return false;

		for (auto &hurtBox : asAObject->_getModifiedHurtBoxes())
			for (auto &hitBox : this->_getModifiedHitBoxes())
				if (hurtBox.intersect(hitBox) || hurtBox.isIn(hitBox) || hitBox.isIn(hurtBox))
					return true;
		return false;
	}

	const FrameData *Object::getCurrentFrameData() const
	{
		try {
			return &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
		} catch (std::out_of_range &) {
			//TODO : Add proper exceptions
			throw std::invalid_argument("Invalid action: Action " + std::to_string(this->_action) + " was not found.");
		}
	}

	Box Object::_applyModifiers(Box box) const
	{
		if (this->_direction)
			return box;

		return Box{
			{-box.pos.x - static_cast<int>(box.size.x), box.pos.y},
			box.size
		};
	}

	void Object::_applyNewAnimFlags()
	{
		auto data = this->getCurrentFrameData();

		if (!data)
			return;
		this->_hasHit &= !data->oFlag.resetHits;
		if (data->dFlag.resetRotation)
			this->_rotation = 0;
		Battle::game.soundMgr.play(data->soundHandle);
	}

	bool Object::_hasMove(unsigned action) const
	{
		return this->_moves.find(action) != this->_moves.end();
	}

	bool Object::_startMove(unsigned int action)
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

	bool Object::_canStartMove(unsigned, const FrameData &)
	{
		return true;
	}

	void Object::_forceStartMove(unsigned int action)
	{
		this->_action = action;
		this->_actionBlock = 0;
		this->_animationCtr = 0;
		this->_animation = 0;
		this->_hasHit = false;
		this->_applyNewAnimFlags();
	}

	void Object::_onMoveEnd(const FrameData &)
	{
		this->_animation = 0;
		this->_applyNewAnimFlags();
	}

	bool Object::_isGrounded() const
	{
		return this->_position.y <= 0 || this->_isOnPlatform();
	}

	void Object::collide(IObject &other)
	{
		auto myData = this->getCurrentFrameData();
		auto data = other.getCurrentFrameData();
		auto asAObject = dynamic_cast<Object *>(&other);

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

	bool Object::collides(const IObject &other) const
	{
		auto myData = this->getCurrentFrameData();

		if (!myData || !myData->collisionBox)
			return false;

		auto data = other.getCurrentFrameData();

		if (!data || !data->collisionBox)
			return false;

		auto asAObject = dynamic_cast<const Object *>(&other);

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

	void Object::_applyMoveAttributes()
	{
		auto data = this->getCurrentFrameData();
		auto oldPos = this->_position;

		if (data->dFlag.resetSpeed)
			this->_speed = {0, 0};
		if (data->dFlag.resetRotation)
			this->_rotation = this->_baseRotation;
		this->_rotation += data->rotation;
		this->_speed += Vector2f{this->_dir * data->speed.x, static_cast<float>(data->speed.y)};
		this->_position += this->_speed;
		this->_checkPlatforms(oldPos);
		if (!this->_isGrounded()) {
			this->_speed *= 0.99;
			this->_speed += this->_gravity;
		} else
			this->_speed *= 0.75;
	}

	void Object::_checkPlatforms(Vector2f oldPos)
	{
		for (auto &platform : game.battleMgr->getPlatforms()) {
			if (platform->isDestructed())
				continue;
			if (this->_position.x < platform->_position.x - platform->getWidth() / 2)
				continue;
			if (this->_position.x > platform->_position.x + platform->getWidth() / 2)
				continue;
			if (this->_position.y > platform->_position.y)
				continue;
			if (oldPos.y < platform->_position.y)
				continue;
			this->_position.y = platform->_position.y;
			return;
		}
	}

	void Object::_tickMove()
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
			Battle::game.soundMgr.play(data->soundHandle);
		}
	}

	std::vector<Rectangle> Object::_getModifiedBoxes(const FrameData &data, const std::vector<Box> &boxes) const
	{
		std::vector<Rectangle> result;
		Vector2f center{
			data.offset.x * (this->_direction ? -1.f : 1.f),
			data.size.y / -2.f - data.offset.y
		};

		for (auto &box : boxes) {
			auto _box = this->_applyModifiers(box);
			Rectangle __box;
			auto real = Vector2f{this->_position.x, -this->_position.y};

			__box.pt1 = real + _box.pos.rotation(this->_rotation, center);
			__box.pt2 = real + (_box.pos + Vector2f{0, static_cast<float>(_box.size.y)}).rotation(this->_rotation, center);
			__box.pt3 = real + (_box.pos + _box.size).rotation(this->_rotation, center);
			__box.pt4 = real + (_box.pos + Vector2f{static_cast<float>(_box.size.x), 0}).rotation(this->_rotation, center);
			result.push_back(__box);
		}
		return result;
	}

	std::vector<Rectangle> Object::_getModifiedHurtBoxes() const
	{
		return this->_getModifiedBoxes(*this->getCurrentFrameData(), this->getCurrentFrameData()->hurtBoxes);
	}

	std::vector<Rectangle> Object::_getModifiedHitBoxes() const
	{
		return this->_getModifiedBoxes(*this->getCurrentFrameData(), this->getCurrentFrameData()->hitBoxes);
	}

	void Object::_drawBox(const Rectangle &box, const sf::Color &color) const
	{
		sf::VertexArray arr{sf::Quads, 4};
		sf::VertexArray arr2{sf::LineStrip, 5};

		for (int i = 0; i < 4; i++) {
			arr[i].color = color;
			arr[i].color.a *= 0x30 / 255.f;
			arr[i].position = (&box.pt1)[i];
		}
		game.screen->draw(arr);

		for (unsigned i = 0; i < 5; i++) {
			arr2[i].color = color;
			arr2[i].position = (&box.pt1)[i % 4];
		}
		game.screen->draw(arr2);
	}

	void Object::kill()
	{
		this->_dead = true;
	}

	unsigned int Object::getBufferSize() const
	{
		return sizeof(Data);
	}

	void Object::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>(data);

		dat->_position = this->_position;
		dat->_speed = this->_speed;
		dat->_gravity = this->_gravity;
		dat->_airDrag = this->_airDrag;
		dat->_groundDrag = this->_groundDrag;
		dat->_action = this->_action;
		dat->_actionBlock = this->_actionBlock;
		dat->_animation = this->_animation;
		dat->_animationCtr = this->_animationCtr;
		dat->_hp = this->_hp;
		dat->_rotation = this->_rotation;
		dat->_team = this->_team;
		dat->_dead = this->_dead;
		dat->_hasHit = this->_hasHit;
		dat->_direction = this->_direction;
		dat->_cornerPriority = this->_cornerPriority;
		dat->_dir = this->_dir;
	}

	void Object::restoreFromBuffer(void *data)
	{
		auto dat = reinterpret_cast<Data *>(data);

		this->_position = dat->_position;
		this->_speed = dat->_speed;
		this->_gravity = dat->_gravity;
		this->_airDrag = dat->_airDrag;
		this->_groundDrag = dat->_groundDrag;
		this->_action = dat->_action;
		this->_actionBlock = dat->_actionBlock;
		this->_animation = dat->_animation;
		this->_animationCtr = dat->_animationCtr;
		this->_hp = dat->_hp;
		this->_rotation = dat->_rotation;
		this->_team = dat->_team;
		this->_dead = dat->_dead;
		this->_hasHit = dat->_hasHit;
		this->_direction = dat->_direction;
		this->_cornerPriority = dat->_cornerPriority;
		this->_dir = dat->_dir;
	}

	unsigned int Object::getClassId() const
	{
		return 0;
	}

	bool Object::_isOnPlatform() const
	{
		return std::any_of(
			game.battleMgr->getPlatforms().begin(),
			game.battleMgr->getPlatforms().end(),
			[this](auto &obj) {
				if (obj->isDestructed())
					return false;
				if (this->_position.y != obj->_position.y)
					return false;
				if (this->_position.x < obj->_position.x - obj->getWidth() / 2)
					return false;
				if (this->_position.x > obj->_position.x + obj->getWidth() / 2)
					return false;
				return true;
			}
		);
	}
}
