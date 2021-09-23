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

		this->_sprite.setPosition(this->_position + data.offset - Vector2u{data.size.x / 2, 0});
		this->_sprite.setScale(1, 1);
		this->_sprite.textureHandle = data.textureHandle;
		this->_sprite.setTextureRect(data.textureBounds);
		game.textureMgr.render(this->_sprite);
		if (this->showBoxes) {
			sf::RectangleShape rect;

			rect.setOutlineThickness(1);
			rect.setOutlineColor(sf::Color{0x00, 0xFF, 0x00, 0xFF});
			rect.setFillColor(sf::Color{0x00, 0xFF, 0x00, 0x60});
			for (auto &hurtBox : data.hurtBoxes) {
				rect.setPosition(hurtBox.pos + this->_position);
				rect.setSize(hurtBox.size);
				game.screen->draw(rect);
			}

			rect.setOutlineColor(sf::Color{0xFF, 0x00, 0x00, 0xFF});
			rect.setFillColor(sf::Color{0xFF, 0x00, 0x00, 0x60});
			for (auto &hitBox : data.hitBoxes) {
				rect.setPosition(hitBox.pos + this->_position);
				rect.setSize(hitBox.size);
				game.screen->draw(rect);
			}

			if (data.collisionBox) {
				rect.setOutlineColor(sf::Color{0xFF, 0xFF, 0x00, 0xFF});
				rect.setFillColor(sf::Color{0xFF, 0xFF, 0x00, 0x60});
				rect.setPosition(data.collisionBox->pos + this->_position);
				rect.setSize(data.collisionBox->size);
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

	void AObject::hit(IObject *other)
	{

	}

	void AObject::hasHit()
	{

	}

	IObject *AObject::hits(IObject *other) const
	{
		auto *oData = other->getCurrentFrameData();
		auto *mData = this->getCurrentFrameData();

		if (!mData || !oData)
			return nullptr;
		for (auto &hurtBox : oData->hurtBoxes) {
			for (auto &hitBox : oData->hurtBoxes) {
				if (
					hurtBox.pos.x < hitBox.pos.x + static_cast<int>(hitBox.size.x) &&
					hurtBox.pos.y < hitBox.pos.y + static_cast<int>(hitBox.size.y) &&
					hurtBox.pos.x + static_cast<int>(hurtBox.size.x) > hitBox.pos.x &&
					hurtBox.pos.y + static_cast<int>(hurtBox.size.y) > hitBox.pos.y
				) {

				}
			}
		}
		return nullptr;
	}

	const FrameData *AObject::getCurrentFrameData() const
	{
		return &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
	}
}
