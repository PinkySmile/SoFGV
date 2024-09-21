//
// Created by PinkySmile on 18/09/2021.
//

#include "Object.hpp"
#include "Resources/Game.hpp"
#include "Logger.hpp"

namespace SpiralOfFate
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
		std::vector<std::vector<Vector2f>> result{4};
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

				result[i].emplace_back(r);
			}
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

	Object::Object()
	{
		this->_fdCache.setSlave();
	}

	void Object::_render(Vector2f spritePos, Vector2f scale) const
	{
		const FrameData &data = *this->getCurrentFrameData();
		Vector2f realPos = this->_position;
		Color baseTint = this->_sprite.getColor();
		Color tint = this->_sprite.getColor();
		int a;

		if (this->_fadeDir)
			a = this->_fadeTimer * 255 / this->_fadeTimerMax;
		else
			a = (this->_fadeTimerMax - this->_fadeTimer) * 255 / this->_fadeTimerMax;
		a = baseTint.a * a / 255;
		tint.a = a;
		this->_sprite.setColor(tint);
		this->_sprite.setOrigin(data.textureBounds.size / 2.f);
		this->_sprite.setRotation(this->_rotation * 180 / M_PI);
		this->_sprite.setPosition(spritePos);
		this->_sprite.setScale(scale);
		this->_sprite.setTextureRect(data.textureBounds);
		this->_sprite.textureHandle = data.textureHandle;
		game->textureMgr.render(this->_sprite);
		if (data.oFlag.spiritElement == data.oFlag.matterElement && data.oFlag.matterElement == data.oFlag.voidElement)
			tint = game->typeColors[data.oFlag.spiritElement ? TYPECOLOR_NEUTRAL : TYPECOLOR_NON_TYPED];
		else if (data.oFlag.spiritElement)
			tint = game->typeColors[TYPECOLOR_SPIRIT];
		else if (data.oFlag.matterElement)
			tint = game->typeColors[TYPECOLOR_MATTER];
		else if (data.oFlag.voidElement)
			tint = game->typeColors[TYPECOLOR_VOID];
		tint.a = a;
		this->_sprite.setColor(tint);
		this->_sprite.textureHandle = data.textureHandleEffects;
		game->textureMgr.render(this->_sprite);
		this->_sprite.setColor(baseTint);

		realPos.y *= -1;
		if (!this->showBoxes)
			return;

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
		if (dynamic_cast<const Character *>(this) == nullptr)
			game->screen->displayElement({
				static_cast<int>(this->_position.x - this->_hitStop),
				static_cast<int>(10 - this->_position.y),
				static_cast<int>(this->_hitStop * 2),
				10
			}, sf::Color::Cyan);

		if (data.hurtBoxes.empty())
			return;

		auto opPos = this->_position;
		sf::RectangleShape rect;

		opPos.y += data.textureBounds.size.y * data.scale.y / 2.f + data.offset.y;
		opPos.y *= -1;
		opPos -= Vector2i{1, 1};
		rect.setFillColor(sf::Color::Black);
		rect.setOutlineColor(sf::Color::White);
		rect.setOutlineThickness(1);
		rect.setSize({3, 3});
		rect.setPosition(opPos);
		game->screen->draw(rect);
	}

	void Object::render() const
	{
		auto &data = *this->getCurrentFrameData();
		auto result = Vector2f{data.offset.x * this->_dir, static_cast<float>(data.offset.y)} + this->_position;
		auto scale = Vector2f{
			this->_dir * data.scale.x,
			data.scale.y
		};
		auto size = Vector2f{
			data.textureBounds.size.x * data.scale.x,
			data.textureBounds.size.y * data.scale.y
		};

		result.y *= -1;
		result += Vector2f{
			!this->_direction * size.x + size.x / -2.f,
			-size.y
		};
		result += Vector2f{
			data.textureBounds.size.x * scale.x / 2,
			data.textureBounds.size.y * scale.y / 2
		};
		this->_render(result, scale);
	}

	void Object::update()
	{
		if (this->_hitStop) {
			this->_hitStop--;
			return;
		}
		this->_tickMove();
		this->_applyNewAnimFlags();
		this->_applyMoveAttributes();
	}

	void Object::reset()
	{
		this->_rotation = this->_baseRotation;
		this->_gravity = this->_baseGravity;
		this->_hp = this->_baseHp;
	}

	bool Object::isDead() const
	{
		return this->_dead;
	}

	void Object::hit(Object &, const FrameData *)
	{
		this->_hasHit = true;
	}

	void Object::getHit(Object &, const FrameData *data)
	{
		this->_hitStop = data->hitOpponentHitStop;
		game->soundMgr.play(data->hitSoundHandle);
	}

	bool Object::hits(const Object &other) const
	{
		if (this->_hasHit || this->isDisabled(other))
			return false;

		if (other._team == this->_team)
			return false;

		auto *oData = other.getCurrentFrameData();
		auto *mData = this->getCurrentFrameData();
		auto neutral = mData->oFlag.matterElement && mData->oFlag.voidElement && mData->oFlag.spiritElement;

		if (mData->hitBoxes.empty() || oData->hurtBoxes.empty())
			return false;
		if (oData->dFlag.invulnerable && !mData->oFlag.grab && !mData->dFlag.projectile)
			return false;
		if (oData->dFlag.projectileInvul && mData->dFlag.projectile)
			return false;
		if (oData->dFlag.grabInvulnerable && mData->oFlag.grab)
			return false;
		if (oData->dFlag.airborne != mData->dFlag.airborne && mData->oFlag.grab)
			return false;
		if (oData->dFlag.spiritInvul && mData->oFlag.spiritElement && !neutral)
			return false;
		if (oData->dFlag.voidInvul && mData->oFlag.voidElement && !neutral)
			return false;
		if (oData->dFlag.matterInvul && mData->oFlag.matterElement && !neutral)
			return false;
		if (oData->dFlag.neutralInvul && neutral)
			return false;
		if ((oData->dFlag.spiritInvul || oData->dFlag.voidInvul || oData->dFlag.matterInvul || oData->dFlag.neutralInvul) && !neutral)
			return false;

		auto hurtBoxes = other._getModifiedHurtBoxes();
		auto hitBoxes = this->_getModifiedHitBoxes();

		// TODO: Check if there is perf benefit in doing this
		if (other._rotation == 0 && this->_rotation == 0) {
			for (auto &hurtBox : hurtBoxes)
				for (auto &hitBox : hitBoxes) {
					if (hitBox.pt3.x < hurtBox.pt1.x)
						continue;
					if (hurtBox.pt3.x < hitBox.pt1.x)
						continue;
					if (hitBox.pt3.y < hurtBox.pt1.y)
						continue;
					if (hurtBox.pt3.y < hitBox.pt1.y)
						continue;
					return true;
				}
			return false;
		}
		for (auto &hurtBox : hurtBoxes)
			for (auto &hitBox : hitBoxes)
				if (hurtBox.intersect(hitBox) || hurtBox.isIn(hitBox) || hitBox.isIn(hurtBox))
					return true;
		return false;
	}

	const FrameData *Object::getCurrentFrameData() const
	{
		if (this->_cacheComputed)
			return &this->_fdCache;
		try {
			return &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
		} catch (std::out_of_range &) {
			throw AssertionFailedExceptionMsg("this->_hasMove(this->_action)", "Invalid action: Action " + std::to_string(this->_action) + " was not found.");
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
		if (!this->_newAnim)
			return;
		this->_newAnim = false;

		auto data = this->getCurrentFrameData();

		if (data->fadeTime == (unsigned)-1) {
			this->_fadeDir = false;
			this->_fadeTimer = 0;
			this->_fadeTimerMax = 1;
		} else if (data->fadeTime == (unsigned)-2) {
			this->_fadeDir = true;
			this->_fadeTimer = 0;
			this->_fadeTimerMax = 1;
		} else if (data->fadeTime) {
			this->_fadeDir = !this->_fadeDir;
			this->_fadeTimer = data->fadeTime;
			this->_fadeTimerMax = data->fadeTime;
		}
		this->_gravity = data->gravity ? *data->gravity : this->_baseGravity;
		this->_hasHit &= !data->oFlag.resetHits;
		if (data->dFlag.resetRotation)
			this->_rotation = 0;
		game->soundMgr.play(data->soundHandle);
	}

	bool Object::_hasMove(unsigned action) const
	{
		return this->_moves.find(action) != this->_moves.end();
	}

	bool Object::_startMove(unsigned int action)
	{
		if (!this->_hasMove(action)) {
			game->logger.debug("Cannot start action " + std::to_string(action));
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
		assert_msg(this->_moves.find(action) != this->_moves.end(), "Invalid action: Action " + std::to_string(action) + " was not found.");
		this->_action = action;
		this->_actionBlock = 0;
		this->_animationCtr = 0;
		this->_animation = 0;
		this->_hasHit = false;
		this->_rotation = 0;
		this->_newAnim = true;
	}

	void Object::_onMoveEnd(const FrameData &)
	{
		this->_animation = 0;
		this->_newAnim = true;
	}

	bool Object::_isGrounded() const
	{
		return this->_position.y <= STAGE_Y_MIN || this->_isOnPlatform();
	}

	void Object::collide(Object &other)
	{
		auto myData = this->getCurrentFrameData();
		auto data = other.getCurrentFrameData();
		auto myBox = this->_applyModifiers(*myData->collisionBox);
		auto opBox = other._applyModifiers(*data->collisionBox);
		float myDiff;
		float opDiff;

		if (this->_position.x > other._position.x || (this->_position.x == other._position.x && this->_cornerPriority > other._cornerPriority)) {
			opDiff = (this->_position.x + myBox.pos.to<float>().x - opBox.pos.to<float>().x  - opBox.size.to<float>().x) - other._position.x;
			myDiff = (other._position.x + opBox.pos.to<float>().x + opBox.size.to<float>().x - myBox.pos.to<float>().x)  - this->_position.x;
		} else {
			opDiff = (this->_position.x + myBox.pos.to<float>().x + myBox.size.to<float>().x - opBox.pos.to<float>().x)  - other._position.x;
			myDiff = (other._position.x + opBox.pos.to<float>().x - myBox.pos.to<float>().x  - myBox.size.to<float>().x) - this->_position.x;
		}
		this->_position.x += myDiff * 0.5f;
		other._position.x += opDiff * 0.5f;
	}

	bool Object::collides(const Object &other) const
	{
		auto myData = this->getCurrentFrameData();

		if (!myData || !myData->collisionBox)
			return false;

		auto data = other.getCurrentFrameData();

		if (!data || !data->collisionBox)
			return false;

		auto _hitBox = this->_applyModifiers(*myData->collisionBox);
		auto _hurtBox = other._applyModifiers(*data->collisionBox);

		_hitBox.pos.x += this->_position.x;
		_hitBox.pos.y -= this->_position.y;
		_hurtBox.pos.x += other._position.x;
		_hurtBox.pos.y -= other._position.y;
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
			this->_rotation = this->_baseRotation * this->_dir;
		this->_rotation += data->rotation * this->_dir;
		this->_rotation = std::fmod(this->_rotation, 2 * M_PI);
		this->_speed += Vector2f{this->_dir * data->speed.x, static_cast<float>(data->speed.y)};
		this->_position += this->_speed;
		this->_checkPlatforms(oldPos);
		if (data->dFlag.airborne) {
			this->_speed.x *= this->_airDrag.x;
			this->_speed.y *= this->_airDrag.y;
		} else
			this->_speed.x *= this->_groundDrag;
		if (!this->_isGrounded())
			this->_speed.y += this->_gravity.y;
		this->_speed.x += this->_gravity.x * this->_dir;
	}

	void Object::_checkPlatforms(Vector2f oldPos)
	{
		for (auto &platform : game->battleMgr->getPlatforms()) {
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
		auto data = &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];

		if (this->_fadeTimer)
			this->_fadeTimer--;
		this->_animationCtr++;
		while (this->_animationCtr >= data->duration) {
			this->_animationCtr = 0;
			this->_animation++;
			if (this->_animation == this->_moves.at(this->_action)[this->_actionBlock].size())
				this->_onMoveEnd(this->_moves.at(this->_action)[this->_actionBlock].back());
			data = &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
			this->_newAnim = true;
		}
	}

	std::vector<Rectangle> Object::_getModifiedBoxes(const FrameData &data, const std::vector<Box> &boxes) const
	{
		std::vector<Rectangle> result;
		Vector2f center{
			data.offset.x * this->_dir,
			data.textureBounds.size.y * data.scale.y / -2.f - data.offset.y
		};

		for (auto &box : boxes) {
			auto _box = this->_applyModifiers(box);
			Rectangle __box;
			auto real = Vector2f{this->_position.x, -this->_position.y};

			__box.pt1 = real + _box.pos.rotation(this->_rotation, center);
			__box.pt2 = real + (_box.pos + Vector2f{0, static_cast<float>(_box.size.y)}).rotation(this->_rotation, center);
			__box.pt3 = real + (_box.pos + _box.size.to<int>()).rotation(this->_rotation, center);
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
		game->screen->draw(arr);

		for (unsigned i = 0; i < 5; i++) {
			arr2[i].color = color;
			arr2[i].position = (&box.pt1)[i % 4];
		}
		game->screen->draw(arr2);
	}

	void Object::kill()
	{
		this->_dead = true;
	}

	unsigned int Object::getBufferSize() const
	{
		return sizeof(Data) + this->_fdCache.getBufferSize();
	}

	void Object::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + this->_fdCache.getBufferSize());

		//TODO: The cache shouldn't need to be saved. There definitely is something wrong here...
		this->_fdCache.copyToBuffer(data);
		game->logger.verbose("Saving Object (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
		dat->_position = this->_position;
		dat->_speed = this->_speed;
		dat->_gravity = this->_gravity;
		dat->_action = this->_action;
		dat->_hitStop = this->_hitStop;
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
		dat->_newAnim = this->_newAnim;
		dat->_fadeTimer = this->_fadeTimer;
		dat->_fadeTimerMax = this->_fadeTimerMax;
		dat->_fadeDir = this->_fadeDir;
	}

	void Object::restoreFromBuffer(void *data)
	{
		//TODO: The cache shouldn't need to be saved. There definitely is something wrong here...
		this->_fdCache.restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + this->_fdCache.getBufferSize());

		this->_position = dat->_position;
		this->_speed = dat->_speed;
		this->_gravity = dat->_gravity;
		this->_action = dat->_action;
		this->_hitStop = dat->_hitStop;
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
		this->_newAnim = dat->_newAnim;
		this->_fadeTimer = dat->_fadeTimer;
		this->_fadeTimerMax = dat->_fadeTimerMax;
		this->_fadeDir = dat->_fadeDir;
		game->logger.verbose("Restored Object @" + std::to_string((uintptr_t)dat));
	}

	unsigned int Object::getClassId() const
	{
		return CLASS_ID;
	}

	bool Object::_isOnPlatform() const
	{
		return this->_speed.y <= 0 && std::any_of(
			game->battleMgr->getPlatforms().begin(),
			game->battleMgr->getPlatforms().end(),
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

	size_t Object::printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const
	{
		auto length = FrameData::printDifference(msgStart, data1, data2, startOffset);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		game->logger.info("Object @" + std::to_string(startOffset + length));
		if (dat1->_position != dat2->_position)
			game->logger.fatal(std::string(msgStart) + "Object::_position: (" + std::to_string(dat1->_position.x) + ", " + std::to_string(dat1->_position.y) + ") vs (" + std::to_string(dat2->_position.x) + ", " + std::to_string(dat2->_position.y) + ")");
		if (dat1->_speed != dat2->_speed)
			game->logger.fatal(std::string(msgStart) + "Object::_speed: (" + std::to_string(dat1->_speed.x) + ", " + std::to_string(dat1->_speed.y) + ") vs (" + std::to_string(dat2->_speed.x) + ", " + std::to_string(dat2->_speed.y) + ")");
		if (dat1->_gravity != dat2->_gravity)
			game->logger.fatal(std::string(msgStart) + "Object::_gravity: (" + std::to_string(dat1->_gravity.x) + ", " + std::to_string(dat1->_gravity.y) + ") vs (" + std::to_string(dat2->_gravity.x) + ", " + std::to_string(dat2->_gravity.y) + ")");
		if (dat1->_action != dat2->_action)
			game->logger.fatal(std::string(msgStart) + "Object::_action: " + std::to_string(dat1->_action) + " vs " + std::to_string(dat2->_action));
		if (dat1->_hitStop != dat2->_hitStop)
			game->logger.fatal(std::string(msgStart) + "Object::_hitStop: " + std::to_string(dat1->_hitStop) + " vs " + std::to_string(dat2->_hitStop));
		if (dat1->_actionBlock != dat2->_actionBlock)
			game->logger.fatal(std::string(msgStart) + "Object::_actionBlock: " + std::to_string(dat1->_actionBlock) + " vs " + std::to_string(dat2->_actionBlock));
		if (dat1->_animation != dat2->_animation)
			game->logger.fatal(std::string(msgStart) + "Object::_animation: " + std::to_string(dat1->_animation) + " vs " + std::to_string(dat2->_animation));
		if (dat1->_animationCtr != dat2->_animationCtr)
			game->logger.fatal(std::string(msgStart) + "Object::_animationCtr: " + std::to_string(dat1->_animationCtr) + " vs " + std::to_string(dat2->_animationCtr));
		if (dat1->_hp != dat2->_hp)
			game->logger.fatal(std::string(msgStart) + "Object::_hp: " + std::to_string(dat1->_hp) + " vs " + std::to_string(dat2->_hp));
		if (dat1->_rotation != dat2->_rotation)
			game->logger.fatal(std::string(msgStart) + "Object::_rotation: " + std::to_string(dat1->_rotation) + " vs " + std::to_string(dat2->_rotation));
		if (dat1->_team != dat2->_team)
			game->logger.fatal(std::string(msgStart) + "Object::_team: " + std::to_string(dat1->_team) + " vs " + std::to_string(dat2->_team));
		if (dat1->_dead != dat2->_dead)
			game->logger.fatal(std::string(msgStart) + "Object::_dead: " + std::to_string(dat1->_dead) + " vs " + std::to_string(dat2->_dead));
		if (dat1->_hasHit != dat2->_hasHit)
			game->logger.fatal(std::string(msgStart) + "Object::_hasHit: " + std::to_string(dat1->_hasHit) + " vs " + std::to_string(dat2->_hasHit));
		if (dat1->_direction != dat2->_direction)
			game->logger.fatal(std::string(msgStart) + "Object::_direction: " + std::to_string(dat1->_direction) + " vs " + std::to_string(dat2->_direction));
		if (dat1->_cornerPriority != dat2->_cornerPriority)
			game->logger.fatal(std::string(msgStart) + "Object::_cornerPriority: " + std::to_string(dat1->_cornerPriority) + " vs " + std::to_string(dat2->_cornerPriority));
		if (dat1->_dir != dat2->_dir)
			game->logger.fatal(std::string(msgStart) + "Object::_dir: " + std::to_string(dat1->_dir) + " vs " + std::to_string(dat2->_dir));
		if (dat1->_newAnim != dat2->_newAnim)
			game->logger.fatal(std::string(msgStart) + "Object::_newAnim: " + std::to_string(dat1->_newAnim) + " vs " + std::to_string(dat2->_newAnim));
		if (dat1->_fadeTimer != dat2->_fadeTimer)
			game->logger.fatal(std::string(msgStart) + "Object::_fadeTimer: " + std::to_string(dat1->_fadeTimer) + " vs " + std::to_string(dat2->_fadeTimer));
		if (dat1->_fadeTimerMax != dat2->_fadeTimerMax)
			game->logger.fatal(std::string(msgStart) + "Object::_fadeTimerMax: " + std::to_string(dat1->_fadeTimerMax) + " vs " + std::to_string(dat2->_fadeTimerMax));
		if (dat1->_fadeDir != dat2->_fadeDir)
			game->logger.fatal(std::string(msgStart) + "Object::_fadeDir: " + std::to_string(dat1->_fadeDir) + " vs " + std::to_string(dat2->_fadeDir));
		return sizeof(Data) + length;
	}

	unsigned Object::getTeam() const
	{
		return this->_team;
	}

	int Object::getLayer() const
	{
		return 0;
	}

	bool Object::getDirection() const
	{
		return this->_direction;
	}

	void Object::_computeFrameDataCache()
	{
		this->_actionCache = this->_action;
		try {
			this->_fdCache = this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
		} catch (std::out_of_range &) {
			throw AssertionFailedExceptionMsg("this->_hasMove(this->_action)", "Invalid action: Action " + std::to_string(this->_action) + " was not found.");
		}
		this->_cacheComputed = true;
	}

	bool Object::isDisabled(const Object &) const
	{
		return false;
	}

	size_t Object::printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const
	{
		auto length = FrameData::printContent(msgStart, data, startOffset, dataSize);

		if (length == 0)
			return 0;

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + length);

		game->logger.info("Object @" + std::to_string(startOffset + length));
		if (startOffset + length + sizeof(Data) >= dataSize)
			game->logger.warn("Object is " + std::to_string(startOffset + length + sizeof(Data) - dataSize) + " bytes bigger than input");
		game->logger.info(std::string(msgStart) + "Object::_position: (" + std::to_string(dat->_position.x) + ", " + std::to_string(dat->_position.y) + ")");
		game->logger.info(std::string(msgStart) + "Object::_speed: (" + std::to_string(dat->_speed.x) + ", " + std::to_string(dat->_speed.y) + ")");
		game->logger.info(std::string(msgStart) + "Object::_gravity: (" + std::to_string(dat->_gravity.x) + ", " + std::to_string(dat->_gravity.y) + ")");
		game->logger.info(std::string(msgStart) + "Object::_action: " + std::to_string(dat->_action));
		game->logger.info(std::string(msgStart) + "Object::_hitStop: " + std::to_string(dat->_hitStop));
		game->logger.info(std::string(msgStart) + "Object::_actionBlock: " + std::to_string(dat->_actionBlock));
		game->logger.info(std::string(msgStart) + "Object::_animation: " + std::to_string(dat->_animation));
		game->logger.info(std::string(msgStart) + "Object::_animationCtr: " + std::to_string(dat->_animationCtr));
		game->logger.info(std::string(msgStart) + "Object::_hp: " + std::to_string(dat->_hp));
		game->logger.info(std::string(msgStart) + "Object::_rotation: " + std::to_string(dat->_rotation));
		game->logger.info(std::string(msgStart) + "Object::_team: " + std::to_string(dat->_team));
		game->logger.info(std::string(msgStart) + "Object::_dead: " + std::to_string(dat->_dead));
		game->logger.info(std::string(msgStart) + "Object::_hasHit: " + std::to_string(dat->_hasHit));
		game->logger.info(std::string(msgStart) + "Object::_direction: " + std::to_string(dat->_direction));
		game->logger.info(std::string(msgStart) + "Object::_cornerPriority: " + std::to_string(dat->_cornerPriority));
		game->logger.info(std::string(msgStart) + "Object::_dir: " + std::to_string(dat->_dir));
		game->logger.info(std::string(msgStart) + "Object::_newAnim: " + std::to_string(dat->_newAnim));
		game->logger.info(std::string(msgStart) + "Object::_fadeTimer: " + std::to_string(dat->_fadeTimer));
		game->logger.info(std::string(msgStart) + "Object::_fadeTimerMax: " + std::to_string(dat->_fadeTimerMax));
		game->logger.info(std::string(msgStart) + "Object::_fadeDir: " + std::to_string(dat->_fadeDir));
		if (startOffset + length + sizeof(Data) >= dataSize) {
			game->logger.fatal("Invalid input frame");
			return 0;
		}
		return sizeof(Data) + length;
	}

	const Vector2f &Object::getPosition() const
	{
		return this->_position;
	}

	unsigned Object::getDebuffDuration() const
	{
		return 60;
	}
}
