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
		auto &data = this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
		auto scale = Vector2f{
			static_cast<float>(data.size.x) / data.textureBounds.size.x,
			static_cast<float>(data.size.y) / data.textureBounds.size.y
		};
		auto result = this->_position + data.offset;

		result.y *= -1;
		result += Vector2i{
			(-static_cast<int>(data.size.x) / 2),
			-static_cast<int>(data.size.y)
		};
		this->_sprite.setPosition(result);
		this->_sprite.setScale(scale);
		this->_sprite.textureHandle = data.textureHandle;
		this->_sprite.setTextureRect(data.textureBounds);
		game.textureMgr.render(this->_sprite);

		sf::RectangleShape rect;

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

	bool EditableObject::hits(IObject &) const
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
}
