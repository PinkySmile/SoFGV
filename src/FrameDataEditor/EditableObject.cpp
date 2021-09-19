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

		this->_sprite.setPosition(this->_position + data.offset - Vector2u{data.size.x / 2, 0});
		this->_sprite.setScale(1, 1);
		this->_sprite.textureHandle = data.textureHandle;
		this->_sprite.setTextureRect(data.textureBounds);
		game.textureMgr.render(this->_sprite);
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
}
