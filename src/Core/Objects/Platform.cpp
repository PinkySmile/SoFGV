//
// Created by Gegel85 on 14/02/2022.
//

#include "Platform.hpp"
#include "../Resources/Game.hpp"

namespace Battle
{
	Platform::Platform(const std::string &framedata, float width, short hp, unsigned cooldown, Vector2f pos) :
		_width(width),
		_cooldown(cooldown)
	{
		this->_team = 2;
		this->_position = pos;
		this->_moves = FrameData::loadFile(framedata);
		this->_baseHp = this->_hp = hp;
	}

	float Platform::getWidth()
	{
		return this->_width;
	}

	bool Platform::isDestructed()
	{
		return this->_hp <= 0;
	}

	void Platform::update()
	{
		Object::update();
		if (this->isDestructed()) {
			if (!this->_deathTimer) {
				this->_hp = this->_baseHp;
				this->_forceStartMove(3);
			} else
				this->_deathTimer--;
		}
	}

	void Platform::_onMoveEnd(const FrameData &lastData)
	{
		if (this->_action == 1)
			return this->_forceStartMove(2);
		if (this->_action == 3) {
			this->_hp = this->_baseHp;
			return this->_forceStartMove(0);
		}
		Object::_onMoveEnd(lastData);
	}

	void Platform::getHit(IObject &other, const FrameData *data)
	{
		Object::getHit(other, data);
		this->_hp -= data->damage;
		if (this->isDestructed()) {
			this->_hp = 0;
			this->_deathTimer = this->_cooldown;
			if (this->_action == 0)
				this->_forceStartMove(1);
		}
	}

	void Platform::render() const
	{
		Object::render();
		if (this->showBoxes) {
			if (this->_deathTimer) {
				game->screen->textSize(20);
				game->screen->fillColor(sf::Color::Red);
				game->screen->displayElement(std::to_string(this->_deathTimer), {this->_position.x - 200, -this->_position.y}, 400, Screen::ALIGN_CENTER);
				game->screen->textSize(30);
			} else
				game->screen->displayElement({
					static_cast<int>(this->_position.x - this->_width / 2),
					static_cast<int>(-this->_position.y - 1),
					static_cast<int>(this->_width),
					2
				}, sf::Color::White);
		}
	}

	Vector2f Platform::getPosition()
	{
		return this->_position;
	}

	void Platform::touch(IObject &)
	{
	}

	void Platform::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		Object::copyToBuffer(data);
#ifdef _DEBUG
		game->logger.debug("Saving Platform (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
#endif
		dat->_width = this->_width;
		dat->_cooldown = this->_cooldown;
		dat->_deathTimer = this->_deathTimer;
	}

	void Platform::restoreFromBuffer(void *data)
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		Object::restoreFromBuffer(data);
		this->_width = dat->_width;
		this->_cooldown = dat->_cooldown;
		this->_deathTimer = dat->_deathTimer;
	}

	unsigned int Platform::getBufferSize() const
	{
		return Object::getBufferSize() + sizeof(Data);
	}
}