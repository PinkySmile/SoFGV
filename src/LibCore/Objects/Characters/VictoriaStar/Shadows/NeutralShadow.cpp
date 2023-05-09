//3
// Created by PinkySmile on 06/05/23.
//

#include "NeutralShadow.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	NeutralShadow::NeutralShadow(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		bool tint
	) :
		Shadow(frameData, hp, direction, pos, owner, id, tint ? sf::Color{0xA6, 0xA6, 0xA6} : sf::Color::White, ANIMBLOCK_NORMAL_ACTIVATED)
	{
		this->_fakeDat.setSlave();
	}

	Shadow *NeutralShadow::create(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		unsigned int id,
		bool tint
	){
		return new NeutralShadow(frameData, hp, direction, pos, owner, id, tint);
	}

	const FrameData *NeutralShadow::getCurrentFrameData() const
	{
		auto dat = Shadow::getCurrentFrameData();

		if (this->_actionBlock != ANIMBLOCK_IDLE)
			return dat;
		this->_fakeDat = *dat;
		for (auto &box : this->_fakeDat.hurtBoxes) {
			box.pos -= Vector2i{20, 20};
			box.size += Vector2i{40, 40};
		}
		return &this->_fakeDat;
	}

	unsigned int NeutralShadow::getBufferSize() const
	{
		return Shadow::getBufferSize() + sizeof(Data);
	}

	void NeutralShadow::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Shadow::getBufferSize());

		Shadow::copyToBuffer(data);
		game->logger.verbose("Saving NeutralShadow (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
		dat->_idleCounter = this->_idleCounter;
	}

	void NeutralShadow::restoreFromBuffer(void *data)
	{
		Shadow::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Shadow::getBufferSize());

		this->_idleCounter = dat->_idleCounter;
		game->logger.verbose("Restored NeutralShadow @" + std::to_string((uintptr_t)dat));
	}

	size_t NeutralShadow::printDifference(const char *msgStart, void *data1, void *data2) const
	{
		auto length = Shadow::printDifference(msgStart, data1, data2);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		if (dat1->_idleCounter != dat2->_idleCounter)
			game->logger.fatal(std::string(msgStart) + "NeutralShadow::_idleCounter: " + std::to_string(dat1->_idleCounter) + " vs " + std::to_string(dat2->_idleCounter));
		return length + sizeof(Data);
	}

	void NeutralShadow::update()
	{
		if (this->_hitStop) {
			this->_hitStop--;
			return;
		}
		Shadow::update();
		this->_idleCounter += this->_actionBlock == ANIMBLOCK_IDLE && this->_idleCounter < 120;
		if (this->_idleCounter < 120)
			return;

		auto color = this->_sprite.getColor();

		if (color.a > 5)
			color.a -= 5;
		else
			color.a = 0;
		this->_sprite.setColor(color);
	}

	void NeutralShadow::getHit(IObject &other, const FrameData *data)
	{
		auto color = this->_sprite.getColor();

		color.a = 255;
		this->_sprite.setColor(color);
		Shadow::getHit(other, data);
	}

	void NeutralShadow::activate()
	{
		auto color = this->_sprite.getColor();

		color.a = 255;
		this->_sprite.setColor(color);
		this->_idleCounter = 0;
		Shadow::activate();
	}
}