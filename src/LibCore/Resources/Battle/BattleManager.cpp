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
#define LIMIT_SPRITE_VOID 0
#define LIMIT_SPRITE_MATTER 2
#define LIMIT_SPRITE_SPIRIT 4
#define LIMIT_SPRITE_NEUTRAL 6

namespace SpiralOfFate
{
	static const char *battleHudSprite[] = {
		"assets/battleui/player_hud.png",        // BATTLEUI_HUD_SEAT
		"assets/battleui/meterbars.png",         // BATTLEUI_MANA_BAR
		"assets/battleui/meterbars_disabled.png",// BATTLEUI_MANA_BAR_CROSS
		"assets/battleui/guard.png",             // BATTLEUI_GUARD_TEXT
		"assets/battleui/guardbar.png",          // BATTLEUI_GUARD_BAR
		"assets/battleui/guardbartmp.png",       // BATTLEUI_GUARD_BAR_TMP
		"assets/battleui/guard_red.png",         // BATTLEUI_GUARD_BAR_DISABLED
		"assets/battleui/lifebar.png",           // BATTLEUI_LIFE_BAR
		"assets/battleui/lifebar_red.png",       // BATTLEUI_LIFE_BAR_RED
		"assets/battleui/lifebar_texture.png",   // BATTLEUI_LIFE_BAR_EFFECT
		"assets/battleui/overdrive.png",         // BATTLEUI_OVERDRIVE
		"assets/battleui/overdrive_outline.png", // BATTLEUI_OVERDRIVE_OUTLINE
		"assets/battleui/round_container.png",   // BATTLEUI_SCORE_SEAT
		"assets/battleui/round_point.png",       // BATTLEUI_SCORE_BULLET
	};

	static const char *limitSprites[] = {
		"assets/battleui/void_limit.png",
		"assets/battleui/void_limit2.png",
		"assets/battleui/matter_limit.png",
		"assets/battleui/matter_limit2.png",
		"assets/battleui/spirit_limit.png",
		"assets/battleui/spirit_limit2.png",
		"assets/battleui/neutral_limit.png",
		"assets/battleui/neutral_limit2.png"
	};

	BattleManager::BattleManager(const StageParams &stage, const CharacterParams &leftCharacter, const CharacterParams &rightCharacter) :
		_leftCharacter(leftCharacter.character),
		_rightCharacter(rightCharacter.character),
		_leftHUDData{*this, *this->_leftCharacter, this->_leftHUDIcon, false},
		_rightHUDData{*this, *this->_rightCharacter, this->_rightHUDIcon, true}
	{
		nlohmann::json json;
		std::ifstream stream{"assets/effects/particles.json"};

		assert_exp(stream);
		stream >> json;
		this->_systemParticles.reserve(json.size());
		for (auto &v : json) {
			this->_systemParticles.emplace_back(v, "assets/effects");
			if (v.contains("index") && v["index"].is_number())
				assert_eq(v["index"].get<size_t>(), this->_systemParticles.size());
		}
		leftCharacter.character->systemParticles = &this->_systemParticles;
		rightCharacter.character->systemParticles = &this->_systemParticles;
		for (unsigned i = 0; i < spritesPaths.size(); i++)
			this->_moveSprites[i] = game->textureMgr.load(spritesPaths[i]);

		//TODO: Move this in another function
		this->_stage.textureHandle = game->textureMgr.load(stage.path);
		this->_stage.setPosition({STAGE_X_MIN - 50, -600});
		for (auto object : stage.objects())
			this->_stageObjects.emplace_back(object);
		for (auto object : stage.platforms())
			this->_platforms.emplace_back(object);
		this->_nbPlatform = this->_platforms.size();

		this->_leftCharacter->setOpponent(rightCharacter.character);
		this->_rightCharacter->setOpponent(leftCharacter.character);
		this->_leftCharacter->init(*this, leftCharacter.data);
		this->_rightCharacter->init(*this, rightCharacter.data);
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
		game->textureMgr.addRef(leftCharacter.icon);
		game->textureMgr.addRef(rightCharacter.icon);
		this->_leftIcon.textureHandle = leftCharacter.icon;
		this->_rightIcon.textureHandle = rightCharacter.icon;
		this->_leftHUDIcon.textureHandle = leftCharacter.icon;
		this->_rightHUDIcon.textureHandle = rightCharacter.icon;
		this->_oosBubble.textureHandle = game->textureMgr.load("assets/effects/oosBubble.png");
		this->_oosBubbleMask.textureHandle = game->textureMgr.load("assets/effects/oosBubbleMask.png");
		this->_stallWarn.textureHandle = game->textureMgr.load("assets/battleui/meter_warning.png");
		assert_exp(this->_stallDown.textureHandle = game->textureMgr.load("assets/battleui/meter_penalty.png", nullptr, true));
		game->textureMgr.setTexture(this->_stallWarn);
		game->textureMgr.setTexture(this->_stallDown);
		game->textureMgr.setTexture(this->_oosBubbleMask);
		game->textureMgr.setTexture(this->_oosBubble);
		game->textureMgr.setTexture(this->_leftIcon);
		game->textureMgr.setTexture(this->_rightIcon);
		game->textureMgr.setTexture(this->_leftHUDIcon);
		game->textureMgr.setTexture(this->_rightHUDIcon);
		this->_font.loadFromFile("assets/battleui/AERO_03.ttf");

		auto texSize1 = game->textureMgr.getTextureSize(this->_leftIcon.textureHandle).to<float>();
		auto texSize2 = game->textureMgr.getTextureSize(this->_rightIcon.textureHandle).to<float>();
		auto texSize = game->textureMgr.getTextureSize(this->_oosBubble.textureHandle);
		auto s1 = texSize.x / texSize1.x;
		auto s2 = texSize.x / texSize2.x;
		auto s3 = 65 / texSize1.x;
		auto s4 = 65 / texSize2.x;

		texSize1.x = 0;
		texSize1.y = texSize.y - texSize1.y * s1;
		this->_leftIcon.setScale({s1, s1});
		this->_leftIcon.setPosition(texSize1);

		texSize2.x = 0;
		texSize2.y = texSize.y - texSize2.y * s2;
		this->_rightIcon.setScale({s2, s2});
		this->_rightIcon.setPosition(texSize2);

		this->_leftHUDIcon.setScale({s3, s3});
		this->_leftHUDIcon.setPosition({0, 0});
		this->_rightHUDIcon.setScale({s4, s4});
		this->_rightHUDIcon.setPosition({0, 0});

		for (int i = 0; i < 8; i++) {
			this->_limitSprites[i].textureHandle = game->textureMgr.load(limitSprites[i]);
			game->textureMgr.setTexture(this->_limitSprites[i]);
			this->_limitSprites[i].setOrigin(
				this->_limitSprites[i].getTexture()->getSize().x / 2.f,
				this->_limitSprites[i].getTexture()->getSize().y / 2.f
			);
		}
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

		assert_exp(this->_leftHUDData.target.create(texSize.x, texSize.y));
		assert_exp(this->_rightHUDData.target.create(texSize.x, texSize.y));
		assert_exp(this->_leftHUD.create(550, 700));
		assert_exp(this->_rightHUD.create(550, 700));
		assert_exp(this->_hud.create(1100, 700));
		this->_hud;
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
		while (this->_fpsTimes.size() >= 60)
			this->_fpsTimes.pop_front();
		this->_fpsTimes.push_back(this->_fpsClock.restart().asMicroseconds());

		std::map<int, std::vector<IObject *>> objectLayers;
		float total = 0;

		for (auto &object : this->_stageObjects)
			objectLayers[object->getLayer()].push_back(&*object);
		for (auto &object : this->_platforms)
			objectLayers[object->getLayer()].push_back(&*object);
		for (auto &object : this->_objects)
			objectLayers[object.second->getLayer()].push_back(&*object.second);
		for (auto &object : this->_iobjects)
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
		sprite.setPosition(STAGE_X_MIN - 50, -600);
		sprite.setTexture(this->_hud.getTexture(), true);
		game->screen->draw(sprite);

		for (auto time : this->_tpsTimes)
			total += time;
		if (!this->_tpsTimes.empty()) {
			char buffer[12];

			sprintf(buffer, "%.2f TPS", 1000000.f / (total / this->_tpsTimes.size()));
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

			sprintf(buffer, "%.2f FPS", 1000000.f / (total / this->_fpsTimes.size()));
			game->screen->borderColor(2, sf::Color::Black);
			game->screen->fillColor(sf::Color::White);
			game->screen->textSize(20);
			game->screen->displayElement(buffer, {900 + STAGE_X_MIN, 50}, 145, Screen::ALIGN_RIGHT);
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
			this->_renderCharacter(*this->_leftCharacter);
			// <= 50, On top of background character
			while (it != objectLayers.end() && it->first <= 50) {
				for (auto obj : it->second)
					obj->render();
				it++;
			}
		}
		this->_renderCharacter(*this->_rightCharacter);
		if (!this->_leftFirst) {
			// <= 50, On top of background character
			while (it != objectLayers.end() && it->first <= 50) {
				for (auto obj: it->second)
					obj->render();
				it++;
			}
			this->_renderCharacter(*this->_leftCharacter);
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

	unsigned BattleManager::registerObject(const std::shared_ptr<Object> &object)
	{
		this->_objects.emplace_back(++this->_lastObjectId, object);
		return this->_lastObjectId;
	}

	unsigned BattleManager::registerObject(const std::shared_ptr<IObject> &object)
	{
		this->_iobjects.emplace_back(++this->_lastObjectId, object);
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
		this->_roundSprite.setPosition({(STAGE_X_MIN + STAGE_X_MAX) / 2.f + STAGE_X_MIN, -250});
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
		this->_roundSprite.setPosition({0, -250});
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
		std::vector<std::tuple<Object *, Object *, const FrameData *>> collisions;
		auto lflash = this->_leftCharacter->getCurrentFrameData()->dFlag.flash;
		auto rflash = this->_rightCharacter->getCurrentFrameData()->dFlag.flash;
		auto lchr = &*this->_leftCharacter;
		auto rchr = &*this->_rightCharacter;

		this->_limitAnimTimer++;
		this->_limitAnimTimer %= 360;
		for (auto &platform : this->_platforms) {
			platform->_cacheComputed = false;
			platform->update();
			platform->_computeFrameDataCache();
		}
		if (!rflash || lflash) {
			lchr->_cacheComputed = false;
			lchr->update();
			lchr->_computeFrameDataCache();
		}
		if (!lflash) {
			rchr->_cacheComputed = false;
			rchr->update();
			rchr->_computeFrameDataCache();
		}

		lflash = this->_leftCharacter->getCurrentFrameData()->dFlag.flash;
		rflash = this->_rightCharacter->getCurrentFrameData()->dFlag.flash;
		if (!lflash && !rflash) {
			// TODO: Using the sizes here since update() can grow the list of objects
			//       causing the iterators to be invalid.
			auto size = this->_objects.size();
			auto size2 = this->_iobjects.size();

			for (unsigned i = 0; i < size; i++) {
				this->_objects[i].second->_cacheComputed = false;
				this->_objects[i].second->update();
				this->_objects[i].second->_computeFrameDataCache();
			}
			for (unsigned i = 0; i < size2; i++)
				this->_iobjects[i].second->update();
			for (auto &object : this->_stageObjects) {
				object->_cacheComputed = false;
				object->update();
				object->_computeFrameDataCache();
			}
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
				if (attacker->isDisabled(*defender))
					continue;
				defender->getHit(*attacker, data);
				attacker->hit(*defender, data);
			}
		}

		assert_exp(!lchr->isDead());
		assert_exp(!rchr->isDead());

		// Not using std::remove_if because it doesn't work with MSVC for some reason
		for (unsigned i = 0; i < this->_objects.size(); i++)
			if (this->_objects[i].second->isDead())
				this->_objects.erase(this->_objects.begin() + i--);
		for (unsigned i = 0; i < this->_iobjects.size(); i++)
			if (this->_iobjects[i].second->isDead())
				this->_iobjects.erase(this->_iobjects.begin() + i--);

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

	std::shared_ptr<Object> BattleManager::getObjectFromId(unsigned int id) const
	{
		for (auto &object : this->_objects)
			if (object.first == id)
				return object.second;
		assert_not_reached();
		return nullptr;
	}

	std::shared_ptr<IObject> BattleManager::getIObjectFromId(unsigned int id) const
	{
		for (auto &object : this->_iobjects)
			if (object.first == id)
				return object.second;
		assert_not_reached();
		return nullptr;
	}

	unsigned int BattleManager::getBufferSize() const
	{
		size_t size = sizeof(Data) + this->_leftCharacter->getBufferSize() + this->_rightCharacter->getBufferSize();

		size += this->_objects.size() * sizeof(unsigned);
		for (auto &object : this->_objects)
			size += game->objFactory.getObjectSize(*object.second);
		size += this->_iobjects.size() * sizeof(unsigned);
		for (auto &object : this->_iobjects)
			size += game->objFactory.getObjectSize(*object.second);
		for (auto &object : this->_stageObjects)
			size += object->getBufferSize();
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
		dat->_limitAnimTimer = this->_limitAnimTimer;
		dat->_ended = this->_ended;
		dat->_lastObjectId = this->_lastObjectId;
		dat->_leftHUDData = this->_leftHUDData;
		dat->_rightHUDData = this->_rightHUDData;
		dat->_currentRound = this->_currentRound;
		dat->_roundStartTimer = this->_roundStartTimer;
		dat->_roundEndTimer = this->_roundEndTimer;
		dat->_nbObjects = this->_objects.size();
		dat->_nbIObjects = this->_iobjects.size();
		dat->_nbStageObjects = this->_stageObjects.size();
		dat->_currentFrame = this->_currentFrame;
		this->_leftCharacter->copyToBuffer((void *)ptr);
		ptr += this->_leftCharacter->getBufferSize();
		this->_rightCharacter->copyToBuffer((void *)ptr);
		ptr += this->_rightCharacter->getBufferSize();
		for (auto &object : this->_objects) {
			*(unsigned *)ptr = object.first;
			ptr += sizeof(unsigned);
			game->objFactory.saveObject(ptr, *object.second);
			ptr += game->objFactory.getObjectSize(*object.second);
		}
		for (auto &object : this->_iobjects) {
			*(unsigned *)ptr = object.first;
			ptr += sizeof(unsigned);
			game->objFactory.saveObject(ptr, *object.second);
			ptr += game->objFactory.getObjectSize(*object.second);
		}
		for (const auto &stageObject : this->_stageObjects) {
			stageObject->copyToBuffer((void *)ptr);
			ptr += stageObject->getBufferSize();
		}
		for (size_t i = 0; i < this->_nbPlatform; i++) {
			this->_platforms[i]->copyToBuffer((void *)ptr);
			ptr += this->_platforms[i]->getBufferSize();
		}
	}

	void BattleManager::restoreFromBuffer(void *data)
	{
		auto dat = reinterpret_cast<Data *>(data);
		char *ptr = (char *)data + sizeof(Data);

		if (dat->random != game->battleRandom.ser.invoke_count)
			game->battleRandom.rollback(dat->random);
		this->_limitAnimTimer = dat->_limitAnimTimer;
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

		this->_iobjects.clear();
		this->_iobjects.reserve(dat->_nbIObjects);
		this->_objects.clear();
		this->_objects.reserve(dat->_nbObjects);
		this->_platforms.erase(this->_platforms.begin() + this->_nbPlatform, this->_platforms.end());
		for (size_t i = 0; i < dat->_nbObjects; i++) {
			auto id = *(unsigned *)ptr;

			ptr += sizeof(unsigned);
			auto obj = game->objFactory.createObject<Object>(*this, ptr, {&*this->_leftCharacter, &*this->_rightCharacter});
			ptr += game->objFactory.getObjectSize(*obj);
			this->_objects.emplace_back(id, obj);
		}
		for (size_t i = 0; i < dat->_nbIObjects; i++) {
			auto id = *(unsigned *)ptr;

			ptr += sizeof(unsigned);
			auto obj = game->objFactory.createObject(*this, ptr, {&*this->_leftCharacter, &*this->_rightCharacter});
			ptr += game->objFactory.getObjectSize(*obj);
			this->_iobjects.emplace_back(id, obj);
		}
		assert_exp(dat->_nbStageObjects == this->_stageObjects.size());
		for (const auto &stageObject : this->_stageObjects) {
			stageObject->restoreFromBuffer((void *)ptr);
			ptr += stageObject->getBufferSize();
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
		while (this->_tpsTimes.size() >= 60)
			this->_tpsTimes.pop_front();
		this->_tpsTimes.push_back(this->_tpsClock.restart().asMicroseconds());

		if (
			this->_roundEndTimer > 120 ||
			(this->_leftCharacter->_hp > 0 && this->_rightCharacter->_hp > 0 && !this->_roundEndTimer) ||
			this->_roundEndTimer % 2 == 0
		)
			this->_gameUpdate();
		else if (this->onFrameSkipped)
			this->onFrameSkipped();

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

	const std::vector<ReplayData> &BattleManager::getLeftReplayData() const
	{
		return this->_leftCharacter->getReplayData();
	}

	const std::vector<ReplayData> &BattleManager::getRightReplayData() const
	{
		return this->_rightCharacter->getReplayData();
	}

	void BattleManager::_renderCharacter(const Character &chr)
	{
		for (int i = 0; i < 4; i++) {
			this->_limitSprites[i * 2].setRotation(i * 33 - this->_limitAnimTimer);
			this->_limitSprites[i * 2 + 1].setRotation(i * 33 + this->_limitAnimTimer);
		}
		if (chr._limitEffects & NEUTRAL_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_NEUTRAL].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_NEUTRAL].getTexture()->getSize().y / 2
			});
			game->textureMgr.render(this->_limitSprites[LIMIT_SPRITE_NEUTRAL]);
		}
		if (chr._limitEffects & MATTER_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_MATTER].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_MATTER].getTexture()->getSize().y / 2
			});
			game->textureMgr.render(this->_limitSprites[LIMIT_SPRITE_MATTER]);
		}
		if (chr._limitEffects & SPIRIT_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_SPIRIT].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_SPIRIT].getTexture()->getSize().y / 2
			});
			game->textureMgr.render(this->_limitSprites[LIMIT_SPRITE_SPIRIT]);
		}
		if (chr._limitEffects & VOID_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_VOID].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_VOID].getTexture()->getSize().y / 2
			});
			game->textureMgr.render(this->_limitSprites[LIMIT_SPRITE_VOID]);
		}
		chr.render();
		if (chr._limitEffects & NEUTRAL_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_NEUTRAL + 1].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_NEUTRAL + 1].getTexture()->getSize().y / 2
			});
			game->textureMgr.render(this->_limitSprites[LIMIT_SPRITE_NEUTRAL + 1]);
		}
		if (chr._limitEffects & MATTER_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_MATTER + 1].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_MATTER + 1].getTexture()->getSize().y / 2
			});
			game->textureMgr.render(this->_limitSprites[LIMIT_SPRITE_MATTER + 1]);
		}
		if (chr._limitEffects & SPIRIT_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_SPIRIT + 1].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_SPIRIT + 1].getTexture()->getSize().y / 2
			});
			game->textureMgr.render(this->_limitSprites[LIMIT_SPRITE_SPIRIT + 1]);
		}
		if (chr._limitEffects & VOID_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_VOID + 1].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_VOID + 1].getTexture()->getSize().y / 2
			});
			game->textureMgr.render(this->_limitSprites[LIMIT_SPRITE_VOID + 1]);
		}
	}

	void BattleManager::renderInputs()
	{
		this->renderLeftInputs();
		this->renderRightInputs();
	}

	void BattleManager::renderLeftInputs()
	{
		this->_renderInputs(this->_leftCharacter->getReplayData(), {-50 + STAGE_X_MIN, -495}, false);
	}

	void BattleManager::renderRightInputs()
	{
		this->_renderInputs(this->_rightCharacter->getReplayData(), {STAGE_X_MAX - 100, -495}, true);
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

	void BattleManager::_renderInputs(const std::vector<ReplayData> &data, Vector2f pos, bool side)
	{
		Sprite sprite;
		sf::Sprite s;
		unsigned total = 0;
		sf::RectangleShape shape;
		float off = 0;

		this->_tex.create(150, INPUT_DISPLAY_SIZE);
		this->_tex.clear(sf::Color::Transparent);
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
		if (dat1->_limitAnimTimer != dat2->_limitAnimTimer)
			game->logger.fatal("BattleManager::limitAnimTimer differs: " + std::to_string(dat1->_limitAnimTimer) + " vs " + std::to_string(dat2->_limitAnimTimer));
		if (dat1->_nbObjects != dat2->_nbObjects)
			game->logger.fatal("BattleManager::nbObjects differs: " + std::to_string(dat1->_nbObjects) + " vs " + std::to_string(dat2->_nbObjects));
		if (dat1->_nbIObjects != dat2->_nbIObjects)
			game->logger.fatal("BattleManager::nbIObjects differs: " + std::to_string(dat1->_nbIObjects) + " vs " + std::to_string(dat2->_nbIObjects));
		if (dat1->_nbStageObjects != dat2->_nbStageObjects)
			game->logger.fatal("BattleManager::nbStageObjects differs: " + std::to_string(dat1->_nbStageObjects) + " vs " + std::to_string(dat2->_nbStageObjects));

		auto length = this->_leftCharacter->printDifference("Player1: ", (void *)ptr1, (void *)ptr2, sizeof(Data));

		if (!length)
			return;
		ptr1 += length;
		ptr2 += length;

		length = this->_rightCharacter->printDifference("Player2: ", (void *)ptr1, (void *)ptr2, (ptrdiff_t)ptr1 - (ptrdiff_t)data1);
		if (!length)
			return;
		ptr1 += length;
		ptr2 += length;

		if (dat1->_nbObjects != dat2->_nbObjects)
			return;

		for (size_t i = 0; i < dat1->_nbObjects; i++) {
			std::shared_ptr<Object> obj;
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
				if (owner1 != owner2 || subobjid1 != subobjid2)
					return;
				obj = (owner1 ? this->_rightCharacter : this->_leftCharacter)->_spawnSubObject(*this,subobjid1, false).second;
				break;
			}
			default:
				game->logger.fatal("BattleManager::object[" + std::to_string(i) + "]::class invalid: " + std::to_string(cl1));
				return;
			}

			length = obj->printDifference(("BattleManager::object[" + std::to_string(i) + "]: ").c_str(), (void *)ptr1, (void *)ptr2, (ptrdiff_t)ptr1 - (ptrdiff_t)data1);
			if (length == 0)
				return;
			ptr1 += length;
			ptr2 += length;
		}

		//if (dat1->_nbIObjects != dat2->_nbIObjects)
		//	return;

		for (size_t i = 0; i < dat1->_nbIObjects; i++) {
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
				game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::class differs: " + std::to_string(cl1) + " vs " + std::to_string(cl2));
				return;
			}
			ptr1 += sizeof(unsigned char);
			ptr2 += sizeof(unsigned char);

			switch (cl1) {
			case 10: {
				auto owner1 = *(unsigned char *) ptr1;
				auto owner2 = *(unsigned char *) ptr2;
				if (owner1 != owner2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::owner differs: " + std::to_string(owner1) + " vs " + std::to_string(owner2));
				ptr1 += sizeof(unsigned char);
				ptr2 += sizeof(unsigned char);

				auto target1 = *(unsigned char *) ptr1;
				auto target2 = *(unsigned char *) ptr2;
				if (target1 != target2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::target differs: " + std::to_string(target1) + " vs " + std::to_string(target2));
				ptr1 += sizeof(unsigned char);
				ptr2 += sizeof(unsigned char);

				auto spawner1 = *(unsigned char *) ptr1;
				auto spawner2 = *(unsigned char *) ptr2;
				if (spawner1 != spawner2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::spawner differs: " + std::to_string(spawner1) + " vs " + std::to_string(spawner2));
				ptr1 += sizeof(unsigned char);
				ptr2 += sizeof(unsigned char);

				auto index1 = *(unsigned *) ptr1;
				auto index2 = *(unsigned *) ptr2;
				if (index1 != index2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::index differs: " + std::to_string(index1) + " vs " + std::to_string(index2));
				ptr1 += sizeof(unsigned);
				ptr2 += sizeof(unsigned);
				if (spawner1 != spawner2 || index1 != index2)
					return;

				auto genDat1 = (
					spawner1 == 2 ?
					this->_systemParticles :
					(
						spawner1 == 1 ?
						this->_rightCharacter :
						this->_leftCharacter
					)->_generators
				)[index1];

				obj = std::make_shared<ParticleGenerator>(
					ParticleGenerator::Source{spawner1, index1},
					genDat1,
					*(owner1 ? this->_rightCharacter : this->_leftCharacter),
					*(target1 ? this->_rightCharacter : this->_leftCharacter)
				);
				break;
			}
			case 11: {
				auto owner1 = *(unsigned char *) ptr1;
				auto owner2 = *(unsigned char *) ptr2;
				if (owner1 != owner2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::owner differs: " + std::to_string(owner1) + " vs " + std::to_string(owner2));
				ptr1 += sizeof(unsigned char);
				ptr2 += sizeof(unsigned char);

				auto spawner1 = *(unsigned char *) ptr1;
				auto spawner2 = *(unsigned char *) ptr2;
				if (spawner1 != spawner2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::spawner differs: " + std::to_string(spawner1) + " vs " + std::to_string(spawner2));
				ptr1 += sizeof(unsigned char);
				ptr2 += sizeof(unsigned char);

				auto genIndex1 = *(unsigned *) ptr1;
				auto genIndex2 = *(unsigned *) ptr2;
				if (genIndex1 != genIndex2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::genIndex differs: " + std::to_string(genIndex1) + " vs " + std::to_string(genIndex2));
				ptr1 += sizeof(unsigned);
				ptr2 += sizeof(unsigned);

				auto index1 = *(unsigned *) ptr1;
				auto index2 = *(unsigned *) ptr2;
				if (index1 != index2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::index differs: " + std::to_string(index1) + " vs " + std::to_string(index2));
				ptr1 += sizeof(unsigned);
				ptr2 += sizeof(unsigned);
				if (spawner1 != spawner2 || index1 != index2)
					return;

				auto genDat1 = (
					spawner1 == 2 ?
					this->_systemParticles :
					(
						spawner1 == 1 ?
						this->_rightCharacter :
						this->_leftCharacter
					)->_generators
				)[genIndex1];

				obj = std::make_shared<Particle>(
					Particle::Source{spawner1, genIndex1, index1},
					genDat1.particles[index1],
					*(owner1 ? this->_rightCharacter : this->_leftCharacter),
					genDat1.sprite,
					Vector2f{0, 0}
				);
				break;
			}
			default:
				game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::class invalid: " + std::to_string(cl1));
				return;
			}

			length = obj->printDifference(("BattleManager::iobject[" + std::to_string(i) + "]: ").c_str(), (void *)ptr1, (void *)ptr2, (ptrdiff_t)ptr1 - (ptrdiff_t)data1);
			if (length == 0)
				return;
			ptr1 += length;
			ptr2 += length;
		}

		if (dat1->_nbStageObjects != dat2->_nbStageObjects)
			return;
		if (dat1->_nbStageObjects != this->_stageObjects.size()) {
			game->logger.fatal("BattleManager::_nbStageObjects invalid: " + std::to_string(dat1->_nbStageObjects) + " != " + std::to_string(this->_stageObjects.size()));
			return;
		}
		for (size_t i = 0; i < this->_stageObjects.size(); i++) {
			length = this->_stageObjects[i]->printDifference(("BattleManager::stageObjects[" + std::to_string(i) + "]: ").c_str(), (void *)ptr1, (void *)ptr2, (ptrdiff_t)ptr1 - (ptrdiff_t)data1);
			if (length == 0)
				return;
			ptr1 += length;
			ptr2 += length;
		}
		for (size_t i = 0; i < this->_nbPlatform; i++) {
			length = this->_platforms[i]->printDifference(("BattleManager::platform[" + std::to_string(i) + "]: ").c_str(), (void *)ptr1, (void *)ptr2, (ptrdiff_t)ptr1 - (ptrdiff_t)data1);
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

	const std::vector<std::pair<unsigned, std::shared_ptr<Object>>> &BattleManager::getObjects() const
	{
		return this->_objects;
	}

	unsigned BattleManager::getFrame(void *data)
	{
		return reinterpret_cast<Data *>(data)->_currentFrame;
	}

	void BattleManager::printContent(void *data, size_t size)
	{
		auto dat1 = reinterpret_cast<Data *>(data);
		char *ptr1 = (char *)data + sizeof(Data);

		assert_not_reached();
		if (sizeof(Data) >= size)
			game->logger.warn("Manager is " + std::to_string(sizeof(Data) - size) + " bytes bigger than input");
		game->logger.info("BattleManager::random: " + std::to_string(dat1->random));
		game->logger.info("BattleManager::_currentFrame: " + std::to_string(dat1->_currentFrame));
		game->logger.info("BattleManager::_limitAnimTimer: " + std::to_string(dat1->_limitAnimTimer));
		game->logger.info("BattleManager::ended: " + std::to_string(dat1->_ended));
		game->logger.info("BattleManager::lastObjectId: " + std::to_string(dat1->_lastObjectId));
		game->logger.info("BattleManager::leftHUDData.comboCtr: " + std::to_string(dat1->_leftHUDData.comboCtr));
		game->logger.info("BattleManager::leftHUDData.hitCtr: " + std::to_string(dat1->_leftHUDData.hitCtr));
		game->logger.info("BattleManager::leftHUDData.neutralLimit: " + std::to_string(dat1->_leftHUDData.neutralLimit));
		game->logger.info("BattleManager::leftHUDData.voidLimit: " + std::to_string(dat1->_leftHUDData.voidLimit));
		game->logger.info("BattleManager::leftHUDData.matterLimit: " + std::to_string(dat1->_leftHUDData.matterLimit));
		game->logger.info("BattleManager::leftHUDData.spiritLimit: " + std::to_string(dat1->_leftHUDData.spiritLimit));
		game->logger.info("BattleManager::leftHUDData.totalDamage: " + std::to_string(dat1->_leftHUDData.totalDamage));
		game->logger.info("BattleManager::leftHUDData.proration: " + std::to_string(dat1->_leftHUDData.proration));
		game->logger.info("BattleManager::_leftHUDData.proration: " + std::to_string(dat1->_leftHUDData.score));
		game->logger.info("BattleManager::rightHUDData.comboCtr: " + std::to_string(dat1->_rightHUDData.comboCtr));
		game->logger.info("BattleManager::rightHUDData.hitCtr: " + std::to_string(dat1->_rightHUDData.hitCtr));
		game->logger.info("BattleManager::rightHUDData.neutralLimit: " + std::to_string(dat1->_rightHUDData.neutralLimit));
		game->logger.info("BattleManager::rightHUDData.voidLimit: " + std::to_string(dat1->_rightHUDData.voidLimit));
		game->logger.info("BattleManager::rightHUDData.spiritLimit: " + std::to_string(dat1->_rightHUDData.spiritLimit));
		game->logger.info("BattleManager::rightHUDData.matterLimit: " + std::to_string(dat1->_rightHUDData.matterLimit));
		game->logger.info("BattleManager::rightHUDData.totalDamage: " + std::to_string(dat1->_rightHUDData.totalDamage));
		game->logger.info("BattleManager::rightHUDData.proration: " + std::to_string(dat1->_rightHUDData.proration));
		game->logger.info("BattleManager::rightHUDData.score: " + std::to_string(dat1->_rightHUDData.score));
		game->logger.info("BattleManager::currentRound: " + std::to_string(dat1->_currentRound));
		game->logger.info("BattleManager::roundStartTimer: " + std::to_string(dat1->_roundStartTimer));
		game->logger.info("BattleManager::roundEndTimer: " + std::to_string(dat1->_roundEndTimer));
		game->logger.info("BattleManager::nbObjects: " + std::to_string(dat1->_nbObjects));
		if (sizeof(Data) >= size) {
			game->logger.fatal("Invalid input frame");
			return;
		}

		auto length = this->_leftCharacter->printContent("Player1: ", (void *)ptr1, sizeof(Data), size);

		if (!length)
			return;
		ptr1 += length;

		length = this->_rightCharacter->printContent("Player2: ", (void *)ptr1, (ptrdiff_t)ptr1 - (ptrdiff_t)data, size);
		if (!length)
			return;
		ptr1 += length;

		for (size_t i = 0; i < dat1->_nbObjects; i++) {
			std::shared_ptr<Object> obj;
			auto id1 = *(unsigned *)ptr1;

			if ((ptrdiff_t)ptr1 - (ptrdiff_t)data + sizeof(unsigned) + sizeof(unsigned char) >= size)
				game->logger.warn("Next object header is " + std::to_string((ptrdiff_t)ptr1 - (ptrdiff_t)data + sizeof(unsigned) + sizeof(unsigned char) - size) + " bytes bigger than input");
			game->logger.info("BattleManager::object[" + std::to_string(i) + "]::objectId: " + std::to_string(id1));
			ptr1 += sizeof(unsigned);

			auto cl1 = *(unsigned char *)ptr1;

			game->logger.info("BattleManager::object[" + std::to_string(i) + "]::class: " + std::to_string(cl1));
			ptr1 += sizeof(unsigned char);

			if ((ptrdiff_t)ptr1 - (ptrdiff_t)data >= size) {
				game->logger.fatal("Invalid input frame");
				return;
			}
			switch (cl1) {
			case 0:
				obj.reset(new Object());
				break;
			case 1:
				obj.reset(new Character());
				break;
			case 2: {
				if ((ptrdiff_t)ptr1 - (ptrdiff_t)data + sizeof(unsigned) + sizeof(bool) >= size)
					game->logger.warn("Next object header is " + std::to_string((ptrdiff_t)ptr1 - (ptrdiff_t)data + sizeof(unsigned) + sizeof(bool) - size) + " bytes bigger than input");

				auto owner1 = *(bool *)ptr1;

				game->logger.info("BattleManager::object[" + std::to_string(i) + "]::owner: " + std::to_string(owner1));
				ptr1 += sizeof(bool);

				auto subobjid1 = *(unsigned *)ptr1;

				game->logger.info("BattleManager::object[" + std::to_string(i) + "]::subobjectId: " + std::to_string(subobjid1));
				ptr1 += sizeof(unsigned);
				if ((ptrdiff_t)ptr1 - (ptrdiff_t)data >= size) {
					game->logger.fatal("Invalid input frame");
					return;
				}
				obj = (owner1 ? this->_rightCharacter : this->_leftCharacter)->_spawnSubObject(*this,subobjid1, false).second;
				break;
			}
			default:
				game->logger.fatal("BattleManager::object[" + std::to_string(i) + "]::class invalid: " + std::to_string(cl1));
				return;
			}

			length = obj->printContent(("BattleManager::object[" + std::to_string(i) + "]: ").c_str(), (void *)ptr1, (ptrdiff_t)ptr1 - (ptrdiff_t)data, size);
			if (length == 0)
				return;
			ptr1 += length;
		}
		for (size_t i = 0; i < this->_nbPlatform; i++) {
			length = this->_platforms[i]->printContent(("BattleManager::platform[" + std::to_string(i) + "]: ").c_str(), (void *)ptr1, (ptrdiff_t)ptr1 - (ptrdiff_t)data, size);
			if (length == 0)
				return;
			ptr1 += length;
		}
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

	BattleManager::HUDData::HUDData(BattleManager &mgr, Character &base, Sprite &icon, bool side) :
		mgr(mgr),
		base(base),
		icon(icon),
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
		auto width = game->textureMgr.getTextureSize(this->mgr._battleUi[guardId].textureHandle).x * guardVals.first / guardVals.second;

		this->mgr._battleUi[guardId].setPosition(260, 78);
		this->mgr._battleUi[guardId].setTextureRect({
			0, 0, static_cast<int>(width),
			static_cast<int>(game->textureMgr.getTextureSize(this->mgr._battleUi[guardId].textureHandle).y)
		});
		output.draw(this->mgr._battleUi[guardId], sf::BlendNone);

		if (!this->base._guardCooldown) {
			auto sizeX = game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_GUARD_BAR_TMP].textureHandle).x;

			this->mgr._battleUi[BATTLEUI_GUARD_BAR_TMP].setPosition(260 + width, 78);
			this->mgr._battleUi[BATTLEUI_GUARD_BAR_TMP].setTextureRect({
				static_cast<int>(width), 0,
				static_cast<int>(sizeX * (this->base._guardBarTmp / 2 + this->base._guardBar) / this->base._maxGuardBar - width),
				static_cast<int>(game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_GUARD_BAR_TMP].textureHandle).y)
			});
			output.draw(this->mgr._battleUi[BATTLEUI_GUARD_BAR_TMP], sf::BlendNone);
		}

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

		this->mgr._battleUi[BATTLEUI_MANA_BAR].setPosition(130, 655);
		output.draw(this->mgr._battleUi[BATTLEUI_MANA_BAR], sf::BlendNone);
		output.draw(this->icon);

		if (LIMIT_EFFECT_TIMER(this->base._limitEffects)) {
			this->renderMeterBar(output, {134, 660}, (float)this->base._mana / this->base._manaMax, {50, 50, 50}, {50, 50, 0});
			this->mgr._battleUi[BATTLEUI_MANA_BAR_CROSS].setPosition(130, 655);
			output.draw(this->mgr._battleUi[BATTLEUI_MANA_BAR_CROSS], sf::BlendAlpha);
		} else
			this->renderMeterBar(output, {134, 660}, (float)this->base._mana / this->base._manaMax, {200, 200, 200}, {200, 200, 0});
		if (this->base._stallingFactor > STALLING_PENALTY_THRESHOLD) {
			this->mgr._stallDown.setPosition(320, 620);
			this->mgr._stallDown.setTextureRect({
				0,
				static_cast<int>(this->penaltyTimer / -200),
				static_cast<int>(this->mgr._stallDown.getTexture()->getSize().x),
				static_cast<int>(this->mgr._stallDown.getTexture()->getSize().y)
			});
			output.draw(this->mgr._stallDown, sf::BlendNone);
		} else if (this->base._stallingFactor > START_STALLING_THRESHOLD) {
			this->mgr._stallWarn.setPosition(320, 620);
			output.draw(this->mgr._stallWarn, sf::BlendNone);
		}
		this->base.drawSpecialHUD(output);
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
				sf::BlendMode::DstColor,
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
		this->base.drawSpecialHUDNoReverse(output);
	}

	void BattleManager::HUDData::update()
	{
		if (this->base._guardCooldown)
			this->guardCrossTimer++;
		if (this->base._odCooldown)
			this->overdriveCrossTimer++;
		if (this->comboCtr)
			this->comboCtr--;
		if (this->base._stallingFactor > STALLING_PENALTY_THRESHOLD)
			this->penaltyTimer += (this->base._stallingFactor - STALLING_PENALTY_THRESHOLD) / 8 + 100;
		this->lifeBarEffect++;
		this->lifeBarEffect %= game->textureMgr.getTextureSize(this->mgr._battleUi[BATTLEUI_LIFE_BAR_EFFECT].textureHandle).x;
		if (this->base._opponent->_comboCtr) {
			this->hitCtr       = this->base._opponent->_comboCtr;
			this->neutralLimit = this->base._opponent->_limit[LIMIT_NEUTRAL];
			this->voidLimit    = this->base._opponent->_limit[LIMIT_VOID];
			this->matterLimit  = this->base._opponent->_limit[LIMIT_MATTER];
			this->spiritLimit  = this->base._opponent->_limit[LIMIT_SPIRIT];
			this->totalDamage  = this->base._opponent->_totalDamage;
			this->proration    = this->base._opponent->_prorate;
			this->counter      = this->base._opponent->_counter;
			this->comboCtr     = 120;
		}
	}

	BattleManager::HUDData &BattleManager::HUDData::operator=(BattleManager::HUDDataPacked &data)
	{
		this->penaltyTimer = data.penaltyTimer;
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
		this->penaltyTimer = data.penaltyTimer;
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
		this->penaltyTimer = data.penaltyTimer;
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
		this->penaltyTimer = data.penaltyTimer;
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