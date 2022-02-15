//
// Created by PinkySmile on 18/09/2021
//

#include <sstream>
#include "BattleManager.hpp"
#include "../Logger.hpp"
#include "Game.hpp"

#define FIRST_TO 2

namespace Battle
{
	BattleManager::BattleManager(const CharacterParams &leftCharacter, const CharacterParams &rightCharacter) :
		_leftCharacter(leftCharacter.character),
		_rightCharacter(rightCharacter.character)
	{
		//TODO: Move this in another function
		this->_stage.textureHandle = game.textureMgr.load("assets/stages/14687.png");
		this->_stage.setPosition({-50, -600});
		this->_platforms.emplace_back(new Platform("assets/stages/platforms.json", 58 * 2, 2000, 15 * 60, {250, 300}));
		this->_platforms.emplace_back(new Platform("assets/stages/platforms.json", 58 * 2, 2000, 15 * 60, {500, 150}));
		this->_platforms.emplace_back(new Platform("assets/stages/platforms.json", 58 * 2, 2000, 15 * 60, {750, 300}));
		this->_nbPlatform = 3;

		this->_leftCharacter->setOpponent(rightCharacter.character);
		this->_rightCharacter->setOpponent(leftCharacter.character);
		this->_leftCharacter->init(
			true,
			leftCharacter.hp,
			leftCharacter.maxJumps,
			leftCharacter.maxAirDash,
			leftCharacter.matterManaMax,
			leftCharacter.voidManaMax,
			leftCharacter.spiritManaMax,
			leftCharacter.manaRegen,
			leftCharacter.maxBlockStun,
			leftCharacter.gravity
		);
		this->_rightCharacter->init(
			false,
			rightCharacter.hp,
			rightCharacter.maxJumps,
			rightCharacter.maxAirDash,
			rightCharacter.matterManaMax,
			rightCharacter.voidManaMax,
			rightCharacter.spiritManaMax,
			rightCharacter.manaRegen,
			rightCharacter.maxBlockStun,
			rightCharacter.gravity
		);
		this->_leftCharacter->setAttacksDisabled(true);
		this->_rightCharacter->setAttacksDisabled(true);
		this->_roundSprites.resize(5 + FIRST_TO * 2 - 1);
		this->_roundSprites[0].loadFromFile("assets/icons/rounds/ko.png");
		this->_roundSprites[1].loadFromFile("assets/icons/rounds/start.png");
		this->_roundSprites[2].loadFromFile("assets/icons/rounds/p1win.png");
		this->_roundSprites[3].loadFromFile("assets/icons/rounds/p2win.png");
		this->_roundSprites[4].loadFromFile("assets/icons/rounds/id.png");
		for (int i = 1; i < FIRST_TO * 2; i++)
			this->_roundSprites[4 + i].loadFromFile("assets/icons/rounds/round" + std::to_string(i) + ".png");
	}

	void BattleManager::consumeEvent(const sf::Event &event)
	{
		this->_leftCharacter->consumeEvent(event);
		this->_rightCharacter->consumeEvent(event);
		if (!game.networkMgr.isConnected()) {
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::J)
				this->_step = !this->_step;
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::K)
				this->_next = true;
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::U)
				this->_speed--;
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::I)
				this->_speed++;
		}
	}

	bool BattleManager::update()
	{
		if (game.networkMgr.isConnected())
			return this->_updateLoop();

		if (this->_step && !this->_next)
			return true;
		this->_next = false;

		this->_time += this->_speed / 60.f;
		while (this->_time > 1) {
			this->_time -= 1;
			if (!this->_updateLoop())
				return false;
		}
		return true;
	}

	void BattleManager::render()
	{
		float total = 0;

		while (this->_fpsTimes.size() >= 15)
			this->_fpsTimes.pop_front();
		this->_fpsTimes.push_back(this->_fpsClock.restart().asMilliseconds());
		game.textureMgr.render(this->_stage);

		this->_renderLeftHUD();
		this->_renderRightHUD();

		for (auto time : this->_tpsTimes)
			total += time;
		if (!this->_tpsTimes.empty()) {
			char buffer[12];

			sprintf(buffer, "%.2f TPS", 1000.f / (total / this->_tpsTimes.size()));
			game.screen->borderColor(2, sf::Color::Black);
			game.screen->fillColor(sf::Color::White);
			game.screen->textSize(20);
			game.screen->displayElement(buffer, {900, 75}, 145, Screen::ALIGN_RIGHT);
			game.screen->textSize(30);
			game.screen->borderColor(0, sf::Color::Transparent);
		}
		total = 0;
		for (auto time : this->_fpsTimes)
			total += time;
		if (!this->_fpsTimes.empty()) {
			char buffer[12];

			sprintf(buffer, "%.2f FPS", 1000.f / (total / this->_fpsTimes.size()));
			game.screen->borderColor(2, sf::Color::Black);
			game.screen->fillColor(sf::Color::White);
			game.screen->textSize(20);
			game.screen->displayElement(buffer, {900, 50}, 145, Screen::ALIGN_RIGHT);
			game.screen->textSize(30);
			game.screen->borderColor(0, sf::Color::Transparent);
		}
		game.networkMgr.renderHUD();

		for (auto &platform : this->_platforms)
			platform->render();
		this->_leftCharacter->render();
		this->_rightCharacter->render();
		for (auto &object : this->_objects)
			object.second->render();
		if (this->_roundEndTimer < 120 && (this->_leftCharacter->_hp <= 0 || this->_rightCharacter->_hp <= 0 || this->_roundEndTimer))
			this->_renderRoundEndAnimation();
		else if (this->_score.first == FIRST_TO || this->_score.second == FIRST_TO)
			this->_renderEndGameAnimation();
		else if (this->_roundStartTimer < 140)
			this->_renderRoundStartAnimation();
	}

	unsigned BattleManager::registerObject(const std::shared_ptr<IObject> &object)
	{
		do {
			this->_lastObjectId++;
			for (auto &pair : this->_objects)
				if (pair.first == this->_lastObjectId)
					continue;
			if (this->_lastObjectId)
				break;
		} while (true);
		this->_objects.emplace_back(this->_lastObjectId, object);
		return this->_lastObjectId;
	}

	void BattleManager::addHitStop(unsigned int stop)
	{
		this->_hitStop = std::max(stop, this->_hitStop);
	}

	const Character *BattleManager::getLeftCharacter() const
	{
		return &*this->_leftCharacter;
	}

	const Character *BattleManager::getRightCharacter() const
	{
		return &*this->_rightCharacter;
	}

	Character *BattleManager::getLeftCharacter()
	{
		return &*this->_leftCharacter;
	}

	Character *BattleManager::getRightCharacter()
	{
		return &*this->_rightCharacter;
	}

	void BattleManager::_updateRoundEndAnimation()
	{
		if (this->_roundEndTimer == 120) {
			this->_roundEndTimer +=
				(this->_leftCharacter->_hp > 0 || this->_leftCharacter->_action == ACTION_KNOCKED_DOWN) &&
				(this->_rightCharacter->_hp > 0 || this->_rightCharacter->_action == ACTION_KNOCKED_DOWN);
			return;
		}
		if (this->_roundEndTimer == 0) {
			this->_score.first += this->_rightCharacter->_hp <= 0;
			this->_score.second += this->_leftCharacter->_hp <= 0;
			this->_currentRound++;
			if (this->_score.first == this->_score.second && this->_score.first == FIRST_TO) {
				this->_score.first--;
				this->_score.second--;
			}
			this->_leftCharacter->setAttacksDisabled(true);
			this->_rightCharacter->setAttacksDisabled(true);
			this->_roundStartTimer = -120;
		}

		float scale;
		float alpha;

		if (this->_roundEndTimer < 0x11) {
			alpha = this->_roundEndTimer * 15;
			scale = this->_roundEndTimer / 17.f;
		} else if (this->_roundEndTimer < 103) {
			alpha = 0xFF;
			scale = 1 + (this->_roundEndTimer - 0x11) / 206.f;
		} else {
			alpha = 0xFF - ((this->_roundEndTimer - 103) * 0xFF / 0x11);
			scale = 1.5 + (this->_roundEndTimer - 103) / 8.5f;
		}

		this->_roundSprite.setTexture(this->_roundSprites[0], true);
		this->_roundSprite.setScale({scale, scale});
		this->_roundSprite.setColor(sf::Color{0xFF, 0xFF, 0xFF, static_cast<sf::Uint8>(alpha)});
		this->_roundSprite.setOrigin(
			this->_roundSprite.getTexture()->getSize().x / 2.f,
			this->_roundSprite.getTexture()->getSize().y / 2.f
		);
		this->_roundSprite.setPosition({500, -250});
		this->_roundEndTimer++;
	}

	void BattleManager::_renderRoundEndAnimation() const
	{
		game.screen->draw(this->_roundSprite);
	}

	void BattleManager::_updateRoundStartAnimation()
	{
		if (this->_roundStartTimer < 0) {
			this->_roundStartTimer++;
			return;
		}
		if (this->_roundStartTimer == 0) {
			this->_roundEndTimer = 0;
			this->_leftCharacter->reset();
			this->_rightCharacter->reset();
		} else if (this->_roundStartTimer == 120) {
			this->_leftCharacter->setAttacksDisabled(false);
			this->_rightCharacter->setAttacksDisabled(false);
		}
		if (this->_roundStartTimer < 120) {
			auto scale = std::exp(this->_roundStartTimer / -2.f) + 1;
			auto alpha = this->_roundStartTimer > 0x11 ? 0xFF : this->_roundStartTimer * 15;

			if (this->_currentRound >= FIRST_TO * 2 - 1)
				this->_roundSprite.setTexture(this->_roundSprites[4], true);
			else
				this->_roundSprite.setTexture(this->_roundSprites[5 + this->_currentRound], true);
			this->_roundSprite.setScale({scale, scale});
			this->_roundSprite.setColor(sf::Color{0xFF, 0xFF, 0xFF, static_cast<sf::Uint8>(alpha)});
		} else {
			auto scale = std::exp((this->_roundStartTimer - 120) / 10.f);
			auto alpha = (20 - (this->_roundStartTimer - 120)) / 20.f * 0xFF;

			this->_roundSprite.setTexture(this->_roundSprites[1], true);
			this->_roundSprite.setScale({scale, scale});
			this->_roundSprite.setColor(sf::Color{0xFF, 0xFF, 0xFF, static_cast<sf::Uint8>(alpha)});
		}
		this->_roundSprite.setOrigin(
			this->_roundSprite.getTexture()->getSize().x / 2.f,
			this->_roundSprite.getTexture()->getSize().y / 2.f
		);
		this->_roundSprite.setPosition({500, -250});
		this->_roundStartTimer++;
	}

	void BattleManager::_renderRoundStartAnimation() const
	{
		if (this->_roundStartTimer < 0)
			return;
		game.screen->draw(this->_roundSprite);
	}

	bool BattleManager::_updateEndGameAnimation()
	{
		float scale;
		float alpha;

		this->_leftCharacter->disableInputs(true);
		this->_rightCharacter->disableInputs(true);
		if (this->_roundStartTimer < -120 + 0x11) {
			alpha = this->_roundStartTimer * 15;
			scale = this->_roundStartTimer / 17.f;
		} else if (this->_roundStartTimer < 0x11) {
			alpha = 0xFF;
			scale = this->_roundStartTimer / 17.f;
		} else if (this->_roundStartTimer < 120) {
			alpha = 0xFF;
			scale = 1 + (this->_roundStartTimer - 0x11) / 206.f;
		} else {
			auto diff = (this->_roundStartTimer - 120) * 0xFF / 0x11;

			if (diff < 0xFF)
				alpha = 0xFF - diff;
			else
				alpha = 0;
			scale = 1 + (this->_roundStartTimer - 0x11) / 206.f;
		}

		this->_roundSprite.setTexture(this->_roundSprites[2 + (this->_score.second == FIRST_TO)], true);
		this->_roundSprite.setScale({scale, scale});
		this->_roundSprite.setColor(sf::Color{0xFF, 0xFF, 0xFF, static_cast<sf::Uint8>(alpha)});
		this->_roundSprite.setOrigin(
			this->_roundSprite.getTexture()->getSize().x / 2.f,
			this->_roundSprite.getTexture()->getSize().y / 2.f
		);
		this->_roundSprite.setPosition({500, -250});
		this->_roundStartTimer++;
		return this->_roundStartTimer <= 140;
	}

	void BattleManager::_renderEndGameAnimation() const
	{
		game.screen->draw(this->_roundSprite);
	}

	void BattleManager::_gameUpdate()
	{
		std::vector<std::tuple<IObject *, IObject *, const FrameData *>> collisions;

		if (this->_hitStop) {
			this->_hitStop--;
			this->_leftCharacter->updateInputs();
			this->_rightCharacter->updateInputs();
			return;
		}

		auto ldata = this->_leftCharacter->getCurrentFrameData();
		auto rdata = this->_rightCharacter->getCurrentFrameData();
		auto lchr = &*this->_leftCharacter;
		auto rchr = &*this->_rightCharacter;

		for (auto &platform : this->_platforms)
			platform->update();
		if (!rdata->dFlag.flash || ldata->dFlag.flash)
			lchr->update();
		if (!ldata->dFlag.flash)
			rchr->update();

		if (!ldata->dFlag.flash && !rdata->dFlag.flash) {
			for (auto &object: this->_objects)
				object.second->update();
			if (lchr->hits(*rchr))
				collisions.emplace_back(&*lchr, &*rchr, lchr->getCurrentFrameData());
			if (rchr->hits(*lchr))
				collisions.emplace_back(&*rchr, &*lchr, rchr->getCurrentFrameData());

			for (auto &platform: this->_platforms) {
				if (lchr->hits(*platform))
					collisions.emplace_back(&*lchr, &*platform, lchr->getCurrentFrameData());
				if (platform->hits(*lchr))
					collisions.emplace_back(&*platform, &*lchr, platform->getCurrentFrameData());

				if (rchr->hits(*platform))
					collisions.emplace_back(&*rchr, &*platform, rchr->getCurrentFrameData());
				if (platform->hits(*rchr))
					collisions.emplace_back(&*platform, &*rchr, platform->getCurrentFrameData());
			}

			for (auto &object: this->_objects) {
				if (lchr->hits(*object.second))
					collisions.emplace_back(&*lchr, &*object.second, lchr->getCurrentFrameData());
				if (object.second->hits(*lchr))
					collisions.emplace_back(&*object.second, &*lchr, object.second->getCurrentFrameData());

				if (rchr->hits(*object.second))
					collisions.emplace_back(&*rchr, &*object.second, rchr->getCurrentFrameData());
				if (object.second->hits(*rchr))
					collisions.emplace_back(&*object.second, &*rchr, object.second->getCurrentFrameData());

				for (auto &object2: this->_objects)
					if (object2.second != object.second)
						if (object.second->hits(*object2.second))
							collisions.emplace_back(&*object.second, &*object2.second, object.second->getCurrentFrameData());
			}

			for (auto &[attacker, defender, data]: collisions) {
				attacker->hit(*defender, data);
				defender->getHit(*attacker, data);
			}
		}

		if (lchr->isDead())
			// The state is messed up
			// TODO: Do real exceptions
			throw std::invalid_argument("Invalid state");
		if (rchr->isDead())
			// The state is messed up
			// TODO: Do real exceptions
			throw std::invalid_argument("Invalid state");

		for (unsigned i = 0; i < this->_objects.size(); i++)
			if (this->_objects[i].second->isDead())
				this->_objects.erase(this->_objects.begin() + i--);

		if (lchr->collides(*rchr))
			lchr->collide(*rchr);
		for (auto &object: this->_objects) {
			if (lchr->collides(*object.second))
				lchr->collide(*object.second);
			if (rchr->collides(*object.second))
				rchr->collide(*object.second);
		}
		lchr->postUpdate();
		rchr->postUpdate();
		if (this->_leftComboCtr)
			this->_leftComboCtr--;
		if (this->_rightComboCtr)
			this->_rightComboCtr--;
		if (this->_rightCharacter->_comboCtr) {
			this->_leftHitCtr       = this->_rightCharacter->_comboCtr;
			this->_leftNeutralLimit = this->_rightCharacter->_limit[0];
			this->_leftVoidLimit    = this->_rightCharacter->_limit[1];
			this->_leftMatterLimit  = this->_rightCharacter->_limit[2];
			this->_leftSpiritLimit  = this->_rightCharacter->_limit[3];
			this->_leftTotalDamage  = this->_rightCharacter->_totalDamage;
			this->_leftProration    = this->_rightCharacter->_prorate;
			this->_leftCounter      = this->_rightCharacter->_counter;
			this->_leftComboCtr     = 120;
		}
		if (this->_leftCharacter->_comboCtr) {
			this->_rightHitCtr       = this->_leftCharacter->_comboCtr;
			this->_rightNeutralLimit = this->_leftCharacter->_limit[0];
			this->_rightVoidLimit    = this->_leftCharacter->_limit[1];
			this->_rightMatterLimit  = this->_leftCharacter->_limit[2];
			this->_rightSpiritLimit  = this->_leftCharacter->_limit[3];
			this->_rightTotalDamage  = this->_leftCharacter->_totalDamage;
			this->_rightProration    = this->_leftCharacter->_prorate;
			this->_rightCounter      = this->_leftCharacter->_counter;
			this->_rightComboCtr     = 120;
		}
	}

	std::shared_ptr<IObject> BattleManager::getObjectFromId(unsigned int id) const
	{
		for (auto &object : this->_objects)
			if (object.first == id)
				return object.second;
		assert(false);
		return nullptr;
	}

	unsigned int BattleManager::getBufferSize() const
	{
		size_t size = sizeof(Data) + this->_leftCharacter->getBufferSize() + this->_rightCharacter->getBufferSize() + this->_objects.size() * (sizeof(unsigned) + sizeof(unsigned char));

		for (auto &object : this->_objects) {
			size += object.second->getBufferSize();
			size += (object.second->getClassId() == 2) * (sizeof(bool) + sizeof(unsigned));
		}
		for (size_t i = 0; i < this->_nbPlatform; i++)
			size += this->_platforms[i]->getBufferSize();
		return size;
	}

	void BattleManager::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>(data);
		ptrdiff_t ptr = (ptrdiff_t)data + sizeof(Data);

		dat->_lastObjectId = this->_lastObjectId;
		dat->_leftComboCtr = this->_leftComboCtr;
		dat->_leftHitCtr = this->_leftHitCtr;
		dat->_leftNeutralLimit = this->_leftNeutralLimit;
		dat->_leftVoidLimit = this->_leftVoidLimit;
		dat->_leftMatterLimit = this->_leftMatterLimit;
		dat->_leftSpiritLimit = this->_leftSpiritLimit;
		dat->_leftTotalDamage = this->_leftTotalDamage;
		dat->_leftProration = this->_leftProration;
		dat->_rightComboCtr = this->_rightComboCtr;
		dat->_rightHitCtr = this->_rightHitCtr;
		dat->_rightNeutralLimit = this->_rightNeutralLimit;
		dat->_rightVoidLimit = this->_rightVoidLimit;
		dat->_rightSpiritLimit = this->_rightSpiritLimit;
		dat->_rightMatterLimit = this->_rightMatterLimit;
		dat->_rightTotalDamage = this->_rightTotalDamage;
		dat->_rightProration = this->_rightProration;
		dat->_score = this->_score;
		dat->_currentRound = this->_currentRound;
		dat->_roundStartTimer = this->_roundStartTimer;
		dat->_roundEndTimer = this->_roundEndTimer;
		dat->_hitStop = this->_hitStop;
		dat->_nbObjects = this->_objects.size();
		this->_leftCharacter->copyToBuffer((void *)ptr);
		ptr += this->_leftCharacter->getBufferSize();
		this->_rightCharacter->copyToBuffer((void *)ptr);
		ptr += this->_rightCharacter->getBufferSize();
		for (auto &object : this->_objects) {
			*(unsigned *)ptr = object.first;
			ptr += sizeof(unsigned);
			*(unsigned char *)ptr = object.second->getClassId();
			ptr += sizeof(unsigned char);
			if (object.second->getClassId() == 2) {
#ifdef _DEBUG
				assert(dynamic_cast<Projectile *>(&*object.second));
#endif
				auto obj = reinterpret_cast<Projectile *>(&*object.second);

				*(bool *)ptr = obj->getOwner();
				ptr += sizeof(bool);
				*(unsigned *)ptr = obj->getId();
				ptr += sizeof(unsigned);
			}
			object.second->copyToBuffer((void *)ptr);
			ptr += object.second->getBufferSize();
		}
		for (size_t i = 0; i < this->_nbPlatform; i++) {
			this->_platforms[i]->copyToBuffer((void *)ptr);
			ptr += this->_platforms[i]->getBufferSize();
		}
	}

	void BattleManager::restoreFromBuffer(void *data)
	{
		auto dat = reinterpret_cast<Data *>(data);
		ptrdiff_t ptr = (ptrdiff_t)data + sizeof(Data);

		this->_lastObjectId = dat->_lastObjectId;
		this->_leftComboCtr = dat->_leftComboCtr;
		this->_leftHitCtr = dat->_leftHitCtr;
		this->_leftNeutralLimit = dat->_leftNeutralLimit;
		this->_leftVoidLimit = dat->_leftVoidLimit;
		this->_leftMatterLimit = dat->_leftMatterLimit;
		this->_leftSpiritLimit = dat->_leftSpiritLimit;
		this->_leftTotalDamage = dat->_leftTotalDamage;
		this->_leftProration = dat->_leftProration;
		this->_rightComboCtr = dat->_rightComboCtr;
		this->_rightHitCtr = dat->_rightHitCtr;
		this->_rightNeutralLimit = dat->_rightNeutralLimit;
		this->_rightVoidLimit = dat->_rightVoidLimit;
		this->_rightSpiritLimit = dat->_rightSpiritLimit;
		this->_rightMatterLimit = dat->_rightMatterLimit;
		this->_rightTotalDamage = dat->_rightTotalDamage;
		this->_rightProration = dat->_rightProration;
		this->_score = dat->_score;
		this->_currentRound = dat->_currentRound;
		this->_roundStartTimer = dat->_roundStartTimer;
		this->_roundEndTimer = dat->_roundEndTimer;
		this->_hitStop = dat->_hitStop;
		this->_leftCharacter->_removeSubobjects();
		this->_rightCharacter->_removeSubobjects();
		this->_leftCharacter->restoreFromBuffer((void *)ptr);
		ptr += this->_leftCharacter->getBufferSize();
		this->_rightCharacter->restoreFromBuffer((void *)ptr);
		ptr += this->_rightCharacter->getBufferSize();

		this->_objects.clear();
		this->_objects.reserve(dat->_nbObjects);
		this->_platforms.erase(this->_platforms.begin() + this->_nbPlatform, this->_platforms.end());
		for (size_t i = 0; i < dat->_nbObjects; i++) {
			std::shared_ptr<IObject> obj;
			auto id = *(unsigned *)ptr;

			ptr += sizeof(unsigned);

			auto cl = *(unsigned char *)ptr;

			ptr += sizeof(unsigned char);
			switch (cl) {
			case 0:
				obj.reset(new Object());
				break;
			case 1:
				obj.reset(new Character());
				break;
			case 2: {
				auto owner = *(bool *)ptr;

				ptr += sizeof(bool);

				auto subobjid = *(unsigned *)ptr;

				ptr += sizeof(unsigned);
				obj = (owner ? this->_rightCharacter : this->_leftCharacter)->_spawnSubobject(subobjid, false).second;
				break;
			}
			default:
				throw std::invalid_argument("Wtf?" + std::to_string(cl));
			}

			obj->restoreFromBuffer((void *)ptr);
			ptr += obj->getBufferSize();
			this->_objects.emplace_back(id, obj);
		}
		for (size_t i = 0; i < this->_nbPlatform; i++) {
			this->_platforms[i]->restoreFromBuffer((void *)ptr);
			ptr += this->_platforms[i]->getBufferSize();
		}
		this->_leftCharacter->resolveSubObjects(*this);
		this->_rightCharacter->resolveSubObjects(*this);
	}

	bool BattleManager::_updateLoop()
	{
		while (this->_tpsTimes.size() >= 15)
			this->_tpsTimes.pop_front();
		this->_tpsTimes.push_back(this->_tpsClock.restart().asMilliseconds());

		if (
			this->_roundEndTimer > 120 ||
			(this->_leftCharacter->_hp > 0 && this->_rightCharacter->_hp > 0 && !this->_roundEndTimer) ||
			this->_roundEndTimer % 2 == 0
		)
			this->_gameUpdate();

		if (this->_roundEndTimer <= 120 && (this->_leftCharacter->_hp <= 0 || this->_rightCharacter->_hp <= 0 || this->_roundEndTimer))
			this->_updateRoundEndAnimation();
		else if (this->_score.first == FIRST_TO || this->_score.second == FIRST_TO) {
			if (!this->_updateEndGameAnimation())
				return false;
		} else if (this->_roundStartTimer < 140)
			this->_updateRoundStartAnimation();
		return true;
	}

	void BattleManager::_renderLeftHUD() const
	{
		sf::RectangleShape rect;

		rect.setOutlineThickness(1);
		rect.setOutlineColor(sf::Color::Black);

		rect.setFillColor(sf::Color{0xA0, 0xA0, 0xA0});
		rect.setPosition(0, -590);
		rect.setSize({400.f, 20});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color{0xFF, 0x50, 0x50});
		rect.setPosition(0, -590);
		rect.setSize({400.f * std::min<float>(this->_leftCharacter->_hp + this->_leftCharacter->_totalDamage, this->_rightCharacter->_baseHp) / this->_leftCharacter->_baseHp, 20});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color::Yellow);
		rect.setPosition(0, -590);
		rect.setSize({400.f * this->_leftCharacter->_hp / this->_leftCharacter->_baseHp, 20});
		if (this->_leftCharacter->_hp > 0)
			game.screen->draw(rect);

		rect.setFillColor(sf::Color::White);
		for (int i = 0; i < FIRST_TO; i++) {
			rect.setPosition(390 - i * 15, -560);
			rect.setSize({10, 8});
			game.screen->draw(rect);
		}
		rect.setFillColor(sf::Color{0xFF, 0x80, 0x00});
		for (int i = 0; i < this->_score.first; i++) {
			rect.setPosition(392 - i * 15, -558);
			rect.setSize({6, 4});
			game.screen->draw(rect);
		}

		rect.setFillColor(sf::Color{51, 204, 204});
		rect.setPosition(100, 40);
		rect.setSize({200.f * this->_leftCharacter->_spiritMana / this->_leftCharacter->_spiritManaMax, 10});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color{187, 94, 0});
		rect.setPosition(100, 55);
		rect.setSize({200.f * this->_leftCharacter->_matterMana / this->_leftCharacter->_matterManaMax, 10});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color{0x80, 0x00, 0x80});
		rect.setPosition(100, 70);
		rect.setSize({200.f * this->_leftCharacter->_voidMana / this->_leftCharacter->_voidManaMax, 10});
		game.screen->draw(rect);

		if (this->_leftComboCtr) {
			unsigned char alpha = this->_leftComboCtr > 51 ? 0xFF : this->_leftComboCtr * 5;

			game.screen->borderColor(2, sf::Color{0, 0, 0, alpha});
			game.screen->fillColor(sf::Color{0xFF, 0x00, 0x00, alpha});
			game.screen->displayElement(std::to_string(this->_leftHitCtr) + " Hit" + (this->_leftHitCtr < 2 ? "" : "s"), {0, -560}, 400, Screen::ALIGN_LEFT);
			game.screen->textSize(20);
			game.screen->fillColor(sf::Color{0xA0, 0xA0, 0xA0, alpha});
			game.screen->displayElement(std::to_string(this->_leftTotalDamage) + " damage" + (this->_leftTotalDamage < 2 ? "" : "s"), {0, -510}, 400, Screen::ALIGN_LEFT);
			game.screen->displayElement(std::to_string(static_cast<int>(this->_leftProration * 100)) + "% proration", {0, -480}, 400, Screen::ALIGN_LEFT);
			game.screen->textSize(15);
			game.screen->fillColor(sf::Color{0xFF, 0xFF, 0xFF, alpha});
			game.screen->displayElement("Neutral Limit: " + std::to_string(this->_leftNeutralLimit), {0, -445}, 400, Screen::ALIGN_LEFT);
			game.screen->fillColor(sf::Color{0x80, 0x00, 0x80, alpha});
			game.screen->displayElement("Void Limit: " + std::to_string(this->_leftVoidLimit), {0, -415}, 400, Screen::ALIGN_LEFT);
			game.screen->fillColor(sf::Color{187, 94, 0, alpha});
			game.screen->displayElement("Matter Limit: " + std::to_string(this->_leftMatterLimit), {0, -390}, 400, Screen::ALIGN_LEFT);
			game.screen->fillColor(sf::Color{51, 204, 204, alpha});
			game.screen->displayElement("Spirit Limit: " + std::to_string(this->_leftSpiritLimit), {0, -365}, 400, Screen::ALIGN_LEFT);
			if (this->_leftCounter) {
				game.screen->textSize(20);
				game.screen->fillColor(sf::Color{0xFF, 0x40, 0x20, alpha});
				game.screen->displayElement("Counter !", {0, -340}, 400, Screen::ALIGN_LEFT);
			}
			game.screen->borderColor(0, sf::Color{0, 0, 0, 0});
			game.screen->textSize(30);
		}
	}

	void BattleManager::_renderRightHUD() const
	{
		sf::RectangleShape rect;

		rect.setOutlineThickness(1);
		rect.setOutlineColor(sf::Color::Black);

		rect.setFillColor(sf::Color{0xA0, 0xA0, 0xA0});
		rect.setPosition(600.f, -590);
		rect.setSize({400.f, 20});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color{0xFF, 0x50, 0x50});
		rect.setPosition(1000 - 400.f * std::min<float>(this->_rightCharacter->_hp + this->_rightCharacter->_totalDamage, this->_rightCharacter->_baseHp) / this->_rightCharacter->_baseHp, -590);
		rect.setSize({400.f * std::min<float>(this->_rightCharacter->_hp + this->_rightCharacter->_totalDamage, this->_rightCharacter->_baseHp) / this->_rightCharacter->_baseHp, 20});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color::Yellow);
		rect.setPosition(1000 - 400.f * this->_rightCharacter->_hp / this->_rightCharacter->_baseHp, -590);
		rect.setSize({400.f * this->_rightCharacter->_hp / this->_rightCharacter->_baseHp, 20});
		if (this->_rightCharacter->_hp > 0)
			game.screen->draw(rect);

		rect.setFillColor(sf::Color::White);
		for (int i = 0; i < FIRST_TO; i++) {
			rect.setPosition(600 + i * 15, -560);
			rect.setSize({10, 8});
			game.screen->draw(rect);
		}
		rect.setFillColor(sf::Color{0xFF, 0x80, 0x00});
		for (int i = 0; i < this->_score.second; i++) {
			rect.setPosition(602 + i * 15, -558);
			rect.setSize({6, 4});
			game.screen->draw(rect);
		}

		rect.setFillColor(sf::Color{51, 204, 204});
		rect.setPosition(900 - 200.f * this->_rightCharacter->_spiritMana / this->_rightCharacter->_spiritManaMax, 40);
		rect.setSize({200.f * this->_rightCharacter->_spiritMana / this->_rightCharacter->_spiritManaMax, 10});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color{187, 94, 0});
		rect.setPosition(900 - 200.f * this->_rightCharacter->_matterMana / this->_rightCharacter->_matterManaMax, 55);
		rect.setSize({200.f * this->_rightCharacter->_matterMana / this->_rightCharacter->_matterManaMax, 10});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color{0x80, 0x00, 0x80});
		rect.setPosition(900 - 200.f * this->_rightCharacter->_voidMana / this->_rightCharacter->_voidManaMax, 70);
		rect.setSize({200.f * this->_rightCharacter->_voidMana / this->_rightCharacter->_voidManaMax, 10});
		game.screen->draw(rect);

		if (this->_rightComboCtr) {
			unsigned char alpha = this->_rightComboCtr > 51 ? 0xFF : this->_rightComboCtr * 5;

			game.screen->borderColor(2, sf::Color{0, 0, 0, alpha});
			game.screen->fillColor(sf::Color{0xFF, 0x00, 0x00, alpha});
			game.screen->displayElement(std::to_string(this->_rightHitCtr) + " Hit" + (this->_rightHitCtr < 2 ? "" : "s"), {600, -560}, 400, Screen::ALIGN_RIGHT);
			game.screen->textSize(20);
			game.screen->fillColor(sf::Color{0xA0, 0xA0, 0xA0, alpha});
			game.screen->displayElement(std::to_string(this->_rightTotalDamage) + " damage" + (this->_rightTotalDamage < 2 ? "" : "s"), {600, -510}, 400, Screen::ALIGN_RIGHT);
			game.screen->displayElement(std::to_string(static_cast<int>(this->_rightProration * 100)) + "% proration", {600, -480}, 400, Screen::ALIGN_RIGHT);
			game.screen->textSize(15);
			game.screen->fillColor(sf::Color{0xFF, 0xFF, 0xFF, alpha});
			game.screen->displayElement("Neutral Limit: " + std::to_string(this->_rightNeutralLimit), {600, -445}, 400, Screen::ALIGN_RIGHT);
			game.screen->fillColor(sf::Color{0x80, 0x00, 0x80, alpha});
			game.screen->displayElement("Void Limit: " + std::to_string(this->_rightVoidLimit), {600, -415}, 400, Screen::ALIGN_RIGHT);
			game.screen->fillColor(sf::Color{187, 94, 0, alpha});
			game.screen->displayElement("Matter Limit: " + std::to_string(this->_rightMatterLimit), {600, -390}, 400, Screen::ALIGN_RIGHT);
			game.screen->fillColor(sf::Color{51, 204, 204, alpha});
			game.screen->displayElement("Spirit Limit: " + std::to_string(this->_rightSpiritLimit), {600, -365}, 400, Screen::ALIGN_RIGHT);
			if (this->_rightCounter) {
				game.screen->textSize(20);
				game.screen->fillColor(sf::Color{0xFF, 0x40, 0x20, alpha});
				game.screen->displayElement("Counter !", {600, -340}, 400, Screen::ALIGN_RIGHT);
			}
			game.screen->borderColor(0, sf::Color{0, 0, 0, 0});
			game.screen->textSize(30);
		}
	}

	const std::vector<std::shared_ptr<Platform>> &BattleManager::getPlatforms() const
	{
		return this->_platforms;
	}
}