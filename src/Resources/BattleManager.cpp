//
// Created by PinkySmile on 18/09/2021
//

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
		this->_stage.textureHandle = game.textureMgr.load("assets/stages/14687.png");
		this->_stage.setPosition({-50, -600});
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
		sf::RectangleShape rect;

		game.textureMgr.render(this->_stage);

		rect.setOutlineThickness(1);
		rect.setOutlineColor(sf::Color::Black);

		rect.setFillColor(sf::Color{0xA0, 0xA0, 0xA0});
		rect.setPosition(0, -590);
		rect.setSize({400.f, 20});
		game.screen->draw(rect);
		rect.setPosition(600.f, -590);
		rect.setSize({400.f, 20});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color{0xFF, 0x50, 0x50});
		rect.setPosition(0, -590);
		rect.setSize({400.f * std::min<float>(this->_leftCharacter->_hp + this->_leftCharacter->_totalDamage, this->_rightCharacter->_baseHp) / this->_leftCharacter->_baseHp, 20});
		game.screen->draw(rect);
		rect.setPosition(1000 - 400.f * std::min<float>(this->_rightCharacter->_hp + this->_rightCharacter->_totalDamage, this->_rightCharacter->_baseHp) / this->_rightCharacter->_baseHp, -590);
		rect.setSize({400.f * std::min<float>(this->_rightCharacter->_hp + this->_rightCharacter->_totalDamage, this->_rightCharacter->_baseHp) / this->_rightCharacter->_baseHp, 20});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color::Yellow);
		rect.setPosition(0, -590);
		rect.setSize({400.f * this->_leftCharacter->_hp / this->_leftCharacter->_baseHp, 20});
		if (this->_leftCharacter->_hp > 0)
			game.screen->draw(rect);
		rect.setPosition(1000 - 400.f * this->_rightCharacter->_hp / this->_rightCharacter->_baseHp, -590);
		rect.setSize({400.f * this->_rightCharacter->_hp / this->_rightCharacter->_baseHp, 20});
		if (this->_rightCharacter->_hp > 0)
			game.screen->draw(rect);

		rect.setFillColor(sf::Color::White);
		for (int i = 0; i < FIRST_TO; i++) {
			rect.setPosition(390 - i * 15, -560);
			rect.setSize({10, 8});
			game.screen->draw(rect);
		}
		for (int i = 0; i < FIRST_TO; i++) {
			rect.setPosition(600 + i * 15, -560);
			rect.setSize({10, 8});
			game.screen->draw(rect);
		}
		rect.setFillColor(sf::Color{0xFF, 0x80, 0x00});
		for (int i = 0; i < this->_score.first; i++) {
			rect.setPosition(392 - i * 15, -558);
			rect.setSize({6, 4});
			game.screen->draw(rect);
		}
		for (int i = 0; i < this->_score.second; i++) {
			rect.setPosition(602 + i * 15, -558);
			rect.setSize({6, 4});
			game.screen->draw(rect);
		}

		rect.setFillColor(sf::Color{51, 204, 204});
		rect.setPosition(100, 40);
		rect.setSize({200.f * this->_leftCharacter->_spiritMana / this->_leftCharacter->_spiritManaMax, 10});
		game.screen->draw(rect);
		rect.setPosition(900 - 200.f * this->_rightCharacter->_spiritMana / this->_rightCharacter->_spiritManaMax, 40);
		rect.setSize({200.f * this->_rightCharacter->_spiritMana / this->_rightCharacter->_spiritManaMax, 10});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color{187, 94, 0});
		rect.setPosition(100, 55);
		rect.setSize({200.f * this->_leftCharacter->_matterMana / this->_leftCharacter->_matterManaMax, 10});
		game.screen->draw(rect);
		rect.setPosition(900 - 200.f * this->_rightCharacter->_matterMana / this->_rightCharacter->_matterManaMax, 55);
		rect.setSize({200.f * this->_rightCharacter->_matterMana / this->_rightCharacter->_matterManaMax, 10});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color{0x80, 0x00, 0x80});
		rect.setPosition(100, 70);
		rect.setSize({200.f * this->_leftCharacter->_voidMana / this->_leftCharacter->_voidManaMax, 10});
		game.screen->draw(rect);
		rect.setPosition(900 - 200.f * this->_rightCharacter->_voidMana / this->_rightCharacter->_voidManaMax, 70);
		rect.setSize({200.f * this->_rightCharacter->_voidMana / this->_rightCharacter->_voidManaMax, 10});
		game.screen->draw(rect);

		game.networkMgr.renderHUD();

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

	const ACharacter *BattleManager::getLeftCharacter() const
	{
		return &*this->_leftCharacter;
	}

	const ACharacter *BattleManager::getRightCharacter() const
	{
		return &*this->_rightCharacter;
	}

	ACharacter *BattleManager::getLeftCharacter()
	{
		return &*this->_leftCharacter;
	}

	ACharacter *BattleManager::getRightCharacter()
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

		for (auto &object : this->_objects)
			size += object.second->getBufferSize();
		return size;
	}

	void BattleManager::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>(data);
		ptrdiff_t ptr = (ptrdiff_t)data + sizeof(Data);

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
			object.second->copyToBuffer((void *)ptr);
			ptr += object.second->getBufferSize();
		}
	}

	void BattleManager::restoreFromBuffer(void *data)
	{
		auto dat = reinterpret_cast<Data *>(data);
		ptrdiff_t ptr = (ptrdiff_t)data + sizeof(Data);

		this->_score = dat->_score;
		this->_currentRound = dat->_currentRound;
		this->_roundStartTimer = dat->_roundStartTimer;
		this->_roundEndTimer = dat->_roundEndTimer;
		this->_hitStop = dat->_hitStop;
		this->_objects.clear();
		this->_objects.reserve(dat->_nbObjects);
		this->_leftCharacter->restoreFromBuffer((void *)ptr);
		ptr += this->_leftCharacter->getBufferSize();
		this->_rightCharacter->restoreFromBuffer((void *)ptr);
		ptr += this->_rightCharacter->getBufferSize();
		for (size_t i = 0; i < dat->_nbObjects; i++) {
			std::shared_ptr<IObject> obj;
			auto id = *(unsigned *)ptr;

			ptr += sizeof(unsigned);

			auto cl = *(unsigned char *)ptr;

			switch (cl) {
			case 0:
				obj.reset(new AObject());
				break;
			case 1:
				obj.reset(new ACharacter());
				break;
			case 2:
				obj.reset(new AProjectile());
				break;
			}

			obj->restoreFromBuffer((void *)ptr);
			ptr += obj->getBufferSize();
			this->_objects.emplace_back(id, obj);
		}
		this->_leftCharacter->resolveSubObjects(*this);
		this->_rightCharacter->resolveSubObjects(*this);
	}

	bool BattleManager::_updateLoop()
	{
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
}