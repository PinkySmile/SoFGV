//
// Created by PinkySmile on 18/09/2021.
//

#include <filesystem>
#include "EditableObject.hpp"

EditableObject::EditableObject(const std::string &frameData) :
	_folder(frameData.substr(0, frameData.find_last_of(std::filesystem::path::preferred_separator)))
{
	this->_moves = SpiralOfFate::FrameData::loadFile(frameData, this->_folder);
}

void EditableObject::render() const
{
	sf::RectangleShape rect;
	auto &data = this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
	auto size = SpiralOfFate::Vector2f{
		data.scale.x * data.textureBounds.size.x,
		data.scale.y * data.textureBounds.size.y
	};
	auto result = data.offset;

	result.y *= -1;
	result += SpiralOfFate::Vector2f{
		size.x / -2.f,
		-size.y
	};
	result += SpiralOfFate::Vector2f{
		data.textureBounds.size.x * data.scale.x / 2,
		data.textureBounds.size.y * data.scale.y / 2
	};
	this->_sprite.setOrigin(data.textureBounds.size / 2.f);
	this->_sprite.setRotation(data.rotation * 180 / M_PI);
	this->_sprite.setPosition(result);
	this->_sprite.setScale(data.scale);
	this->_sprite.textureHandle = data.textureHandle;
	this->_sprite.setTextureRect(data.textureBounds);
	SpiralOfFate::game->textureMgr.render(this->_sprite);

	rect.setOutlineThickness(2);
	rect.setOutlineColor(sf::Color::White);
	rect.setFillColor(sf::Color::Black);
	rect.setPosition(SpiralOfFate::Vector2f{-4, -4});
	rect.setSize({9, 9});
	SpiralOfFate::game->screen->draw(rect);
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
		SpiralOfFate::game->soundMgr.play(data->soundHandle);
	}
	this->_position += this->_speed;
	this->_speed.y += this->_gravity;
}

bool EditableObject::isDead() const
{
	return false;
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

size_t EditableObject::printDifference(const char *, void *, void *, unsigned) const
{
	return 0;
}

int EditableObject::getLayer() const
{
	return 0;
}

size_t EditableObject::printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const
{
	return 0;
}
