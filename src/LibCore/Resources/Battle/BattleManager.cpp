//
// Created by PinkySmile on 18/09/2021.
//

#include <sstream>
#include "BattleManager.hpp"
#include "Logger.hpp"
#include "Resources/Game.hpp"
#include "Objects/Characters/SubObject.hpp"

#define FIRST_TO 2
#define INPUT_DISPLAY_SIZE 24

namespace SpiralOfFate
{
	static const char *battleHudSprite[] = {
		"assets/battleui/player_hud.png",        // BATTLEUI_HUD_SEAT
		"assets/battleui/meterbars.png",         // BATTLEUI_MANA_BAR
		"assets/battleui/guard.png",             // BATTLEUI_GUARD_TEXT
		"assets/battleui/guardbar.png",          // BATTLEUI_GUARD_BAR
		"assets/battleui/guard_red.png",         // BATTLEUI_GUARD_BAR_DISABLED
		"assets/battleui/lifebar.png",           // BATTLEUI_LIFE_BAR
		"assets/battleui/lifebar_red.png",       // BATTLEUI_LIFE_BAR_RED
		"assets/battleui/lifebar_texture.png",   // BATTLEUI_LIFE_BAR_EFFECT
		"assets/battleui/overdrive.png",         // BATTLEUI_OVERDRIVE
		"assets/battleui/overdrive_outline.png", // BATTLEUI_OVERDRIVE_OUTLINE
		"assets/battleui/round_container.png",  // BATTLEUI_SCORE_SEAT
		"assets/battleui/round_point.png",      // BATTLEUI_SCORE_BULLET
	};

	BattleManager::BattleManager(const StageParams &stage, const CharacterParams &leftCharacter, const CharacterParams &rightCharacter) :
		_leftCharacter(leftCharacter.character),
		_rightCharacter(rightCharacter.character),
		_leftHUDData{*this, *this->_leftCharacter, false},
		_rightHUDData{*this, *this->_rightCharacter, true}
	{
		for (unsigned i = 0; i < spritesPaths.size(); i++)
			this->_moveSprites[i] = game->textureMgr.load(spritesPaths[i]);

		//TODO: Move this in another function
		this->_stage.textureHandle = game->textureMgr.load(stage.path);
		this->_stage.setPosition({-50, -600});
		for (auto object : stage.objects())
			this->_stageObjects.emplace_back(object);
		for (auto object : stage.platforms())
			this->_platforms.emplace_back(object);
		this->_nbPlatform = this->_platforms.size();

		this->_leftCharacter->setOpponent(rightCharacter.character);
		this->_rightCharacter->setOpponent(leftCharacter.character);
		this->_leftCharacter->init(*this, {
			true,
			leftCharacter.hp,
			leftCharacter.maxJumps,
			leftCharacter.maxAirDash,
			leftCharacter.matterManaMax,
			leftCharacter.voidManaMax,
			leftCharacter.spiritManaMax,
			leftCharacter.manaRegen,
			leftCharacter.maxGuard,
			leftCharacter.guardCooldown,
			leftCharacter.odCd,
			leftCharacter.groundDrag,
			leftCharacter.airDrag,
			leftCharacter.gravity
		});
		this->_rightCharacter->init(*this, {
			false,
			rightCharacter.hp,
			rightCharacter.maxJumps,
			rightCharacter.maxAirDash,
			rightCharacter.matterManaMax,
			rightCharacter.voidManaMax,
			rightCharacter.spiritManaMax,
			rightCharacter.manaRegen,
			rightCharacter.maxGuard,
			rightCharacter.guardCooldown,
			rightCharacter.odCd,
			rightCharacter.groundDrag,
			rightCharacter.airDrag,
			rightCharacter.gravity
		});
		this->_leftCharacter->setAttacksDisabled(true);
		this->_rightCharacter->setAttacksDisabled(true);
		this->_roundSprites.resize(5 + FIRST_TO * 2 - 1);
		this->_cross.loadFromFile("assets/icons/netplay/twitter.png");
		this->_roundSprites[0].loadFromFile("assets/icons/rounds/ko.png");
		this->_roundSprites[1].loadFromFile("assets/icons/rounds/start.png");
		this->_roundSprites[2].loadFromFile("assets/icons/rounds/p1win.png");
		this->_roundSprites[3].loadFromFile("assets/icons/rounds/p2win.png");
		this->_roundSprites[4].loadFromFile("assets/icons/rounds/id.png");
		for (int i = 1; i < FIRST_TO * 2; i++)
			this->_roundSprites[4 + i].loadFromFile("assets/icons/rounds/round" + std::to_string(i) + ".png");
		this->_leftIcon.textureHandle = leftCharacter.icon;
		game->textureMgr.addRef(this->_leftIcon.textureHandle);
		this->_rightIcon.textureHandle = rightCharacter.icon;
		game->textureMgr.addRef(this->_rightIcon.textureHandle);
		this->_oosBubble.textureHandle = game->textureMgr.load("assets/effects/oosBubble.png");
		this->_oosBubbleMask.textureHandle = game->textureMgr.load("assets/effects/oosBubbleMask.png");
		game->textureMgr.setTexture(this->_oosBubbleMask);
		game->textureMgr.setTexture(this->_oosBubble);
		game->textureMgr.setTexture(this->_leftIcon);
		game->textureMgr.setTexture(this->_rightIcon);
		this->_font.loadFromFile("assets/battleui/AERO_03.ttf");

		auto texSize1 = game->textureMgr.getTextureSize(this->_leftIcon.textureHandle).to<float>();
		auto texSize2 = game->textureMgr.getTextureSize(this->_rightIcon.textureHandle).to<float>();
		auto texSize = game->textureMgr.getTextureSize(this->_oosBubble.textureHandle);
		auto s1 = texSize.x / texSize1.x;
		auto s2 = texSize.x / texSize2.x;

		texSize1.x = 0;
		texSize1.y = texSize.y - texSize1.y * s1;
		this->_leftIcon.setScale({s1, s1});
		this->_leftIcon.setPosition(texSize1);

		texSize2.x = 0;
		texSize2.y = texSize.y - texSize2.y * s2;
		this->_rightIcon.setScale({s2, s2});
		this->_rightIcon.setPosition(texSize2);

		for (int i = 0; i < BATTLEUI_NB_SPRITES; i++) {
			this->_battleUi[i].textureHandle = game->textureMgr.load(battleHudSprite[i], nullptr, true);
			game->textureMgr.setTexture(this->_battleUi[i]);
		}

		this->_battleUi[BATTLEUI_GUARD_TEXT].setOrigin({
			static_cast<float>(game->textureMgr.getTextureSize(this->_battleUi[BATTLEUI_GUARD_TEXT].textureHandle).x / 2),
			0,
		});
		this->_battleUi[BATTLEUI_OVERDRIVE].setOrigin({
			static_cast<float>(game->textureMgr.getTextureSize(this->_battleUi[BATTLEUI_OVERDRIVE].textureHandle).x / 2),
			static_cast<float>(game->textureMgr.getTextureSize(this->_battleUi[BATTLEUI_OVERDRIVE].textureHandle).y / 2),
		});
		this->_battleUi[BATTLEUI_OVERDRIVE_OUTLINE].setOrigin({
			static_cast<float>(game->textureMgr.getTextureSize(this->_battleUi[BATTLEUI_OVERDRIVE_OUTLINE].textureHandle).x / 2),
			static_cast<float>(game->textureMgr.getTextureSize(this->_battleUi[BATTLEUI_OVERDRIVE_OUTLINE].textureHandle).y / 2),
		});

		my_assert(this->_leftHUDData.target.create(texSize.x, texSize.y));
		my_assert(this->_rightHUDData.target.create(texSize.x, texSize.y));
		my_assert(this->_leftHUD.create(550, 700));
		my_assert(this->_rightHUD.create(550, 700));
		my_assert(this->_hud.create(1100, 700));
	}

	BattleManager::~BattleManager()
	{
		for (auto id : this->_moveSprites)
			game->textureMgr.remove(id);
	}

	void BattleManager::consumeEvent(const sf::Event &event)
	{
		this->_leftCharacter->consumeEvent(event);
		this->_rightCharacter->consumeEvent(event);
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F1)
			game->screen->setSize({1100, 700});
	}

	bool BattleManager::update()
	{
		return this->_updateLoop();
	}

	void BattleManager::render()
	{
		while (this->_fpsTimes.size() >= 15)
			this->_fpsTimes.pop_front();
		this->_fpsTimes.push_back(this->_fpsClock.restart().asMilliseconds());

		std::map<int, std::vector<IObject *>> objectLayers;
		float total = 0;

		for (auto &object : this->_stageObjects)
			objectLayers[object->getLayer()].push_back(&*object);
		for (auto &object : this->_platforms)
			objectLayers[object->getLayer()].push_back(&*object);
		for (auto &object : this->_objects)
			objectLayers[object.second->getLayer()].push_back(&*object.second);

		auto it = objectLayers.begin();

		// <= -1000, behind stage
		while (it != objectLayers.end() && it->first <= -1000) {
			for (auto obj : it->second)
				obj->render();
			it++;
		}
		game->textureMgr.render(this->_stage);
		// <= -500, behind HUD
		while (it != objectLayers.end() && it->first <= -500) {
			for (auto obj : it->second)
				obj->render();
			it++;
		}

		sf::Sprite sprite;

		this->_leftHUDData.render(this->_leftHUD);
		this->_leftHUD.display();
		this->_rightHUDData.render(this->_rightHUD);
		this->_rightHUD.display();

		this->_hud.clear(sf::Color::Transparent);
		sprite.setTexture(this->_leftHUD.getTexture(), true);
		sprite.setScale(1, 1);
		sprite.setPosition(0, 0);
		this->_hud.draw(sprite);
		sprite.setTexture(this->_rightHUD.getTexture(), true);
		sprite.setScale(-1, 1);
		sprite.setPosition(1100, 0);
		this->_hud.draw(sprite);

		this->_leftHUDData.renderNoReverse(this->_hud);
		this->_rightHUDData.renderNoReverse(this->_hud);

		this->_hud.display();
		sprite.setScale(1, 1);
		sprite.setPosition(-50, -600);
		sprite.setTexture(this->_hud.getTexture(), true);
		game->screen->draw(sprite);

		for (auto time : this->_tpsTimes)
			total += time;
		if (!this->_tpsTimes.empty()) {
			char buffer[12];

			sprintf(buffer, "%.2f TPS", 1000.f / (total / this->_tpsTimes.size()));
			game->screen->borderColor(2, sf::Color::Black);
			game->screen->fillColor(sf::Color::White);
			game->screen->textSize(20);
			game->screen->displayElement(buffer, {900, 75}, 145, Screen::ALIGN_RIGHT);
			game->screen->textSize(30);
			game->screen->borderColor(0, sf::Color::Transparent);
		}
		total = 0;
		for (auto time : this->_fpsTimes)
			total += time;
		if (!this->_fpsTimes.empty()) {
			char buffer[12];

			sprintf(buffer, "%.2f FPS", 1000.f / (total / this->_fpsTimes.size()));
			game->screen->borderColor(2, sf::Color::Black);
			game->screen->fillColor(sf::Color::White);
			game->screen->textSize(20);
			game->screen->displayElement(buffer, {900, 50}, 145, Screen::ALIGN_RIGHT);
			game->screen->textSize(30);
			game->screen->borderColor(0, sf::Color::Transparent);
		}

		// < -50, behind characters
		while (it != objectLayers.end() && it->first < -50) {
			for (auto obj : it->second)
				obj->render();
			it++;
		}
		if (this->_leftFirst) {
			this->_leftCharacter->render();
			// <= 0, On top of background character
			while (it != objectLayers.end() && it->first <= 50) {
				for (auto obj : it->second)
					obj->render();
				it++;
			}
		}
		this->_rightCharacter->render();
		if (!this->_leftFirst) {
			// <= 50, On top of background character
			while (it != objectLayers.end() && it->first <= 50) {
				for (auto obj: it->second)
					obj->render();
				it++;
			}
			this->_leftCharacter->render();
		}
		while (it != objectLayers.end()) {
			for (auto obj : it->second)
				obj->render();
			it++;
		}
		if (this->_roundEndTimer < 120 && (this->_leftCharacter->_hp <= 0 || this->_rightCharacter->_hp <= 0 || this->_roundEndTimer))
			this->_renderRoundEndAnimation();
		else if (this->_leftHUDData.score == FIRST_TO || this->_rightHUDData.score == FIRST_TO)
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
			game->soundMgr.play(BASICSOUND_KNOCK_OUT);
			this->_leftHUDData.score += this->_rightCharacter->_hp <= 0;
			this->_rightHUDData.score += this->_leftCharacter->_hp <= 0;
			this->_currentRound++;
			if (this->_leftHUDData.score == this->_rightHUDData.score && this->_leftHUDData.score == FIRST_TO) {
				this->_leftHUDData.score--;
				this->_rightHUDData.score--;
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
		game->screen->draw(this->_roundSprite);
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
		game->screen->draw(this->_roundSprite);
	}

	bool BattleManager::_updateEndGameAnimation()
	{
		float scale;
		float alpha;

		if (this->_roundStartTimer == -120) {
			this->_leftCharacter->disableInputs(true);
			this->_rightCharacter->disableInputs(true);
			if ((this->_leftCharacter->_hp > 0 ? this->_leftCharacter : this->_rightCharacter)->_action == 0)
				(this->_leftCharacter->_hp > 0 ? this->_leftCharacter : this->_rightCharacter)->onMatchEnd();
			else
				return true;
			this->_roundStartTimer++;
		} else
			this->_ended |= !(this->_leftCharacter->_hp > 0 ? this->_leftCharacter : this->_rightCharacter)->matchEndUpdate();

		if (!this->_ended) {
			this->_roundSprite.setScale({0, 0});
			return true;
		}

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

		this->_roundSprite.setTexture(this->_roundSprites[2 + (this->_rightHUDData.score == FIRST_TO)], true);
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
		game->screen->draw(this->_roundSprite);
	}

	void BattleManager::_gameUpdate()
	{
		std::vector<std::tuple<IObject *, IObject *, const FrameData *>> collisions;
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

		ldata = this->_leftCharacter->getCurrentFrameData();
		rdata = this->_rightCharacter->getCurrentFrameData();
		if (!ldata->dFlag.flash && !rdata->dFlag.flash) {
			for (auto &object: this->_objects)
				object.second->update();
			for (auto &object : this->_stageObjects)
				object->update();
			if (lchr->hits(*rchr))
				collisions.emplace_back(&*lchr, &*rchr, lchr->getCurrentFrameData());
			if (rchr->hits(*lchr))
				collisions.emplace_back(&*rchr, &*lchr, rchr->getCurrentFrameData());

			for (auto &platform: this->_platforms) {
				if (rchr->_comboCtr == 0) {
					if (lchr->hits(*platform))
						collisions.emplace_back(&*lchr, &*platform, lchr->getCurrentFrameData());
					if (platform->hits(*lchr))
						collisions.emplace_back(&*platform, &*lchr, platform->getCurrentFrameData());
				}
				if (lchr->_comboCtr == 0) {
					if (rchr->hits(*platform))
						collisions.emplace_back(&*rchr, &*platform, rchr->getCurrentFrameData());
					if (platform->hits(*rchr))
						collisions.emplace_back(&*platform, &*rchr, platform->getCurrentFrameData());
				}
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
			for (auto &object: this->_stageObjects) {
				if (lchr->hits(*object))
					collisions.emplace_back(&*lchr, &*object, lchr->getCurrentFrameData());
				if (object->hits(*lchr))
					collisions.emplace_back(&*object, &*lchr, object->getCurrentFrameData());

				if (rchr->hits(*object))
					collisions.emplace_back(&*rchr, &*object, rchr->getCurrentFrameData());
				if (object->hits(*rchr))
					collisions.emplace_back(&*object, &*rchr, object->getCurrentFrameData());

				for (auto &object2: this->_objects)
					if (object->hits(*object2.second))
						collisions.emplace_back(&*object, &*object2.second, object->getCurrentFrameData());
			}

			for (auto &[attacker, defender, data]: collisions) {
				defender->getHit(*attacker, data);
				attacker->hit(*defender, data);
			}
		}

		my_assert(!lchr->isDead());
		my_assert(!rchr->isDead());

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
		if (this->_leftCharacter->startedAttack)
			this->_leftFirst = false;
		else if (this->_rightCharacter->startedAttack)
			this->_leftFirst = true;
		this->_leftCharacter->startedAttack = false;
		this->_rightCharacter->startedAttack = false;
		this->_leftHUDData.update();
		this->_rightHUDData.update();
	}

	bool BattleManager::isLeftFirst() const
	{
		return this->_leftFirst;
	}

	std::shared_ptr<IObject> BattleManager::getObjectFromId(unsigned int id) const
	{
		for (auto &object : this->_objects)
			if (object.first == id)
				return object.second;
		my_assert(false);
		return nullptr;
	}

	unsigned int BattleManager::getBufferSize() const
	{
		size_t size = sizeof(Data) + this->_leftCharacter->getBufferSize() + this->_rightCharacter->getBufferSize();

		size += this->_objects.size() * (sizeof(unsigned) + sizeof(unsigned char));
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
		char *ptr = (char *)data + sizeof(Data);

		game->logger.verbose("Saving BattleManager (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
		dat->random = game->battleRandom.ser.invoke_count;
		dat->_ended = this->_ended;
		dat->_lastObjectId = this->_lastObjectId;
		dat->_leftHUDData = this->_leftHUDData;
		dat->_rightHUDData = this->_rightHUDData;
		dat->_currentRound = this->_currentRound;
		dat->_roundStartTimer = this->_roundStartTimer;
		dat->_roundEndTimer = this->_roundEndTimer;
		dat->_nbObjects = this->_objects.size();
		dat->_currentFrame = this->_currentFrame;
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
				my_assert(dynamic_cast<SubObject *>(&*object.second));

				auto obj = reinterpret_cast<SubObject *>(&*object.second);

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
		//TODO: Also save the stage objects. The clouds call the random number generator so it's a must
	}

	void BattleManager::restoreFromBuffer(void *data)
	{
		auto dat = reinterpret_cast<Data *>(data);
		char *ptr = (char *)data + sizeof(Data);

		if (dat->random != game->battleRandom.ser.invoke_count)
			game->battleRandom.rollback(dat->random);
		this->_ended = dat->_ended;
		this->_currentFrame = dat->_currentFrame;
		this->_lastObjectId = dat->_lastObjectId;
		this->_leftHUDData = dat->_leftHUDData;
		this->_rightHUDData = dat->_rightHUDData;
		this->_currentRound = dat->_currentRound;
		this->_roundStartTimer = dat->_roundStartTimer;
		this->_roundEndTimer = dat->_roundEndTimer;
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
				obj = (owner ? this->_rightCharacter : this->_leftCharacter)->_spawnSubObject(*this, subobjid, false).second;
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
		game->logger.verbose("Restored BattleManager @" + std::to_string((uintptr_t)dat));
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

		this->_currentFrame++;
		if (this->_roundEndTimer <= 120 && (this->_leftCharacter->_hp <= 0 || this->_rightCharacter->_hp <= 0 || this->_roundEndTimer))
			this->_updateRoundEndAnimation();
		else if (this->_leftHUDData.score == FIRST_TO || this->_rightHUDData.score == FIRST_TO) {
			if (!this->_updateEndGameAnimation())
				return false;
		} else if (this->_roundStartTimer < 140)
			this->_updateRoundStartAnimation();
		return true;
	}

	const std::vector<std::shared_ptr<Platform>> &BattleManager::getPlatforms() const
	{
		return this->_platforms;
	}

	const std::vector<Character::ReplayData> &BattleManager::getLeftReplayData() const
	{
		return this->_leftCharacter->getReplayData();
	}

	const std::vector<Character::ReplayData> &BattleManager::getRightReplayData() const
	{
		return this->_rightCharacter->getReplayData();
	}

	void BattleManager::renderInputs()
	{
		this->renderLeftInputs();
		this->renderRightInputs();
	}

	void BattleManager::renderLeftInputs()
	{
		this->_renderInputs(this->_leftCharacter->getReplayData(), {-50, -495}, false);
	}

	void BattleManager::renderRightInputs()
	{
		this->_renderInputs(this->_rightCharacter->getReplayData(), {900, -495}, true);
	}

	void BattleManager::_renderButton(unsigned spriteId, float offset, int k, Vector2f pos)
	{
		Sprite sprite;

		sprite.textureHandle = this->_moveSprites[spriteId];
		sprite.setScale({
			(INPUT_DISPLAY_SIZE - 4.f) / game->textureMgr.getTextureSize(this->_moveSprites[spriteId]).x,
			(INPUT_DISPLAY_SIZE - 4.f) / game->textureMgr.getTextureSize(this->_moveSprites[spriteId]).y
		});
		game->textureMgr.setTexture(sprite);
		game->screen->displayElement(sprite, {
			2 + pos.x + 4 + offset,
			2 + pos.y + k * (INPUT_DISPLAY_SIZE + 4)
		});
	}

	void BattleManager::_renderInputs(const std::vector<Character::ReplayData> &data, Vector2f pos, bool side)
	{
		Sprite sprite;
		sf::Sprite s;
		unsigned total = 0;
		sf::RectangleShape shape;
		float off = 0;

		this->_tex.clear(sf::Color::Transparent);
		this->_tex.create(150, INPUT_DISPLAY_SIZE);
		shape.setOutlineThickness(0);
		shape.setSize({INPUT_DISPLAY_SIZE, INPUT_DISPLAY_SIZE});
		shape.setFillColor(sf::Color{0, 0, 0, 0xA0});
		this->_tex.draw(shape);
		shape.setPosition(INPUT_DISPLAY_SIZE, 0);
		shape.setSize({150 - INPUT_DISPLAY_SIZE, INPUT_DISPLAY_SIZE});
		shape.setFillColor(sf::Color{0xA0, 0xA0, 0xA0, 0xA0});
		this->_tex.draw(shape);
		this->_tex.display();

		s.setTexture(this->_tex.getTexture());
		if (!side) {
			s.setScale({-1, 1});
			off = 150;
		}
		for (unsigned k = 0; k < 18; k++) {
			s.setPosition(pos.x + off, pos.y + k * (INPUT_DISPLAY_SIZE + 4));
			game->screen->draw(s);
		}
		if (!side)
			off -= INPUT_DISPLAY_SIZE;

		for (size_t i = 0, k = 0; i < data.size() && k < 18; i++) {
			auto &elem = data[data.size() - 1 - i];

			total += elem.time + 1;
			if (i < data.size() - 1) {
				auto &elem2 = data[data.size() - 2 - i];

				if (
					elem._h == elem2._h &&
					elem._v == elem2._v &&
					elem.n == elem2.n &&
					elem.m == elem2.m &&
					elem.v == elem2.v &&
					elem.s == elem2.s &&
					elem.a == elem2.a &&
					elem.d == elem2.d
				)
					continue;
			}

			int dir = ((elem._h + 2) + (elem._v + 1) * 3);
			int spriteId = SPRITE_1 + dir - 1;
			float offset = off;


			game->textureMgr.setTexture(sprite);
			if (dir != 5) {
				if (spriteId > SPRITE_4)
					spriteId -= 1;
				sprite.textureHandle = this->_moveSprites[spriteId];
				sprite.setScale({
					(INPUT_DISPLAY_SIZE - 4.f) / game->textureMgr.getTextureSize(this->_moveSprites[spriteId]).x,
					(INPUT_DISPLAY_SIZE - 4.f) / game->textureMgr.getTextureSize(this->_moveSprites[spriteId]).y
				});
				game->textureMgr.setTexture(sprite);
				game->screen->displayElement(sprite, {
					pos.x + 2 + offset,
					pos.y + 2 + k * (INPUT_DISPLAY_SIZE + 4)
				});
			}
			offset -= (side ? -INPUT_DISPLAY_SIZE : INPUT_DISPLAY_SIZE) + 4;

			if (elem.n) {
				this->_renderButton(SPRITE_N, offset, k, pos);
				offset -= (side ? -INPUT_DISPLAY_SIZE : INPUT_DISPLAY_SIZE) - 3;
			}
			if (elem.m || elem.s || elem.v) {
				if (elem.m) {
					this->_renderButton(SPRITE_M, offset, k, pos);
					offset -= (side ? -INPUT_DISPLAY_SIZE : INPUT_DISPLAY_SIZE) * 0.4;
				}
				if (elem.s) {
					this->_renderButton(SPRITE_S, offset, k, pos);
					offset -= (side ? -INPUT_DISPLAY_SIZE : INPUT_DISPLAY_SIZE) * 0.4;
				}
				if (elem.v) {
					this->_renderButton(SPRITE_V, offset, k, pos);
					offset -= (side ? -INPUT_DISPLAY_SIZE : INPUT_DISPLAY_SIZE) * 0.4;
				}
				offset -= (side ? -INPUT_DISPLAY_SIZE : INPUT_DISPLAY_SIZE) * 0.6 - 3;
			}
			if (elem.a) {
				this->_renderButton(SPRITE_A, offset, k, pos);
				offset -= (side ? -INPUT_DISPLAY_SIZE : INPUT_DISPLAY_SIZE) - 3;
			}
			if (elem.d) {
				this->_renderButton(SPRITE_D, offset, k, pos);
				offset -= (side ? -INPUT_DISPLAY_SIZE : INPUT_DISPLAY_SIZE) - 3;
			}
			game->screen->textSize(12);
			game->screen->setFont(this->_font);
			game->screen->displayElement(std::to_string(total), {pos.x + 2, pos.y + k * (INPUT_DISPLAY_SIZE + 4) + 5}, 146, (!side ? Screen::ALIGN_LEFT : Screen::ALIGN_RIGHT));
			game->screen->setFont(game->font);
			game->screen->textSize(30);
			k++;
			total = 0;
		}
	}

	void BattleManager::logDifference(void *data1, void *data2)
	{
		auto dat1 = reinterpret_cast<Data *>(data1);
		auto dat2 = reinterpret_cast<Data *>(data2);
		char *ptr1 = (char *)data1 + sizeof(Data);
		char *ptr2 = (char *)data2 + sizeof(Data);

		if (dat1->random != dat2->random)
			game->logger.fatal("BattleManager::random differs: " + std::to_string(dat1->random) + " vs " + std::to_string(dat2->random));
		if (dat1->_currentFrame != dat2->_currentFrame)
			game->logger.fatal("BattleManager::_currentFrame differs: " + std::to_string(dat1->_currentFrame) + " vs " + std::to_string(dat2->_currentFrame));
		if (dat1->_ended != dat2->_ended)
			game->logger.fatal("BattleManager::ended differs: " + std::to_string(dat1->_ended) + " vs " + std::to_string(dat2->_ended));
		if (dat1->_lastObjectId != dat2->_lastObjectId)
			game->logger.fatal("BattleManager::lastObjectId differs: " + std::to_string(dat1->_lastObjectId) + " vs " + std::to_string(dat2->_lastObjectId));
		if (dat1->_leftHUDData.comboCtr != dat2->_leftHUDData.comboCtr)
			game->logger.fatal("BattleManager::leftHUDData.comboCtr differs: " + std::to_string(dat1->_leftHUDData.comboCtr) + " vs " + std::to_string(dat2->_leftHUDData.comboCtr));
		if (dat1->_leftHUDData.hitCtr != dat2->_leftHUDData.hitCtr)
			game->logger.fatal("BattleManager::leftHUDData.hitCtr differs: " + std::to_string(dat1->_leftHUDData.hitCtr) + " vs " + std::to_string(dat2->_leftHUDData.hitCtr));
		if (dat1->_leftHUDData.neutralLimit != dat2->_leftHUDData.neutralLimit)
			game->logger.fatal("BattleManager::leftHUDData.neutralLimit differs: " + std::to_string(dat1->_leftHUDData.neutralLimit) + " vs " + std::to_string(dat2->_leftHUDData.neutralLimit));
		if (dat1->_leftHUDData.voidLimit != dat2->_leftHUDData.voidLimit)
			game->logger.fatal("BattleManager::leftHUDData.voidLimit differs: " + std::to_string(dat1->_leftHUDData.voidLimit) + " vs " + std::to_string(dat2->_leftHUDData.voidLimit));
		if (dat1->_leftHUDData.matterLimit != dat2->_leftHUDData.matterLimit)
			game->logger.fatal("BattleManager::leftHUDData.matterLimit differs: " + std::to_string(dat1->_leftHUDData.matterLimit) + " vs " + std::to_string(dat2->_leftHUDData.matterLimit));
		if (dat1->_leftHUDData.spiritLimit != dat2->_leftHUDData.spiritLimit)
			game->logger.fatal("BattleManager::leftHUDData.spiritLimit differs: " + std::to_string(dat1->_leftHUDData.spiritLimit) + " vs " + std::to_string(dat2->_leftHUDData.spiritLimit));
		if (dat1->_leftHUDData.totalDamage != dat2->_leftHUDData.totalDamage)
			game->logger.fatal("BattleManager::leftHUDData.totalDamage differs: " + std::to_string(dat1->_leftHUDData.totalDamage) + " vs " + std::to_string(dat2->_leftHUDData.totalDamage));
		if (dat1->_leftHUDData.proration != dat2->_leftHUDData.proration)
			game->logger.fatal("BattleManager::leftHUDData.proration differs: " + std::to_string(dat1->_leftHUDData.proration) + " vs " + std::to_string(dat2->_leftHUDData.proration));
		if (dat1->_leftHUDData.score != dat2->_leftHUDData.score)
			game->logger.fatal("BattleManager::_leftHUDData.proration differs: " + std::to_string(dat1->_leftHUDData.score) + " vs " + std::to_string(dat2->_leftHUDData.score));
		if (dat1->_rightHUDData.comboCtr != dat2->_rightHUDData.comboCtr)
			game->logger.fatal("BattleManager::rightHUDData.comboCtr differs: " + std::to_string(dat1->_rightHUDData.comboCtr) + " vs " + std::to_string(dat2->_rightHUDData.comboCtr));
		if (dat1->_rightHUDData.hitCtr != dat2->_rightHUDData.hitCtr)
			game->logger.fatal("BattleManager::rightHUDData.hitCtr differs: " + std::to_string(dat1->_rightHUDData.hitCtr) + " vs " + std::to_string(dat2->_rightHUDData.hitCtr));
		if (dat1->_rightHUDData.neutralLimit != dat2->_rightHUDData.neutralLimit)
			game->logger.fatal("BattleManager::rightHUDData.neutralLimit differs: " + std::to_string(dat1->_rightHUDData.neutralLimit) + " vs " + std::to_string(dat2->_rightHUDData.neutralLimit));
		if (dat1->_rightHUDData.voidLimit != dat2->_rightHUDData.voidLimit)
			game->logger.fatal("BattleManager::rightHUDData.voidLimit differs: " + std::to_string(dat1->_rightHUDData.voidLimit) + " vs " + std::to_string(dat2->_rightHUDData.voidLimit));
		if (dat1->_rightHUDData.spiritLimit != dat2->_rightHUDData.spiritLimit)
			game->logger.fatal("BattleManager::rightHUDData.spiritLimit differs: " + std::to_string(dat1->_rightHUDData.spiritLimit) + " vs " + std::to_string(dat2->_rightHUDData.spiritLimit));
		if (dat1->_rightHUDData.matterLimit != dat2->_rightHUDData.matterLimit)
			game->logger.fatal("BattleManager::rightHUDData.matterLimit differs: " + std::to_string(dat1->_rightHUDData.matterLimit) + " vs " + std::to_string(dat2->_rightHUDData.matterLimit));
		if (dat1->_rightHUDData.totalDamage != dat2->_rightHUDData.totalDamage)
			game->logger.fatal("BattleManager::rightHUDData.totalDamage differs: " + std::to_string(dat1->_rightHUDData.totalDamage) + " vs " + std::to_string(dat2->_rightHUDData.totalDamage));
		if (dat1->_rightHUDData.proration != dat2->_rightHUDData.proration)
			game->logger.fatal("BattleManager::rightHUDData.proration differs: " + std::to_string(dat1->_rightHUDData.proration) + " vs " + std::to_string(dat2->_rightHUDData.proration));
		if (dat1->_rightHUDData.score != dat2->_rightHUDData.score)
			game->logger.fatal("BattleManager::rightHUDData.score differs: " + std::to_string(dat1->_rightHUDData.score) + " vs " + std::to_string(dat2->_rightHUDData.score));
		if (dat1->_currentRound != dat2->_currentRound)
			game->logger.fatal("BattleManager::currentRound differs: " + std::to_string(dat1->_currentRound) + " vs " + std::to_string(dat2->_currentRound));
		if (dat1->_roundStartTimer != dat2->_roundStartTimer)
			game->logger.fatal("BattleManager::roundStartTimer differs: " + std::to_string(dat1->_roundStartTimer) + " vs " + std::to_string(dat2->_roundStartTimer));
		if (dat1->_roundEndTimer != dat2->_roundEndTimer)
			game->logger.fatal("BattleManager::roundEndTimer differs: " + std::to_string(dat1->_roundEndTimer) + " vs " + std::to_string(dat2->_roundEndTimer));
		if (dat1->_nbObjects != dat2->_nbObjects)
			game->logger.fatal("BattleManager::nbObjects differs: " + std::to_string(dat1->_nbObjects) + " vs " + std::to_string(dat2->_nbObjects));

		auto length = this->_leftCharacter->printDifference("Player1: ", (void *)ptr1, (void *)ptr2);

		if (!length)
			return;
		ptr1 += length;
		ptr2 += length;

		length = this->_rightCharacter->printDifference("Player2: ", (void *)ptr1, (void *)ptr2);
		if (!length)
			return;
		ptr1 += length;
		ptr2 += length;

		if (dat1->_nbObjects != dat2->_nbObjects)
			return;

		for (size_t i = 0; i < dat1->_nbObjects; i++) {
			std::shared_ptr<IObject> obj;
			auto id1 = *(unsigned *)ptr1;
			auto id2 = *(unsigned *)ptr2;

			if (id1 != id2)
				game->logger.fatal("BattleManager::object[" + std::to_string(i) + "]::objectId differs: " + std::to_string(id1) + " vs " + std::to_string(id2));
			ptr1 += sizeof(unsigned);
			ptr2 += sizeof(unsigned);

			auto cl1 = *(unsigned char *)ptr1;
			auto cl2 = *(unsigned char *)ptr2;

			if (cl1 != cl2) {
				game->logger.fatal("BattleManager::object[" + std::to_string(i) + "]::class differs: " + std::to_string(cl1) + " vs " + std::to_string(cl2));
				return;
			}
			ptr1 += sizeof(unsigned char);
			ptr2 += sizeof(unsigned char);

			switch (cl1) {
			case 0:
				obj.reset(new Object());
				break;
			case 1:
				obj.reset(new Character());
				break;
			case 2: {
				auto owner1 = *(bool *)ptr1;
				auto owner2 = *(bool *)ptr2;

				if (owner1 != owner2)
					game->logger.fatal("BattleManager::object[" + std::to_string(i) + "]::owner differs: " + std::to_string(owner1) + " vs " + std::to_string(owner2));
				ptr1 += sizeof(bool);
				ptr2 += sizeof(bool);

				auto subobjid1 = *(unsigned *)ptr1;
				auto subobjid2 = *(unsigned *)ptr2;

				if (subobjid1 != subobjid2)
					game->logger.fatal("BattleManager::object[" + std::to_string(i) + "]::subobjectId differs: " + std::to_string(subobjid1) + " vs " + std::to_string(subobjid2));
				ptr1 += sizeof(unsigned);
				ptr2 += sizeof(unsigned);
				obj = (owner1 ? this->_rightCharacter : this->_leftCharacter)->_spawnSubObject(*this,subobjid1, false).second;
				break;
			}
			default:
				game->logger.fatal("BattleManager::object[" + std::to_string(i) + "]::class invalid: " + std::to_string(cl1));
				return;
			}

			length = obj->printDifference(("BattleManager::object[" + std::to_string(i) + "]: ").c_str(), (void *)ptr1, (void *)ptr2);
			if (length == 0)
				return;
			ptr1 += length;
			ptr2 += length;
		}
		for (size_t i = 0; i < this->_nbPlatform; i++) {
			length = this->_platforms[i]->printDifference(("BattleManager::platform[" + std::to_string(i) + "]: ").c_str(), (void *)ptr1, (void *)ptr2);
			if (length == 0)
				return;
			ptr1 += length;
			ptr2 += length;
		}
	}

	unsigned BattleManager::getCurrentFrame() const
	{
		return this->_currentFrame;
	}

	static float getTextSize(const std::string &str, const sf::Text &txt)
	{
		float size = 0;

		for (char c : str)
			size += txt.getFont()->getGlyph(c, txt.getCharacterSize(), false).advance;
		return size;
	}

	static Vector2f getPos(Vector2f basePos, unsigned size, bool side)
	{
		if (!side)
			return basePos;
		return {
			1100 - basePos.x - size,
			basePos.y
		};
	}

	static Vector2f getPos(const std::string &str, const sf::Text &txt, Vector2f basePos, bool side)
	{
		if (!side)
			return basePos;
		return {
			1100 - basePos.x - getTextSize(str, txt),
			basePos.y
		};
	}

	static void renderText(sf::RenderTarget &output, const std::string &str, sf::Text &txt, Vector2f basePos, bool side)
	{
		txt.setPosition(getPos(str, txt, basePos, side));
		txt.setString(str);
		output.draw(txt);
	}

	BattleManager::HUDData::HUDData(BattleManager &mgr, Character &base, bool side) :
		mgr(mgr),
		base(base),
		side(side)
	{
	}

	void BattleManager::HUDData::renderMeterBar(sf::RenderTarget &output, Vector2i pos, float bar, sf::Color minColor, sf::Color maxColor) const
	{
		sf::VertexArray buffer{sf::TriangleStrip, 4};
		sf::Vertex vertex;
		sf::Text text;

		vertex.color = minColor;
		vertex.position = pos;
		buffer[0] = vertex;
		vertex.position = {16.f + pos.x, 16.f + pos.y};
		buffer[2] = vertex;

		vertex.color = sf::Color{
			static_cast<sf::Uint8>(minColor.r + (maxColor.r - minColor.r) * bar),
			static_cast<sf::Uint8>(minColor.g + (maxColor.g - minColor.g) * bar),
			static_cast<sf::Uint8>(minColor.b + (maxColor.b - minColor.b) * bar),
		};
		vertex.position = {pos.x + bar * 169, pos.y * 1.f};
		buffer[1] = vertex;
		vertex.position = {16 + pos.x + bar * 169, 16.f + pos.y};
		buffer[3] = vertex;
		output.draw(buffer);

		auto str = std::to_string((int)(bar * 100));

		text.setFont(this->mgr._font);
		text.setFillColor(vertex.color);
		text.setOutlineThickness(0);
		text.setCharacterSize(14);
		if (this->side) {
			size_t size = 0;

			for (auto c : str)
				size += this->mgr._font.getGlyph(c, 14, false).advance;
			text.setScale(-1, 1);
			text.setPosition(pos.x + 190 + size, pos.y - 1);
		} else
			text.setPosition(pos.x + 190, pos.y - 1);
		text.setString(str);
		output.draw(text);
	}

	void BattleManager::HUDData::render(sf::RenderTarget &output) const
	{
		output.clear(sf::Color::Transparent);

		this->mgr._battleUi[BATTLEUI_HUD_SEAT].setPosition(20, 20);
		output.draw(this->mgr._battleUi[BATTLEUI_HUD_SEAT], sf::BlendNone);

		this->mgr._battleUi[BATTLEUI_LIFE_BAR_RED].setPosition(69, 40);
		this->mgr._battleUi[BATTLEUI_LIFE_BAR_RED].setTextureRect({
			0, 0,
			static_cast<int>(game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_LIFE_BAR_RED].textureHandle).x * std::min<float>(
				this->base._hp + static_cast<float>(this->base._totalDamage), this->base._baseHp
			) / this->base._baseHp),
			static_cast<int>(game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_LIFE_BAR_RED].textureHandle).y)
		});
		output.draw(this->mgr._battleUi[BATTLEUI_LIFE_BAR_RED], sf::BlendAlpha);

		this->mgr._battleUi[BATTLEUI_LIFE_BAR].setPosition(69, 40);
		this->mgr._battleUi[BATTLEUI_LIFE_BAR].setTextureRect({
			0, 0,
			static_cast<int>(game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_LIFE_BAR].textureHandle).x * this->base._hp / this->base._baseHp),
			static_cast<int>(game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_LIFE_BAR].textureHandle).y)
		});
		output.draw(this->mgr._battleUi[BATTLEUI_LIFE_BAR], sf::BlendAlpha);

		this->mgr._battleUi[BATTLEUI_LIFE_BAR_EFFECT].setPosition(69, 40);
		this->mgr._battleUi[BATTLEUI_LIFE_BAR_EFFECT].setTextureRect({
			static_cast<int>(this->lifeBarEffect), 0,
			static_cast<int>(game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_LIFE_BAR].textureHandle).x * this->base._hp / this->base._baseHp),
			static_cast<int>(game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_LIFE_BAR].textureHandle).y)
		});
		output.draw(this->mgr._battleUi[BATTLEUI_LIFE_BAR_EFFECT], sf::BlendMode{
			sf::BlendMode::DstColor, sf::BlendMode::Zero, sf::BlendMode::Add,
			sf::BlendMode::Zero,     sf::BlendMode::One,  sf::BlendMode::Add
		});

		auto guardVals = this->base._guardCooldown ?
		                 std::pair<int, int>(this->base._maxGuardCooldown - this->base._guardCooldown, this->base._maxGuardCooldown) :
		                 std::pair<int, int>(this->base._guardBar, this->base._maxGuardBar);
		auto guardId = this->base._guardCooldown ? BATTLEUI_GUARD_BAR_DISABLED : BATTLEUI_GUARD_BAR;

		this->mgr._battleUi[guardId].setPosition(260, 78);
		this->mgr._battleUi[guardId].setTextureRect({
			0, 0,
			static_cast<int>(game->textureMgr.getTextureSize(this->mgr._battleUi[guardId].textureHandle).x * guardVals.first / guardVals.second),
			static_cast<int>(game->textureMgr.getTextureSize(this->mgr._battleUi[guardId].textureHandle).y)
		});
		output.draw(this->mgr._battleUi[guardId], sf::BlendNone);

		this->mgr._battleUi[BATTLEUI_GUARD_TEXT].setScale(this->side ? -1 : 1, 1);
		this->mgr._battleUi[BATTLEUI_GUARD_TEXT].setPosition(222 + game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_GUARD_TEXT].textureHandle).x / 2, 62);
		this->mgr._battleUi[BATTLEUI_GUARD_TEXT].setColor(this->base._guardCooldown ? sf::Color{0x40, 0x40, 0x40} : sf::Color::White);
		output.draw(this->mgr._battleUi[BATTLEUI_GUARD_TEXT], sf::BlendAlpha);

		if (this->base._guardCooldown && this->guardCrossTimer % 60 > 30) {
			sf::Sprite sprite;

			sprite.setTexture(this->mgr._cross, true);
			sprite.setPosition(237, 65);
			output.draw(sprite, sf::BlendAlpha);
		}

		auto size = game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_OVERDRIVE].textureHandle);

		this->mgr._battleUi[BATTLEUI_OVERDRIVE].setScale(this->side ? -1 : 1, 1);
		this->mgr._battleUi[BATTLEUI_OVERDRIVE].setPosition(
			420 + game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_OVERDRIVE].textureHandle).x / 2,
			65 + game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_OVERDRIVE].textureHandle).y / 2
		);
		this->mgr._battleUi[BATTLEUI_OVERDRIVE].setTextureRect({
			0, 0,
			static_cast<int>(size.x),
			static_cast<int>(size.y)
		});
		this->mgr._battleUi[BATTLEUI_OVERDRIVE].setColor(this->base._odCooldown ? sf::Color{0x40, 0x40, 0x40} : sf::Color::White);
		output.draw(this->mgr._battleUi[BATTLEUI_OVERDRIVE], sf::BlendAlpha);

		if (this->base._odCooldown) {
			this->mgr._battleUi[BATTLEUI_OVERDRIVE].setScale(this->side ? -1 : 1, 1);
			this->mgr._battleUi[BATTLEUI_OVERDRIVE].setPosition(
				420 + size.x / 2 + (this->side ? 0 : size.x * this->base._odCooldown / this->base._barMaxOdCooldown),
				65 + size.y / 2
			);
			this->mgr._battleUi[BATTLEUI_OVERDRIVE].setTextureRect({
				(this->side ? 0 : static_cast<int>(size.x * this->base._odCooldown / this->base._barMaxOdCooldown)),
				0,
				static_cast<int>(size.x - size.x * this->base._odCooldown / this->base._barMaxOdCooldown),
				static_cast<int>(size.y)
			});
			this->mgr._battleUi[BATTLEUI_OVERDRIVE].setColor(sf::Color{0xFF, 0x80, 0x80});
			output.draw(this->mgr._battleUi[BATTLEUI_OVERDRIVE], sf::BlendAlpha);
		}

		this->mgr._battleUi[BATTLEUI_OVERDRIVE_OUTLINE].setScale(this->side ? -1 : 1, 1);
		this->mgr._battleUi[BATTLEUI_OVERDRIVE_OUTLINE].setPosition(
			420 + game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_OVERDRIVE_OUTLINE].textureHandle).x / 2,
			65 + game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_OVERDRIVE_OUTLINE].textureHandle).y / 2
		);
		output.draw(this->mgr._battleUi[BATTLEUI_OVERDRIVE_OUTLINE], sf::BlendAlpha);

		if (this->base._odCooldown && this->overdriveCrossTimer % 60 > 30) {
			sf::Sprite sprite;

			sprite.setScale(2, 2);
			sprite.setOrigin(8, 8);
			sprite.setTexture(this->mgr._cross, true);
			sprite.setPosition(420 + size.x / 2, 65 + size.y / 2);
			output.draw(sprite, sf::BlendAlpha);
		}
		for (int i = 0; i < FIRST_TO; i++) {
			this->mgr._battleUi[BATTLEUI_SCORE_SEAT].setPosition(162 - i * 46, 69);
			output.draw(this->mgr._battleUi[BATTLEUI_SCORE_SEAT], sf::BlendNone);
		}
		for (int i = 0; i < this->score; i++) {
			this->mgr._battleUi[BATTLEUI_SCORE_BULLET].setPosition(167 - i * 46, 72);
			output.draw(this->mgr._battleUi[BATTLEUI_SCORE_BULLET], sf::BlendNone);
		}

		this->mgr._battleUi[BATTLEUI_MANA_BAR].setPosition(20, 611);
		output.draw(this->mgr._battleUi[BATTLEUI_MANA_BAR], sf::BlendNone);

		this->renderMeterBar(output, {24,  616}, (float)this->base._spiritMana / this->base._spiritManaMax, {0,   162, 195}, {45, 219, 255});
		this->renderMeterBar(output, {79,  638}, (float)this->base._matterMana / this->base._matterManaMax, {184, 92,  0},   {255, 156, 56});
		this->renderMeterBar(output, {134, 660}, (float)this->base._voidMana   / this->base._voidManaMax,   {158, 0,   158}, {255, 63, 255});
	}

	void BattleManager::HUDData::renderNoReverse(sf::RenderTarget &output) const
	{
		sf::Text text;

		text.setFont(this->mgr._font);
		if (this->comboCtr) {
			unsigned char alpha = this->comboCtr > 51 ? 0xFF : this->comboCtr * 5;

			text.setCharacterSize(40);
			text.setOutlineColor(sf::Color{0, 0, 0, alpha});
			text.setOutlineThickness(2);
			text.setFillColor(sf::Color{0xFF, 0x00, 0x00, alpha});
			renderText(output, std::to_string(this->hitCtr) + " Hit" + (this->hitCtr < 2 ? "" : "s"), text, {50, 80}, this->side);
			text.setCharacterSize(30);
			text.setFillColor(sf::Color{0xA0, 0xA0, 0xA0, alpha});
			renderText(output, std::to_string(this->totalDamage) + " damage", text, {50, 130}, this->side);
			renderText(output, std::to_string(static_cast<int>(this->proration * 100)) + "% proration", text, {50, 160}, this->side);
			text.setCharacterSize(20);
			text.setFillColor(sf::Color{0xFF, 0xFF, 0xFF, alpha});
			renderText(output, "Neutral Limit: " + std::to_string(this->neutralLimit), text, {50, 200}, this->side);
			text.setFillColor(sf::Color{0x80, 0x00, 0x80, alpha});
			renderText(output, "Void Limit: " + std::to_string(this->voidLimit), text, {50, 225}, this->side);
			text.setFillColor(sf::Color{187, 94, 0, alpha});
			renderText(output, "Matter Limit: " + std::to_string(this->matterLimit), text, {50, 250}, this->side);
			text.setFillColor(sf::Color{51, 204, 204, alpha});
			renderText(output, "Spirit Limit: " + std::to_string(this->spiritLimit), text, {50, 275}, this->side);
			if (this->counter) {
				text.setCharacterSize(25);
				text.setFillColor(sf::Color{0xFF, 0x40, 0x20, alpha});
				renderText(output, "Counter !", text, {50, 300}, this->side);
			}
		}
		if (this->base._position.y > 540) {
			this->target.clear(sf::Color::Transparent);
			this->target.draw(this->mgr._oosBubbleMask, sf::BlendNone);
			this->target.draw(side ? this->mgr._rightIcon : this->mgr._leftIcon, sf::BlendMode{
				sf::BlendMode::SrcColor,
				sf::BlendMode::Zero,
				sf::BlendMode::Add,
				sf::BlendMode::Zero,
				sf::BlendMode::DstColor,
				sf::BlendMode::Add
			});
			this->target.draw(this->mgr._oosBubble);
			this->target.display();

			sf::Sprite sprite(this->target.getTexture());
			auto pos = this->base._position;

			if (!this->base._direction) {
				sprite.setScale(-1, 1);
				pos.x += this->target.getSize().x / 2;
			} else
				pos.x -= this->target.getSize().x / 2;
			pos.y = std::max(-pos.y, -540.f);
			pos.x += 50;
			pos.y += 600;
			sprite.setPosition(pos);
			output.draw(sprite);
		}
	}

	void BattleManager::HUDData::update()
	{
		if (this->base._guardCooldown)
			this->guardCrossTimer++;
		if (this->base._odCooldown)
			this->overdriveCrossTimer++;
		if (this->comboCtr)
			this->comboCtr--;
		this->lifeBarEffect++;
		this->lifeBarEffect %= game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_LIFE_BAR_EFFECT].textureHandle).x;
		if (this->base._opponent->_comboCtr) {
			auto superRate = this->base._opponent->_supersUsed >= 2 ? std::min(1.f, std::max(0.f, (100.f - (10 << (this->base._opponent->_supersUsed - 2))) / 100.f)) : 1;
			auto skillRate = this->base._opponent->_skillsUsed >= 2 ? std::min(1.f, std::max(0.f, (100.f - (3 << (this->base._opponent->_skillsUsed - 2))) / 100.f)) : 1;

			this->hitCtr       = this->base._opponent->_comboCtr;
			this->neutralLimit = this->base._opponent->_limit[0];
			this->voidLimit    = this->base._opponent->_limit[1];
			this->matterLimit  = this->base._opponent->_limit[2];
			this->spiritLimit  = this->base._opponent->_limit[3];
			this->totalDamage  = this->base._opponent->_totalDamage;
			this->proration    = this->base._opponent->_prorate * superRate * skillRate;
			this->counter      = this->base._opponent->_counter;
			this->comboCtr     = 120;
		}
	}

	BattleManager::HUDData &BattleManager::HUDData::operator=(BattleManager::HUDDataPacked &data)
	{
		this->comboCtr = data.comboCtr;
		this->hitCtr = data.hitCtr;
		this->neutralLimit = data.neutralLimit;
		this->voidLimit = data.voidLimit;
		this->spiritLimit = data.spiritLimit;
		this->matterLimit = data.matterLimit;
		this->totalDamage = data.totalDamage;
		this->guardCrossTimer = data.guardCrossTimer;
		this->overdriveCrossTimer = data.overdriveCrossTimer;
		this->lifeBarEffect = data.lifeBarEffect;
		this->proration = data.proration;
		this->counter = data.counter;
		this->score = data.score;
		return *this;
	}

	BattleManager::HUDData &BattleManager::HUDData::operator=(const BattleManager::HUDDataPacked &data)
	{
		this->comboCtr = data.comboCtr;
		this->hitCtr = data.hitCtr;
		this->neutralLimit = data.neutralLimit;
		this->voidLimit = data.voidLimit;
		this->spiritLimit = data.spiritLimit;
		this->matterLimit = data.matterLimit;
		this->totalDamage = data.totalDamage;
		this->guardCrossTimer = data.guardCrossTimer;
		this->overdriveCrossTimer = data.overdriveCrossTimer;
		this->lifeBarEffect = data.lifeBarEffect;
		this->proration = data.proration;
		this->counter = data.counter;
		this->score = data.score;
		return *this;
	}

	BattleManager::HUDDataPacked &BattleManager::HUDDataPacked::operator=(BattleManager::HUDData &data)
	{
		this->comboCtr = data.comboCtr;
		this->hitCtr = data.hitCtr;
		this->neutralLimit = data.neutralLimit;
		this->voidLimit = data.voidLimit;
		this->spiritLimit = data.spiritLimit;
		this->matterLimit = data.matterLimit;
		this->totalDamage = data.totalDamage;
		this->guardCrossTimer = data.guardCrossTimer;
		this->overdriveCrossTimer = data.overdriveCrossTimer;
		this->lifeBarEffect = data.lifeBarEffect;
		this->proration = data.proration;
		this->counter = data.counter;
		this->score = data.score;
		return *this;
	}

	BattleManager::HUDDataPacked &BattleManager::HUDDataPacked::operator=(const BattleManager::HUDData &data)
	{
		this->comboCtr = data.comboCtr;
		this->hitCtr = data.hitCtr;
		this->neutralLimit = data.neutralLimit;
		this->voidLimit = data.voidLimit;
		this->spiritLimit = data.spiritLimit;
		this->matterLimit = data.matterLimit;
		this->totalDamage = data.totalDamage;
		this->guardCrossTimer = data.guardCrossTimer;
		this->overdriveCrossTimer = data.overdriveCrossTimer;
		this->lifeBarEffect = data.lifeBarEffect;
		this->proration = data.proration;
		this->counter = data.counter;
		this->score = data.score;
		return *this;
	}
}