//
// Created by PinkySmile on 18/09/2021
//

#include "AObject.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"

namespace Battle
{
	void AObject::render() const
	{
		auto &data = this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
		auto scale = Vector2f{
			static_cast<float>(data.size.x) / data.textureBounds.size.x,
			static_cast<float>(data.size.y) / data.textureBounds.size.y
		};
		auto result = this->_position + (this->_direction ? data.offset : -data.offset);
		auto realPos = this->_position;

		realPos.y *= -1;
		result.y *= -1;
		result.y += 128;
		result += Vector2i{
			this->_direction ? (-static_cast<int>(data.size.x) / 2) : (static_cast<int>(data.size.x) / 2),
			-static_cast<int>(data.size.y)
		};
		this->_sprite.setPosition(result);
		this->_sprite.setScale(this->_direction ? scale.x : -scale.x, scale.y);
		this->_sprite.textureHandle = data.textureHandle;
		this->_sprite.setTextureRect(data.textureBounds);
		game.textureMgr.render(this->_sprite);
		if (this->showBoxes) {
			sf::RectangleShape rect;

			rect.setOutlineThickness(1);
			rect.setOutlineColor(sf::Color{0x00, 0xFF, 0x00, 0xFF});
			rect.setFillColor(sf::Color{0x00, 0xFF, 0x00, 0x60});
			for (auto &hurtBox : data.hurtBoxes) {
				auto box = this->_applyModifiers(hurtBox);

				rect.setPosition(box.pos + realPos);
				rect.setSize(box.size);
				game.screen->draw(rect);
			}

			rect.setOutlineColor(sf::Color{0xFF, 0x00, 0x00, 0xFF});
			rect.setFillColor(sf::Color{0xFF, 0x00, 0x00, 0x60});
			for (auto &hitBox : data.hitBoxes) {
				auto box = this->_applyModifiers(hitBox);

				rect.setPosition(box.pos + realPos);
				rect.setSize(box.size);
				game.screen->draw(rect);
			}

			if (data.collisionBox) {
				auto box = this->_applyModifiers(*data.collisionBox);

				rect.setOutlineColor(sf::Color{0xFF, 0xFF, 0x00, 0xFF});
				rect.setFillColor(sf::Color{0xFF, 0xFF, 0x00, 0x60});
				rect.setPosition(box.pos + realPos);
				rect.setSize(box.size);
				game.screen->draw(rect);
			}
		}
	}

	void AObject::update()
	{
		auto *data = &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];

		this->_animationCtr++;
		while (this->_animationCtr >= data->duration) {
			this->_animationCtr = 0;
			this->_animation++;
			this->_hasHit &= this->_animation < this->_moves.at(this->_action)[this->_actionBlock].size();
			this->_animation %= this->_moves.at(this->_action)[this->_actionBlock].size();
			data = &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
			this->_applyNewAnimFlags();
		}
		this->_position += this->_speed;
		if (data->dFlag.airborne)
			this->_speed += this->_gravity;
	}

	void AObject::reset()
	{
		this->_rotation = this->_baseRotation;
		this->_gravity = this->_baseGravity;
		this->_hp = this->_baseHp;
	}

	bool AObject::isDead() const
	{
		return this->_dead;
	}

	void AObject::hit(IObject &other, const FrameData *data)
	{
		logger.debug(std::to_string((ptrdiff_t)this) + " has hit " + std::to_string((long long)&other) + " !");
		this->_hasHit = true;
	}

	void AObject::getHit(IObject &other, const FrameData *data)
	{
		logger.debug(std::to_string((ptrdiff_t)this) + " is hit by " + std::to_string((long long)&other) + " !");
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

		for (auto &hurtBox : oData->hurtBoxes)
			for (auto &hitBox : mData->hitBoxes) {
				auto _hitBox = this->_applyModifiers(hitBox);
				auto _hurtBox = asAObject->_applyModifiers(hurtBox);

				if (
					_hurtBox.pos.x < _hitBox.pos.x + static_cast<int>(_hitBox.size.x) &&
					_hurtBox.pos.y < _hitBox.pos.y + static_cast<int>(_hitBox.size.y) &&
					_hurtBox.pos.x + static_cast<int>(_hurtBox.size.x) > _hitBox.pos.x &&
					_hurtBox.pos.y + static_cast<int>(_hurtBox.size.y) > _hitBox.pos.y
				)
					return true;
			}
		return false;
	}

	const FrameData *AObject::getCurrentFrameData() const
	{
		return &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
	}

	Box AObject::_applyModifiers(Box box) const
	{
		if (this->_direction)
			return box;

		return Box{
			{static_cast<int>(-box.pos.x - box.size.x), box.pos.y},
			box.size
		};
	}

	void AObject::_applyNewAnimFlags()
	{
		auto data = this->getCurrentFrameData();

		if (!data)
			return;
		this->_hasHit &= data->oFlag.resetHits;
		if (data->oFlag.resetSpeed)
			this->_speed = {0, 0};
		if (data->dFlag.resetRotation)
			this->_rotation = 0;
	}
}
