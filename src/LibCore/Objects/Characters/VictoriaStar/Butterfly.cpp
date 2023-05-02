//
// Created by PinkySmile on 30/04/23.
//

#include "Butterfly.hpp"
#include "Objects/Character.hpp"
#include "Resources/Game.hpp"
#include "VictoriaStar.hpp"

#define MAX_ALPHA 225
#define ALPHA_STEP 20
#define MAX_CTR 10

namespace SpiralOfFate
{
	Butterfly::Butterfly(VictoriaStar *owner, Character *opponent, bool isLeft, Butterfly *copy, const std::vector<std::vector<FrameData>> &frameData, unsigned id) :
		SubObject(id, isLeft),
		_owner(owner),
		_opponent(opponent),
		_copy(copy)
	{
		this->_moves[0] = frameData;
		this->_fakeFrameData.setSlave();
		this->_position = this->_owner->_position;
		this->_position.y += 160;
	}

	const FrameData *Butterfly::getCurrentFrameData() const
	{
		auto angle = this->getAngle();
		auto scale = 1.5 - std::sin(angle) * 0.5;

		this->_fakeFrameData = *Object::getCurrentFrameData();
		this->_fakeFrameData.size = this->_fakeFrameData.textureBounds.size * scale;
		return &this->_fakeFrameData;
	}

	int Butterfly::getLayer() const
	{
		return (this->getOwner() ^ !game->battleMgr->isLeftFirst()) * 100 - 50 - std::sin(this->getAngle()) * NB_BUTTERFLIES / 2;
	}

	void Butterfly::update()
	{
		if (this->_copy) {
			this->_position = this->_copy->_position;
			this->_animation = this->_copy->_animation;
			this->_actionBlock = this->_copy->_actionBlock;
			this->_direction = this->_copy->_direction;
			this->_dir = this->_copy->_dir;
			this->_rotation = this->_copy->_rotation;
			this->_sprite.setColor(sf::Color{
				255, 255, 255,
				static_cast<sf::Uint8>(this->_copy->_maxAlpha * (1 - this->_copy->_alpha))
			});
			return;
		}

		auto distance = this->_owner->_position.distance2(reinterpret_cast<VictoriaStar *>(this->_opponent)->_position);

		this->_counter++;
		if (distance <= 500 * 500)
			this->_counter += (500 * 500 - distance) * 4 / (500 * 500);
		if (this->_counter > 360)
			this->_counter -= 360;

		auto angle = this->getAngle();
		int nb = (1000 * 1000 - 50 * 50) / (distance - 50 * 50);
		auto id = (this->getId() - BUTTERFLIES_START_ID) % NB_BUTTERFLIES;
		int div = NB_BUTTERFLIES / (nb + !nb);
		auto wasDisabled = this->_disabled;

		this->_disabled = div > 0 && id % div != 0;
		if (distance >= 500 * 500) {
			this->_actionBlock = (M_PI_4 <= angle && angle < 3 * M_PI_4) || (5 * M_PI_4 <= angle && angle < 7 * M_PI_4);
			this->_rotation = (3 * M_PI_4 <= angle && angle < 5 * M_PI_4) * M_PI;
			this->_dir = std::copysign(1, -std::sin(angle));
			this->_target = this->_owner->_position + Vector2f{
				std::cos(angle) * 40,
				std::sin(angle) * 30 + 160,
			};
			if (random_distrib(game->battleRandom, 0, 16) != 0)
				Object::update();
			if (random_distrib(game->battleRandom, 0, 16) == 0)
				Object::update();
			this->_alpha = 1.f;
			if (wasDisabled)
				this->_position = this->_target;
			else
				this->_position += (this->_target - this->_position) / 4;
			this->_ctr = MAX_CTR;
		} else if (distance >= 300 * 300) {
			auto diff = distance - 300 * 300;
			float xRand = random_distrib(game->battleRandom, 0, 500 * 500 - diff) * 20 / (500 * 500 - 300 * 300);
			float yRand = random_distrib(game->battleRandom, 0, 500 * 500 - diff) * 20 / (500 * 500 - 300 * 300);

			this->_target = this->_owner->_position + Vector2f{
				std::cos(angle) * 40 + xRand - 10,
				std::sin(angle) * 30 + 160 - yRand + 10,
			};
			this->_actionBlock = (M_PI_4 <= angle && angle < 3 * M_PI_4) || (5 * M_PI_4 <= angle && angle < 7 * M_PI_4);
			this->_rotation = (3 * M_PI_4 <= angle && angle < 5 * M_PI_4) * M_PI;
			this->_dir = std::copysign(1, -std::sin(angle));
			Object::update();
			if (random_distrib(game->battleRandom, 0, 10 - 200 * 200 * 10) / diff == 0)
				Object::update();
			if (distance >= 400 * 400)
				this->_alpha = 1.f;
			else
				this->_alpha = diff / (400 * 400 - 300 * 300);
			if (wasDisabled)
				this->_position = this->_target;
			else
				this->_position += (this->_target - this->_position) / 4;
			this->_ctr = MAX_CTR;
		} else {
			auto diff = this->_target - this->_position;

			this->_alpha = 0;
			if (std::abs(diff.x) > std::abs(diff.y)) {
				this->_actionBlock = 1;
				this->_rotation = 0;
				this->_dir = std::copysign(1, -diff.x);
			} else {
				this->_actionBlock = 0;
				this->_rotation = (diff.y < 0) * M_PI;
				this->_dir = 1;
			}
			if (random_distrib(game->battleRandom, 0, 1) == 0)
				this->_ctr++;
			if (this->_ctr >= MAX_CTR) {
				float xRand = random_distrib(game->battleRandom, 0, 100);
				float yRand = random_distrib(game->battleRandom, 0, 120);

				this->_ctr = 1;
				this->_base = this->_position;
				this->_target = this->_owner->_position + Vector2f{xRand - 50, yRand + 60};
			}
			this->_position = this->_base + (this->_target - this->_base) * this->_ctr / (float)MAX_CTR;
		}

		auto maxAlpha = (1 - this->_disabled) * MAX_ALPHA;

		if (std::abs(this->_maxAlpha - maxAlpha) > ALPHA_STEP)
			this->_maxAlpha += std::copysign(ALPHA_STEP, maxAlpha - this->_maxAlpha);
		else
			this->_maxAlpha = maxAlpha;
		this->_sprite.setColor(sf::Color{
			255, 255, 255,
			static_cast<sf::Uint8>(this->_maxAlpha * this->_alpha)
		});
		this->_direction = this->_dir == 1;
	}

	float Butterfly::getAngle() const
	{
		float id = (this->getId() - BUTTERFLIES_START_ID) % NB_BUTTERFLIES;
		auto ctr = (this->_copy ? this->_copy : this)->_counter;

		return fmod(ctr + id / NB_BUTTERFLIES * 360, 360) * M_PI / 180;
	}

	unsigned int Butterfly::getBufferSize() const
	{
		return Object::getBufferSize() + (this->_copy ? sizeof(WeirdData) : sizeof(HappyData));
	}

	void Butterfly::copyToBuffer(void *data) const
	{
		Object::copyToBuffer(data);
		if (this->_copy) {
			auto dat = reinterpret_cast<WeirdData *>((uintptr_t)data + Object::getBufferSize());

			return;
		}

		auto dat = reinterpret_cast<HappyData *>((uintptr_t)data + Object::getBufferSize());

		dat->_counter = this->_counter;
		dat->_target = this->_target;
		dat->_maxAlpha = this->_maxAlpha;
		dat->_base = this->_base;
		dat->_ctr = this->_ctr;
	}

	void Butterfly::restoreFromBuffer(void *data)
	{
		Object::restoreFromBuffer(data);
		if (this->_copy) {
			auto dat = reinterpret_cast<WeirdData *>((uintptr_t)data + Object::getBufferSize());

			return;
		}

		auto dat = reinterpret_cast<HappyData *>((uintptr_t)data + Object::getBufferSize());

		this->_maxAlpha = dat->_maxAlpha;
		this->_counter = dat->_counter;
		this->_target = dat->_target;
		this->_base = dat->_base;
		this->_ctr = dat->_ctr;
	}

	size_t Butterfly::printDifference(const char *msgStart, void *data1, void *data2) const
	{
		auto length = Object::printDifference(msgStart, data1, data2);

		if (length == 0)
			return 0;
		if (this->_copy) {
			auto dat1 = reinterpret_cast<WeirdData *>((uintptr_t)data1 + length);
			auto dat2 = reinterpret_cast<WeirdData *>((uintptr_t)data2 + length);

			return length + sizeof(WeirdData);
		}

		auto dat1 = reinterpret_cast<HappyData *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<HappyData *>((uintptr_t)data2 + length);

		if (dat1->_counter != dat2->_counter)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_counter: " + std::to_string(dat1->_counter) + " vs " + std::to_string(dat2->_counter));
		if (dat1->_target.x != dat2->_target.x)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_target.x: " + std::to_string(dat1->_target.x) + " vs " + std::to_string(dat2->_target.x));
		if (dat1->_target.y != dat2->_target.y)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_target.y: " + std::to_string(dat1->_target.y) + " vs " + std::to_string(dat2->_target.y));
		if (dat1->_base.x != dat2->_base.x)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_base.x: " + std::to_string(dat1->_base.x) + " vs " + std::to_string(dat2->_base.x));
		if (dat1->_base.y != dat2->_base.y)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_base.y: " + std::to_string(dat1->_base.y) + " vs " + std::to_string(dat2->_base.y));
		if (dat1->_ctr != dat2->_ctr)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_ctr: " + std::to_string(dat1->_ctr) + " vs " + std::to_string(dat2->_ctr));
		if (dat1->_maxAlpha != dat2->_maxAlpha)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_maxAlpha: " + std::to_string(dat1->_maxAlpha) + " vs " + std::to_string(dat2->_maxAlpha));
		return length + sizeof(HappyData);
	}
}