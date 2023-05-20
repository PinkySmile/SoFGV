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
	)
	{
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

	void NeutralShadow::update()
	{
		if (this->_hitStop) {
			this->_hitStop--;
			return;
		}
		Shadow::update();
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