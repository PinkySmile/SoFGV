//
// Created by PinkySmile on 30/04/23.
//

#include "Butterfly.hpp"
#include "Shadow.hpp"
#include "Objects/Character.hpp"
#include "Resources/Game.hpp"
#include "VictoriaStar.hpp"

#define MAX_ALPHA 175
#define ALPHA_STEP 20
#define MAX_CTR 10

namespace SpiralOfFate
{
	Butterfly::Butterfly(VictoriaStar *owner, Character *opponent, bool isLeft, Butterfly *copy, const std::vector<std::vector<FrameData>> &frameData, unsigned id) :
		SubObject(id, isLeft, owner),
		_owner(owner),
		_opponent(opponent),
		_copy(copy)
	{
		this->_team = isLeft;
		this->_moves[0] = frameData;
		this->_position = this->_owner->_position;
		this->_position.y += 160;
	}

	int Butterfly::getLayer() const
	{
		if (this->_defenseCtr)
			return 50;
		return (this->getOwner() == game->battleMgr->isLeftFirst()) * 100 - 50 - std::sin(this->getAngle()) * NB_BUTTERFLIES / 2;
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
		if (this->_defenseCtr) {
			this->_defenseCtr--;
			return;
		}
		if (this->_attackId) {
			this->_updateCurrentAttack();
			if (this->_actionBlock >= 2) {
				this->_sprite.setColor(sf::Color{
					255, 255, 255,
					static_cast<sf::Uint8>(this->_maxAlpha * this->_alpha)
				});
				Object::update();
				return;
			} else if (!this->_attackFadeCtr) {
				this->_sprite.setColor(sf::Color{
					255, 255, 255,
					static_cast<sf::Uint8>(this->_maxAlpha * this->_alpha)
				});
				return;
			}
		}

		auto distance = this->_owner->_position.distance2(this->_opponent->getPosition());

		this->_counter++;
		if (distance <= 500 * 500)
			this->_counter += (500 * 500 - distance) * 4 / (500 * 500);
		if (this->_counter > 360)
			this->_counter -= 360;

		auto angle = this->getAngle();
		int nb = (1000 * 1000 - 50 * 50) / (distance - 50 * 50);
		auto id = (this->getId() - BUTTERFLIES_START_ID) % NB_BUTTERFLIES;
		int div = NB_BUTTERFLIES / (nb + !nb);

		if (VictoriaStar::isHitAction(this->_owner->_action)) {
			this->_disabled = false;
			goto notHappy;
		}
		this->_disabled = div > 0 && id % div != 0;
		if (this->_owner->_action >= ACTION_5N) {
			this->_disabled = true;
			if (!this->_attackId)
				this->_maxAlpha = 0;
		}
		if (distance >= 500 * 500) {
			this->_actionBlock = (M_PI_4 <= angle && angle < 3 * M_PI_4) || (5 * M_PI_4 <= angle && angle < 7 * M_PI_4);
			this->_rotation = (3 * M_PI_4 <= angle && angle < 5 * M_PI_4) * M_PI;
			this->_dir = std::copysign(1, -std::sin(angle));
			this->_target = this->_owner->_position + Vector2f{
				std::cos(angle) * 40,
				std::sin(angle) * 30 + 160,
			};
			if (random_distrib(game->battleRandom, 0, 16) != 0)
				this->_tickMove();
			if (random_distrib(game->battleRandom, 0, 16) == 0)
				this->_tickMove();
			this->_alpha = 1.f;
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
			this->_tickMove();
			if (random_distrib(game->battleRandom, 0, 10 - 200 * 200 * 10) / diff == 0)
				this->_tickMove();
			if (distance >= 400 * 400)
				this->_alpha = 1.f;
			else
				this->_alpha = diff / (400 * 400 - 300 * 300);
			this->_position += (this->_target - this->_position) / 4;
			this->_ctr = MAX_CTR;
		} else {
		notHappy:
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
			if (random_distrib(game->battleRandom, 0, 16) != 0)
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

		if (!this->_attackId) {
			auto maxAlpha = (1 - this->_disabled) * MAX_ALPHA;

			if (std::abs(this->_maxAlpha - maxAlpha) > ALPHA_STEP)
				this->_maxAlpha += std::copysign(ALPHA_STEP, maxAlpha - this->_maxAlpha);
			else
				this->_maxAlpha = maxAlpha;
		}
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
		return Object::getBufferSize() + (this->_copy ? 0 : sizeof(HappyData));
	}

	void Butterfly::copyToBuffer(void *data) const
	{
		Object::copyToBuffer(data);
		if (this->_copy)
			return;

		auto dat = reinterpret_cast<HappyData *>((uintptr_t)data + Object::getBufferSize());

		dat->_attackFadeTime = this->_attackFadeTime;
		dat->_attackAppearCtr = this->_attackAppearCtr;
		dat->_attackPos = this->_attackPos;
		dat->_attackPosStep = this->_attackPosStep;
		dat->_attackId = this->_attackId;
		dat->_attackFadeCtr = this->_attackFadeCtr;
		dat->_attackAttackCtr = this->_attackAttackCtr;
		dat->_attackTravelCtr = this->_attackTravelCtr;
		dat->_defenseCtr = this->_defenseCtr;
		dat->_maxAlpha = this->_maxAlpha;
		dat->_counter = this->_counter;
		dat->_target = this->_target;
		dat->_base = this->_base;
		dat->_ctr = this->_ctr;
	}

	void Butterfly::restoreFromBuffer(void *data)
	{
		Object::restoreFromBuffer(data);
		if (this->_copy)
			return;

		auto dat = reinterpret_cast<HappyData *>((uintptr_t)data + Object::getBufferSize());

		this->_attackFadeTime = dat->_attackFadeTime;
		this->_attackAppearCtr = dat->_attackAppearCtr;
		this->_attackPos = dat->_attackPos;
		this->_attackPosStep = dat->_attackPosStep;
		this->_attackId = dat->_attackId;
		this->_attackFadeCtr = dat->_attackFadeCtr;
		this->_attackAttackCtr = dat->_attackAttackCtr;
		this->_attackTravelCtr = dat->_attackTravelCtr;
		this->_defenseCtr = dat->_defenseCtr;
		this->_maxAlpha = dat->_maxAlpha;
		this->_counter = dat->_counter;
		this->_target = dat->_target;
		this->_base = dat->_base;
		this->_ctr = dat->_ctr;
	}

	size_t Butterfly::printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const
	{
		auto length = Object::printDifference(msgStart, data1, data2, startOffset);

		if (length == 0)
			return 0;
		game->logger.info("Butterfly @" + std::to_string(startOffset + length));
		if (this->_copy)
			return length;

		auto dat1 = reinterpret_cast<HappyData *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<HappyData *>((uintptr_t)data2 + length);

		if (dat1->_attackPos.x != dat2->_attackPos.x)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_attackPos.x: " + std::to_string(dat1->_attackPos.x) + " vs " + std::to_string(dat2->_attackPos.x));
		if (dat1->_attackPos.y != dat2->_attackPos.y)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_attackPos.y: " + std::to_string(dat1->_attackPos.y) + " vs " + std::to_string(dat2->_attackPos.y));
		if (dat1->_attackPosStep.x != dat2->_attackPosStep.x)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_attackPosStep.x: " + std::to_string(dat1->_attackPosStep.x) + " vs " + std::to_string(dat2->_attackPosStep.x));
		if (dat1->_attackPosStep.y != dat2->_attackPosStep.y)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_attackPosStep.y: " + std::to_string(dat1->_attackPosStep.y) + " vs " + std::to_string(dat2->_attackPosStep.y));
		if (dat1->_attackAppearCtr != dat2->_attackAppearCtr)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_attackAppearCtr: " + std::to_string(dat1->_attackAppearCtr) + " vs " + std::to_string(dat2->_attackAppearCtr));
		if (dat1->_attackId != dat2->_attackId)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_attackId: " + std::to_string(dat1->_attackId) + " vs " + std::to_string(dat2->_attackId));
		if (dat1->_attackFadeCtr != dat2->_attackFadeCtr)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_attackFadeCtr: " + std::to_string(dat1->_attackFadeCtr) + " vs " + std::to_string(dat2->_attackFadeCtr));
		if (dat1->_attackAttackCtr != dat2->_attackAttackCtr)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_attackAttackCtr: " + std::to_string(dat1->_attackAttackCtr) + " vs " + std::to_string(dat2->_attackAttackCtr));
		if (dat1->_attackTravelCtr != dat2->_attackTravelCtr)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_attackTravelCtr: " + std::to_string(dat1->_attackTravelCtr) + " vs " + std::to_string(dat2->_attackTravelCtr));
		if (dat1->_counter != dat2->_counter)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_counter: " + std::to_string(dat1->_counter) + " vs " + std::to_string(dat2->_counter));
		if (dat1->_defenseCtr != dat2->_defenseCtr)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_defenseCtr: " + std::to_string(dat1->_defenseCtr) + " vs " + std::to_string(dat2->_defenseCtr));
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
		if (dat1->_attackFadeTime != dat2->_attackFadeTime)
			game->logger.fatal(std::string(msgStart) + "Butterfly::_attackFadeTime: " + std::to_string(dat1->_attackFadeTime) + " vs " + std::to_string(dat2->_attackFadeTime));
		return length + sizeof(HappyData);
	}

	void Butterfly::defensiveFormation(const Object &target)
	{
		if (this->_copy) {
			this->_position = this->_copy->_position;
			this->_disabled = false;
			this->_maxAlpha = MAX_ALPHA;
			this->_sprite.setColor(sf::Color{
				255, 255, 255,
				static_cast<sf::Uint8>(this->_maxAlpha * this->_alpha)
			});
			return;
		}

		auto owDat = this->_owner->getCurrentFrameData();
		auto opDat = target.getCurrentFrameData();
		auto owCenter = this->_owner->_position + Vector2f{0, owDat->size.y / 2.f};
		auto opCenter = target.getPosition() + Vector2f{0, opDat->size.y / 2.f};
		auto dir = opCenter - owCenter;
		auto normalized = dir.normalized();
		Vector2f normalPt = {normalized.x * 60, normalized.y * 80};
		auto normal = dir.normal(normalPt).normalized();
		auto id = (this->getId() - BUTTERFLIES_START_ID) % NB_BUTTERFLIES;

		normal *= (int)(id - NB_BUTTERFLIES / 2) * 10;
		this->_position = owCenter + normal + normalPt;
		this->_disabled = false;
		this->_maxAlpha = MAX_ALPHA;
		this->_base = this->_position;
		this->_ctr = 0;
		this->_sprite.setColor(sf::Color{
			255, 255, 255,
			static_cast<sf::Uint8>(this->_maxAlpha * this->_alpha)
		});
		this->_defenseCtr = 15;
	}

	void Butterfly::_computeFrameDataCache()
	{
		if (this->_actionBlock >= 2)
			return Object::_computeFrameDataCache();

		auto angle = this->getAngle();
		auto scale = 1.5 - std::sin(angle) * 0.5;

		Object::_computeFrameDataCache();
		this->_fdCache.size = this->_fdCache.textureBounds.size * scale;
	}

	void Butterfly::startAttack(Vector2f pos, unsigned int block, unsigned char fadeTime, unsigned char attackTime, unsigned char travelTime)
	{
		this->_attackPos = pos;
		this->_attackId = block;
		this->_attackFadeCtr = fadeTime;
		this->_attackAppearCtr = fadeTime;
		this->_attackFadeTime = fadeTime;
		this->_attackAttackCtr = attackTime;
		this->_attackTravelCtr = travelTime;
	}

	void Butterfly::_updateCurrentAttack()
	{
		if (!this->_attackTravelCtr && !this->_attackAttackCtr)
			return;
		if (this->_attackFadeCtr) {
			this->_attackFadeCtr--;
			this->_maxAlpha = MAX_ALPHA * this->_attackFadeCtr / this->_attackFadeTime;
			if (!this->_attackFadeCtr) {
				if (this->_attackTravelCtr)
					this->_attackPosStep = (this->_attackPos - this->_position) / this->_attackTravelCtr;
				else
					this->_position = this->_attackPos;
			}
		} else if (this->_attackAppearCtr) {
			this->_attackAppearCtr--;
			this->_maxAlpha = MAX_ALPHA * (this->_attackFadeTime - this->_attackAppearCtr) / this->_attackFadeTime;
		}
		if (this->_attackAttackCtr) {
			this->_attackAttackCtr--;
			if (!this->_attackAttackCtr) {
				this->_actionBlock = this->_attackId;
				this->_rotation = 0;
				this->_animation = 0;
				this->_animationCtr = 0;
				this->_alpha = 1;
				this->_hasHit = false;
				this->_newAnim = true;
			}
			return;
		}
		if (this->_attackTravelCtr) {
			this->_position += this->_attackPosStep;
			this->_attackTravelCtr--;
		}
	}

	void Butterfly::_onMoveEnd(const FrameData &lastData)
	{
		if (this->_actionBlock == this->_attackId && this->_attackId)
			this->_attackId = 0;
		Object::_onMoveEnd(lastData);
	}

	bool Butterfly::isDisabled(const Object &target) const
	{
		return this->_owner->_hasHit && !dynamic_cast<const Shadow *>(&target);
	}

	void Butterfly::hit(Object &other, const FrameData *data)
	{
		Object::hit(other, data);
		this->_owner->_hasHit = true;
		this->_hitStop = 0;
	}

	bool Butterfly::hits(const Object &other) const
	{
		auto old = this->_hasHit;
		auto t = const_cast<Butterfly *>(this);
		auto shadow = dynamic_cast<const Shadow *>(&other);

		if (shadow && shadow->getOwner() == this->_team)
			t->_hasHit = false;

		auto result = SubObject::hits(other);

		t->_hasHit = old;
		return result;
	}

	size_t Butterfly::printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const
	{
		auto length = Object::printContent(msgStart, data, startOffset, dataSize);

		if (length == 0)
			return 0;
		game->logger.info("Butterfly @" + std::to_string(startOffset + length));
		if (this->_copy)
			return length;

		auto dat1 = reinterpret_cast<HappyData *>((uintptr_t)data + length);

		if (startOffset + length + sizeof(HappyData) >= dataSize)
			game->logger.warn("Object is " + std::to_string(startOffset + length + sizeof(Data) - dataSize) + " bytes bigger than input");
		game->logger.info(std::string(msgStart) + "Butterfly::_attackPos.x: " + std::to_string(dat1->_attackPos.x));
		game->logger.info(std::string(msgStart) + "Butterfly::_attackPos.y: " + std::to_string(dat1->_attackPos.y));
		game->logger.info(std::string(msgStart) + "Butterfly::_attackPosStep.x: " + std::to_string(dat1->_attackPosStep.x));
		game->logger.info(std::string(msgStart) + "Butterfly::_attackPosStep.y: " + std::to_string(dat1->_attackPosStep.y));
		game->logger.info(std::string(msgStart) + "Butterfly::_attackAppearCtr: " + std::to_string(dat1->_attackAppearCtr));
		game->logger.info(std::string(msgStart) + "Butterfly::_attackId: " + std::to_string(dat1->_attackId));
		game->logger.info(std::string(msgStart) + "Butterfly::_attackFadeCtr: " + std::to_string(dat1->_attackFadeCtr));
		game->logger.info(std::string(msgStart) + "Butterfly::_attackAttackCtr: " + std::to_string(dat1->_attackAttackCtr));
		game->logger.info(std::string(msgStart) + "Butterfly::_attackTravelCtr: " + std::to_string(dat1->_attackTravelCtr));
		game->logger.info(std::string(msgStart) + "Butterfly::_counter: " + std::to_string(dat1->_counter));
		game->logger.info(std::string(msgStart) + "Butterfly::_defenseCtr: " + std::to_string(dat1->_defenseCtr));
		game->logger.info(std::string(msgStart) + "Butterfly::_target.x: " + std::to_string(dat1->_target.x));
		game->logger.info(std::string(msgStart) + "Butterfly::_target.y: " + std::to_string(dat1->_target.y));
		game->logger.info(std::string(msgStart) + "Butterfly::_base.x: " + std::to_string(dat1->_base.x));
		game->logger.info(std::string(msgStart) + "Butterfly::_base.y: " + std::to_string(dat1->_base.y));
		game->logger.info(std::string(msgStart) + "Butterfly::_ctr: " + std::to_string(dat1->_ctr));
		game->logger.info(std::string(msgStart) + "Butterfly::_maxAlpha: " + std::to_string(dat1->_maxAlpha));
		game->logger.info(std::string(msgStart) + "Butterfly::_attackFadeTime: " + std::to_string(dat1->_attackFadeTime));
		if (startOffset + length + sizeof(HappyData) >= dataSize) {
			game->logger.fatal("Invalid input frame");
			return 0;
		}
		return length + sizeof(HappyData);
	}
}