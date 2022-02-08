//
// Created by PinkySmile on 18/09/2021
//

#include "EditableObject.hpp"
#include "../Resources/Game.hpp"

namespace Battle
{
	EditableObject::EditableObject(const std::string &frameData)
	{
		this->_moves = FrameData::loadFile(frameData);
	}

	void EditableObject::render() const
	{
		sf::RectangleShape rect;
		auto &data = this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
		auto scale = Vector2f{
			static_cast<float>(data.size.x) / data.textureBounds.size.x,
			static_cast<float>(data.size.y) / data.textureBounds.size.y
		};
		auto result = data.offset;

		result.y *= -1;
		result += Vector2f{
			data.size.x / -2.f,
			-static_cast<float>(data.size.y)
		};
		result += Vector2f{
			data.textureBounds.size.x * scale.x / 2,
			data.textureBounds.size.y * scale.y / 2
		};
		this->_sprite.setOrigin(data.textureBounds.size / 2.f);
		this->_sprite.setRotation(data.rotation * 180 / M_PI);
		this->_sprite.setPosition(result);
		this->_sprite.setScale(scale);
		this->_sprite.textureHandle = data.textureHandle;
		this->_sprite.setTextureRect(data.textureBounds);
		game.textureMgr.render(this->_sprite);

		rect.setOutlineThickness(2);
		rect.setOutlineColor(sf::Color::White);
		rect.setFillColor(sf::Color::Black);
		rect.setPosition(this->_position - Vector2f{4, 4});
		rect.setSize({9, 9});
		game.screen->draw(rect);
	}

	void EditableObject::update()
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
		this->_speed.y += this->_gravity;
	}

	void EditableObject::reset()
	{
	}

	bool EditableObject::isDead() const
	{
		return false;
	}

	bool EditableObject::hits(const IObject &) const
	{
		return false;
	}

	void EditableObject::hit(IObject &, const FrameData *)
	{
	}

	void EditableObject::getHit(IObject &, const FrameData *)
	{
	}

	const FrameData *EditableObject::getCurrentFrameData() const
	{
		return nullptr;
	}

	bool EditableObject::collides(const IObject &) const
	{
		return false;
	}

	void EditableObject::collide(IObject &)
	{
	}

	void EditableObject::kill()
	{

	}

	unsigned int EditableObject::getBufferSize() const
	{
		return 0;
	}

	void EditableObject::copyToBuffer(void *) const
	{

	}

	void EditableObject::restoreFromBuffer(void *)
	{

	}

	unsigned int EditableObject::getClassId() const
	{
		return 0;
	}
}
