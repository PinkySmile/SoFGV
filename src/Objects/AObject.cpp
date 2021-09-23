//
// Created by PinkySmile on 18/09/2021
//

#include "AObject.hpp"
#include "../Resources/Game.hpp"

namespace Battle
{
	void AObject::render() const
	{
		auto &data = this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
		auto result =
			this->_position +
			(this->_direction ? data.offset : -data.offset) +
			Vector2i{
				this->_direction ? (-static_cast<int>(data.size.x) / 2) : (static_cast<int>(data.size.x) / 2),
				0
			};

		this->_sprite.setPosition(result);
		this->_sprite.setScale(this->_direction ? 1 : -1, 1);
		this->_sprite.textureHandle = data.textureHandle;
		this->_sprite.setTextureRect(data.textureBounds);
		game.textureMgr.render(this->_sprite);
		if (this->showBoxes) {
			sf::RectangleShape rect;

			rect.setOutlineThickness(1);
			rect.setOutlineColor(sf::Color{0x00, 0xFF, 0x00, 0xFF});
			rect.setFillColor(sf::Color{0x00, 0xFF, 0x00, 0x60});
			for (auto &hurtBox : data.hurtBoxes) {
				auto box = this->applyModifiers(hurtBox);

				rect.setPosition(box.pos + this->_position);
				rect.setSize(box.size);
				game.screen->draw(rect);
			}

			rect.setOutlineColor(sf::Color{0xFF, 0x00, 0x00, 0xFF});
			rect.setFillColor(sf::Color{0xFF, 0x00, 0x00, 0x60});
			for (auto &hitBox : data.hitBoxes) {
				auto box = this->applyModifiers(hitBox);

				rect.setPosition(box.pos + this->_position);
				rect.setSize(box.size);
				game.screen->draw(rect);
			}

			if (data.collisionBox) {
				auto box = this->applyModifiers(*data.collisionBox);

				rect.setOutlineColor(sf::Color{0xFF, 0xFF, 0x00, 0xFF});
				rect.setFillColor(sf::Color{0xFF, 0xFF, 0x00, 0x60});
				rect.setPosition(box.pos + this->_position);
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
			this->_animation %= this->_moves.at(this->_action)[this->_actionBlock].size();
			data = &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
		}
		this->_position += this->_speed;
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

	}

	void AObject::getHit(IObject &other, const FrameData *data)
	{

	}

	bool AObject::hits(IObject &other) const
	{
		auto *oData = other.getCurrentFrameData();
		auto *mData = this->getCurrentFrameData();

		if (!mData || !oData)
			return false;

		auto asAObject = dynamic_cast<AObject *>(&other);

		if (asAObject && asAObject->_team == this->_team)
			return false;

		for (auto &hurtBox : oData->hurtBoxes)
			for (auto &hitBox : mData->hurtBoxes) {
				auto _hitBox = this->applyModifiers(hitBox);
				auto _hurtBox = asAObject->applyModifiers(hurtBox);

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

	Box AObject::applyModifiers(Box box) const
	{
		if (this->_direction)
			return box;

		return Box{
			{static_cast<int>(-box.pos.x - box.size.x), box.pos.y},
			box.size
		};
	}
}
