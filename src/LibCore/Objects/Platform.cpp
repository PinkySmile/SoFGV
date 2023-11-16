//
// Created by PinkySmile on 14/02/2022.
//

#include "Platform.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	Platform::Platform(const std::string &framedata, float width, short hp, unsigned cooldown, Vector2f pos) :
		_width(width),
		_cooldown(cooldown)
	{
		this->_team = 2;
		this->_position = pos;
		//TODO
		this->_moves = FrameData::loadFile(framedata, framedata.substr(0, framedata.find_last_of('/')));
		this->_baseHp = this->_hp = hp;
	}

	float Platform::getWidth()
	{
		return this->_width;
	}

	bool Platform::isDestructed()
	{
		return this->_hp == 0;
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

	void Platform::getHit(Object &other, const FrameData *data)
	{
		Object::getHit(other, data);
		if (this->_hp < data->damage)
			this->_hp = 0;
		else
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

	void Platform::touch(Object &)
	{
	}

	void Platform::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		Object::copyToBuffer(data);
		game->logger.verbose("Saving Platform (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
		dat->_width = this->_width;
		dat->_cooldown = this->_cooldown;
		dat->_deathTimer = this->_deathTimer;
	}

	void Platform::restoreFromBuffer(void *data)
	{
		Object::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		this->_width = dat->_width;
		this->_cooldown = dat->_cooldown;
		this->_deathTimer = dat->_deathTimer;
		game->logger.verbose("Restored Platform @" + std::to_string((uintptr_t)dat));
	}

	unsigned int Platform::getBufferSize() const
	{
		return Object::getBufferSize() + sizeof(Data);
	}

	size_t Platform::printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const
	{
		auto length = Object::printDifference(msgStart, data1, data2, startOffset);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		game->logger.info("Platform @" + std::to_string(startOffset + length));
		if (dat1->_width != dat2->_width)
			game->logger.fatal(std::string(msgStart) + "Platform::_width: " + std::to_string(dat1->_width) + " vs " + std::to_string(dat2->_width));
		if (dat1->_cooldown != dat2->_cooldown)
			game->logger.fatal(std::string(msgStart) + "Platform::_cooldown: " + std::to_string(dat1->_cooldown) + " vs " + std::to_string(dat2->_cooldown));
		if (dat1->_deathTimer != dat2->_deathTimer)
			game->logger.fatal(std::string(msgStart) + "Platform::_deathTimer: " + std::to_string(dat1->_deathTimer) + " vs " + std::to_string(dat2->_deathTimer));
		return length + sizeof(Data);
	}

	int Platform::getLayer() const
	{
		return -51;
	}

	size_t Platform::printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const
	{
		auto length = Object::printContent(msgStart, data,  startOffset, dataSize);

		if (length == 0)
			return 0;

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + length);

		game->logger.info("Platform @" + std::to_string(startOffset + length));
		if (startOffset + length + sizeof(Data) >= dataSize)
			game->logger.warn("Object is " + std::to_string(startOffset + length + sizeof(Data) - dataSize) + " bytes bigger than input");
		game->logger.info(std::string(msgStart) + "Platform::_width: " + std::to_string(dat->_width));
		game->logger.info(std::string(msgStart) + "Platform::_cooldown: " + std::to_string(dat->_cooldown));
		game->logger.info(std::string(msgStart) + "Platform::_deathTimer: " + std::to_string(dat->_deathTimer));
		if (startOffset + length + sizeof(Data) >= dataSize) {
			game->logger.fatal("Invalid input frame");
			return 0;
		}
		return length + sizeof(Data);
	}
}