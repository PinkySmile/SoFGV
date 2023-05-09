//
// Created by PinkySmile on 06/05/23.
//

#include "SpiritShadow.hpp"
#include "Resources/Game.hpp"

#define TRIGGER_RADIUS 150

namespace SpiralOfFate
{
	SpiritShadow::SpiritShadow(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		bool tint
	) :
		Shadow(frameData, hp, direction, pos, owner, id, tint ? sf::Color{0x33, 0xCC, 0xCC} : sf::Color::White, ANIMBLOCK_SPIRIT_ACTIVATED)
	{
	}

	Shadow *SpiritShadow::create(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		bool tint
	){
		return new SpiritShadow(frameData, hp, direction, pos, owner, id, tint);
	}

	unsigned int SpiritShadow::getBufferSize() const
	{
		return Shadow::getBufferSize() + sizeof(Data);
	}

	void SpiritShadow::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Shadow::getBufferSize());

		Shadow::copyToBuffer(data);
		game->logger.verbose("Saving SpiritShadow (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
		dat->_idleCounter = this->_idleCounter;
	}

	void SpiritShadow::restoreFromBuffer(void *data)
	{
		Shadow::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Shadow::getBufferSize());

		this->_idleCounter = dat->_idleCounter;
		game->logger.verbose("Restored SpiritShadow @" + std::to_string((uintptr_t)dat));
	}

	size_t SpiritShadow::printDifference(const char *msgStart, void *data1, void *data2) const
	{
		auto length = Shadow::printDifference(msgStart, data1, data2);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		if (dat1->_idleCounter != dat2->_idleCounter)
			game->logger.fatal(std::string(msgStart) + "SpiritShadow::_idleCounter: " + std::to_string(dat1->_idleCounter) + " vs " + std::to_string(dat2->_idleCounter));
		return length + sizeof(Data);
	}

	void SpiritShadow::update()
	{
		if (this->_hitStop) {
			this->_hitStop--;
			return;
		}
		Shadow::update();
		this->_idleCounter += this->_actionBlock == ANIMBLOCK_IDLE && this->_idleCounter < 120;
		if (this->_idleCounter < 120 || this->_actionBlock != ANIMBLOCK_IDLE)
			return;

		auto myPos = this->_position;
		auto &dat = *this->getCurrentFrameData();
		std::vector<Object *> objects;
		auto &subObjs = game->battleMgr->getObjects();

		objects.reserve(subObjs.size() + 2);
		objects.push_back(&*game->battleMgr->getLeftCharacter());
		objects.push_back(&*game->battleMgr->getRightCharacter());
		for (auto &object : subObjs)
			objects.push_back(reinterpret_cast<Object *>(&*object.second));
		myPos.y += (dat.size.y + dat.offset.y) / 2.f;
		for (auto &obj : objects) {
			auto &data = *obj->getCurrentFrameData();

			if (data.hurtBoxes.empty() || this->getTeam() == obj->getTeam())
				continue;

			auto opPos = reinterpret_cast<SpiritShadow *>(obj)->_position;

			opPos.y += (data.size.y + data.offset.y) / 2.f;
			if (myPos.distance2(opPos) <= TRIGGER_RADIUS * TRIGGER_RADIUS)
				return this->activate();
		}
	}

	void SpiritShadow::render() const
	{
		Object::render();
		if (!this->showBoxes)
			return;
		if (this->_actionBlock != ANIMBLOCK_IDLE)
			return;

		sf::CircleShape circle;
		auto myPos = this->_position;

		myPos.y += this->getCurrentFrameData()->size.y / 2.f;
		myPos.y *= -1;
		if (this->_idleCounter < 120)
			circle.setOutlineColor(sf::Color::Red);
		else
			circle.setOutlineColor(sf::Color::Green);
		circle.setFillColor(sf::Color::Transparent);
		circle.setRadius(TRIGGER_RADIUS);
		circle.setOrigin(TRIGGER_RADIUS, TRIGGER_RADIUS);
		circle.setPosition(myPos);
		circle.setOutlineThickness(2);
		game->screen->draw(circle);
	}
}