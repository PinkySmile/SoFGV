//
// Created by PinkySmile on 18/09/2021.
//

#include <memory>
#include <sstream>
#include "BattleManager.hpp"
#include "Logger.hpp"
#include "Resources/Game.hpp"
#include "Objects/Characters/SubObject.hpp"
#include "Utils.hpp"
#include "Objects/CheckUtils.hpp"

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
		_stage{ game->textureMgr.load(stage.path) },
		_font{ "assets/battleui/AERO_03.ttf" },
		_hud{ {1100, 700} },
		_leftHUD{ {550, 700} },
		_rightHUD{ {550, 700} },
		_stallWarn{ game->textureMgr.load("assets/battleui/meter_warning.png") },
		_stallDown{ game->textureMgr.load("assets/battleui/meter_penalty.png", nullptr, true) },
		_leftIcon{ leftCharacter.icon },
		_rightIcon{ rightCharacter.icon },
		_leftHUDIcon{ leftCharacter.icon },
		_rightHUDIcon{ rightCharacter.icon },
		_oosBubble{ game->textureMgr.load("assets/effects/oosBubble.png") },
		_oosBubbleMask{ game->textureMgr.load("assets/effects/oosBubbleMask.png") },
		_battleUi{
			game->textureMgr.load(battleHudSprite[BATTLEUI_HUD_SEAT],           nullptr, true),
			game->textureMgr.load(battleHudSprite[BATTLEUI_MANA_BAR],           nullptr, true),
			game->textureMgr.load(battleHudSprite[BATTLEUI_MANA_BAR_CROSS],     nullptr, true),
			game->textureMgr.load(battleHudSprite[BATTLEUI_GUARD_TEXT],         nullptr, true),
			game->textureMgr.load(battleHudSprite[BATTLEUI_GUARD_BAR],          nullptr, true),
			game->textureMgr.load(battleHudSprite[BATTLEUI_GUARD_BAR_TMP],      nullptr, true),
			game->textureMgr.load(battleHudSprite[BATTLEUI_GUARD_BAR_DISABLED], nullptr, true),
			game->textureMgr.load(battleHudSprite[BATTLEUI_LIFE_BAR],           nullptr, true),
			game->textureMgr.load(battleHudSprite[BATTLEUI_LIFE_BAR_RED],       nullptr, true),
			game->textureMgr.load(battleHudSprite[BATTLEUI_LIFE_BAR_EFFECT],    nullptr, true),
			game->textureMgr.load(battleHudSprite[BATTLEUI_OVERDRIVE],          nullptr, true),
			game->textureMgr.load(battleHudSprite[BATTLEUI_OVERDRIVE_OUTLINE],  nullptr, true),
			game->textureMgr.load(battleHudSprite[BATTLEUI_SCORE_SEAT],         nullptr, true),
			game->textureMgr.load(battleHudSprite[BATTLEUI_SCORE_BULLET],       nullptr, true),
		},
		_limitSprites{
			game->textureMgr.load(limitSprites[0]),
			game->textureMgr.load(limitSprites[1]),
			game->textureMgr.load(limitSprites[2]),
			game->textureMgr.load(limitSprites[3]),
			game->textureMgr.load(limitSprites[4]),
			game->textureMgr.load(limitSprites[5]),
			game->textureMgr.load(limitSprites[6]),
			game->textureMgr.load(limitSprites[7])
		},
		_cross{"assets/icons/netplay/twitter.png"},
		_roundSprites{
			sf::Texture{ "assets/icons/rounds/ko.png" },
			sf::Texture{ "assets/icons/rounds/start.png" },
			sf::Texture{ "assets/icons/rounds/p1win.png" },
			sf::Texture{ "assets/icons/rounds/p2win.png" },
			sf::Texture{ "assets/icons/rounds/id.png" },
			sf::Texture{ "assets/icons/rounds/round1.png" },
			sf::Texture{ "assets/icons/rounds/round2.png" },
			sf::Texture{ "assets/icons/rounds/round3.png" }
		},
		_roundSprite{ this->_roundSprites[1] },
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

		auto texSize1 = this->_leftIcon.getTextureSize().to<float>();
		auto texSize2 = this->_rightIcon.getTextureSize().to<float>();
		auto texSize = this->_oosBubble.getTextureSize();
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

		for (auto &limitSprite : this->_limitSprites)
			limitSprite.setOrigin(limitSprite.getTextureSize() / 2.f);

		this->_battleUi[BATTLEUI_GUARD_TEXT].setOrigin({
			static_cast<float>(this->_battleUi[BATTLEUI_GUARD_TEXT].getTextureSize().x / 2),
			0,
		});
		this->_battleUi[BATTLEUI_OVERDRIVE].setOrigin({
			static_cast<float>(this->_battleUi[BATTLEUI_OVERDRIVE].getTextureSize().x / 2),
			static_cast<float>(this->_battleUi[BATTLEUI_OVERDRIVE].getTextureSize().y / 2),
		});
		this->_battleUi[BATTLEUI_OVERDRIVE_OUTLINE].setOrigin({
			static_cast<float>(this->_battleUi[BATTLEUI_OVERDRIVE_OUTLINE].getTextureSize().x / 2),
			static_cast<float>(this->_battleUi[BATTLEUI_OVERDRIVE_OUTLINE].getTextureSize().y / 2),
		});

		assert_exp(this->_leftHUDData.target.resize(texSize));
		assert_exp(this->_rightHUDData.target.resize(texSize));
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

		auto e = event.getIf<sf::Event::KeyPressed>();

		if (e && e->code == sf::Keyboard::Key::F1)
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
			++it;
		}
		game->screen->displayElement(this->_stage);
		// <= -500, behind HUD
		while (it != objectLayers.end() && it->first <= -500) {
			for (auto obj : it->second)
				obj->render();
			++it;
		}

		this->_leftHUDData.render(this->_leftHUD);
		this->_leftHUD.display();
		this->_rightHUDData.render(this->_rightHUD);
		this->_rightHUD.display();

		sf::Sprite sprite{ this->_leftHUD.getTexture() };

		this->_hud.clear(Color::Transparent);
		sprite.setScale({1, 1});
		sprite.setPosition({0, 0});
		this->_hud.draw(sprite);
		sprite.setTexture(this->_rightHUD.getTexture(), true);
		sprite.setScale({-1, 1});
		sprite.setPosition({1100, 0});
		this->_hud.draw(sprite);

		this->_leftHUDData.renderNoReverse(this->_hud);
		this->_rightHUDData.renderNoReverse(this->_hud);

		this->_hud.display();
		sprite.setScale({1, 1});
		sprite.setPosition({STAGE_X_MIN - 50, -600});
		sprite.setTexture(this->_hud.getTexture(), true);
		game->screen->draw(sprite);

		for (auto time : this->_tpsTimes)
			total += time;
		if (!this->_tpsTimes.empty()) {
			char buffer[12];

			sprintf(buffer, "%.2f TPS", 1000000.f / (total / this->_tpsTimes.size()));
			game->screen->borderColor(2, Color::Black);
			game->screen->fillColor(Color::White);
			game->screen->textSize(20);
			game->screen->displayElement(buffer, {900 + STAGE_X_MIN, 75}, 145, Screen::ALIGN_RIGHT);
			game->screen->textSize(30);
			game->screen->borderColor(0, Color::Transparent);
		}
		total = 0;
		for (auto time : this->_fpsTimes)
			total += time;
		if (!this->_fpsTimes.empty()) {
			char buffer[12];

			sprintf(buffer, "%.2f FPS", 1000000.f / (total / this->_fpsTimes.size()));
			game->screen->borderColor(2, Color::Black);
			game->screen->fillColor(Color::White);
			game->screen->textSize(20);
			game->screen->displayElement(buffer, {900 + STAGE_X_MIN, 50}, 145, Screen::ALIGN_RIGHT);
			game->screen->textSize(30);
			game->screen->borderColor(0, Color::Transparent);
		}

		// < -50, behind characters
		while (it != objectLayers.end() && it->first < -50) {
			for (auto obj : it->second)
				obj->render();
			++it;
		}
		if (this->_leftFirst) {
			this->_renderCharacter(*this->_leftCharacter);
			// <= 50, On top of background character
			while (it != objectLayers.end() && it->first <= 50) {
				for (auto obj : it->second)
					obj->render();
				++it;
			}
		}
		this->_renderCharacter(*this->_rightCharacter);
		if (!this->_leftFirst) {
			// <= 50, On top of background character
			while (it != objectLayers.end() && it->first <= 50) {
				for (auto obj: it->second)
					obj->render();
				++it;
			}
			this->_renderCharacter(*this->_leftCharacter);
		}
		while (it != objectLayers.end()) {
			for (auto obj : it->second)
				obj->render();
			++it;
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
		this->_roundSprite.setColor(sf::Color{0xFF, 0xFF, 0xFF, static_cast<uint8_t>(alpha)});
		this->_roundSprite.setOrigin({
			this->_roundSprite.getTexture().getSize().x / 2.f,
			this->_roundSprite.getTexture().getSize().y / 2.f
		});
		this->_roundSprite.setPosition({(STAGE_X_MIN + STAGE_X_MAX) / 2.f, -250});
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
			this->_roundSprite.setColor(sf::Color{0xFF, 0xFF, 0xFF, static_cast<uint8_t>(alpha)});
		} else {
			auto scale = std::exp((this->_roundStartTimer - 120) / 10.f);
			auto alpha = (20 - (this->_roundStartTimer - 120)) / 20.f * 0xFF;

			this->_roundSprite.setTexture(this->_roundSprites[1], true);
			this->_roundSprite.setScale({scale, scale});
			this->_roundSprite.setColor(sf::Color{0xFF, 0xFF, 0xFF, static_cast<uint8_t>(alpha)});
		}
		this->_roundSprite.setOrigin({
			this->_roundSprite.getTexture().getSize().x / 2.f,
			this->_roundSprite.getTexture().getSize().y / 2.f
		});
		this->_roundSprite.setPosition({(STAGE_X_MIN + STAGE_X_MAX) / 2.f, -250});
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
		this->_roundSprite.setColor(sf::Color{0xFF, 0xFF, 0xFF, static_cast<uint8_t>(alpha)});
		this->_roundSprite.setOrigin({
			this->_roundSprite.getTexture().getSize().x / 2.f,
			this->_roundSprite.getTexture().getSize().y / 2.f
		});
		this->_roundSprite.setPosition({(STAGE_X_MIN + STAGE_X_MAX) / 2.f, -250});
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

			for (auto [attacker, defender, data]: collisions) {
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
		auto dat = static_cast<Data *>(data);
		char *ptr = static_cast<char *>(data) + sizeof(Data);

		game->logger.verbose("Saving BattleManager (Data size: " + std::to_string(sizeof(Data)) + ") @" + Utils::toHex(reinterpret_cast<uintptr_t>(dat)));
		dat->_random = game->battleRandom.ser.invoke_count;
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
		this->_leftCharacter->copyToBuffer(ptr);
		ptr += this->_leftCharacter->getBufferSize();
		this->_rightCharacter->copyToBuffer(ptr);
		ptr += this->_rightCharacter->getBufferSize();
		for (auto &object : this->_objects) {
			*reinterpret_cast<unsigned *>(ptr) = object.first;
			ptr += sizeof(unsigned);
			game->objFactory.saveObject(ptr, *object.second);
			ptr += game->objFactory.getObjectSize(*object.second);
		}
		for (auto &object : this->_iobjects) {
			*reinterpret_cast<unsigned *>(ptr) = object.first;
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
		auto dat = static_cast<Data *>(data);
		char *ptr = static_cast<char *>(data) + sizeof(Data);

		if (dat->_random != game->battleRandom.ser.invoke_count)
			game->battleRandom.rollback(dat->_random);
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
		this->_leftCharacter->restoreFromBuffer(ptr);
		ptr += this->_leftCharacter->getBufferSize();
		this->_rightCharacter->restoreFromBuffer(ptr);
		ptr += this->_rightCharacter->getBufferSize();

		// TODO: Instead of delete and recreate all object,
		//       hold onto them for a while
		this->_iobjects.clear();
		this->_iobjects.reserve(dat->_nbIObjects);
		this->_objects.clear();
		this->_objects.reserve(dat->_nbObjects);
		this->_platforms.erase(this->_platforms.begin() + this->_nbPlatform, this->_platforms.end());
		for (size_t i = 0; i < dat->_nbObjects; i++) {
			auto id = *reinterpret_cast<unsigned *>(ptr);

			ptr += sizeof(unsigned);
			auto obj = game->objFactory.createObject<Object>(*this, ptr, {&*this->_leftCharacter, &*this->_rightCharacter});
			ptr += game->objFactory.getObjectSize(*obj);
			this->_objects.emplace_back(id, obj);
		}
		for (size_t i = 0; i < dat->_nbIObjects; i++) {
			auto id = *reinterpret_cast<unsigned *>(ptr);

			ptr += sizeof(unsigned);
			auto obj = game->objFactory.createObject(*this, ptr, {&*this->_leftCharacter, &*this->_rightCharacter});
			ptr += game->objFactory.getObjectSize(*obj);
			this->_iobjects.emplace_back(id, obj);
		}
		assert_exp(dat->_nbStageObjects == this->_stageObjects.size());
		for (const auto &stageObject : this->_stageObjects) {
			stageObject->restoreFromBuffer(ptr);
			ptr += stageObject->getBufferSize();
		}
		for (size_t i = 0; i < this->_nbPlatform; i++) {
			this->_platforms[i]->restoreFromBuffer(ptr);
			ptr += this->_platforms[i]->getBufferSize();
		}
		this->_leftCharacter->resolveSubObjects(*this);
		this->_rightCharacter->resolveSubObjects(*this);
		game->logger.verbose("Restored BattleManager @" + Utils::toHex((uintptr_t)dat));
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
			this->_limitSprites[i * 2].setRotation(sf::degrees(i * 33 - this->_limitAnimTimer));
			this->_limitSprites[i * 2 + 1].setRotation(sf::degrees(i * 33 + this->_limitAnimTimer));
		}
		if (chr._limitEffects & NEUTRAL_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_NEUTRAL].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_NEUTRAL].getTextureSize().y / 2
			});
			game->screen->displayElement(this->_limitSprites[LIMIT_SPRITE_NEUTRAL]);
		}
		if (chr._limitEffects & MATTER_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_MATTER].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_MATTER].getTextureSize().y / 2
			});
			game->screen->displayElement(this->_limitSprites[LIMIT_SPRITE_MATTER]);
		}
		if (chr._limitEffects & SPIRIT_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_SPIRIT].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_SPIRIT].getTextureSize().y / 2
			});
			game->screen->displayElement(this->_limitSprites[LIMIT_SPRITE_SPIRIT]);
		}
		if (chr._limitEffects & VOID_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_VOID].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_VOID].getTextureSize().y / 2
			});
			game->screen->displayElement(this->_limitSprites[LIMIT_SPRITE_VOID]);
		}
		chr.render();
		if (chr._limitEffects & NEUTRAL_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_NEUTRAL + 1].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_NEUTRAL + 1].getTextureSize().y / 2
			});
			game->screen->displayElement(this->_limitSprites[LIMIT_SPRITE_NEUTRAL + 1]);
		}
		if (chr._limitEffects & MATTER_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_MATTER + 1].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_MATTER + 1].getTextureSize().y / 2
			});
			game->screen->displayElement(this->_limitSprites[LIMIT_SPRITE_MATTER + 1]);
		}
		if (chr._limitEffects & SPIRIT_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_SPIRIT + 1].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_SPIRIT + 1].getTextureSize().y / 2
			});
			game->screen->displayElement(this->_limitSprites[LIMIT_SPRITE_SPIRIT + 1]);
		}
		if (chr._limitEffects & VOID_LIMIT_EFFECT) {
			this->_limitSprites[LIMIT_SPRITE_VOID + 1].setPosition({
				chr._position.x,
				-chr._position.y - this->_limitSprites[LIMIT_SPRITE_VOID + 1].getTextureSize().y / 2
			});
			game->screen->displayElement(this->_limitSprites[LIMIT_SPRITE_VOID + 1]);
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
		Sprite sprite{ this->_moveSprites[spriteId] };

		sprite.setScale({
			(INPUT_DISPLAY_SIZE - 4.f) / sprite.getTextureSize().x,
			(INPUT_DISPLAY_SIZE - 4.f) / sprite.getTextureSize().y
		});
		game->screen->displayElement(sprite, {
			2 + pos.x + 4 + offset,
			2 + pos.y + k * (INPUT_DISPLAY_SIZE + 4)
		});
	}

	void BattleManager::_renderInputs(const std::vector<ReplayData> &data, Vector2f pos, bool side)
	{
		unsigned total = 0;
		sf::RectangleShape shape;
		float off = 0;

		assert_exp(this->_tex.resize({150, INPUT_DISPLAY_SIZE}));
		this->_tex.clear(sf::Color::Transparent);
		shape.setOutlineThickness(0);
		shape.setSize({INPUT_DISPLAY_SIZE, INPUT_DISPLAY_SIZE});
		shape.setFillColor(sf::Color{0, 0, 0, 0xA0});
		this->_tex.draw(shape);

		shape.setPosition({INPUT_DISPLAY_SIZE, 0});
		shape.setSize({150 - INPUT_DISPLAY_SIZE, INPUT_DISPLAY_SIZE});
		shape.setFillColor(sf::Color{0xA0, 0xA0, 0xA0, 0xA0});
		this->_tex.draw(shape);
		this->_tex.display();

		sf::Sprite s{this->_tex.getTexture()};

		if (!side) {
			s.setScale({-1, 1});
			off = 150;
		}
		for (unsigned k = 0; k < 18; k++) {
			s.setPosition({pos.x + off, pos.y + k * (INPUT_DISPLAY_SIZE + 4)});
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

				Sprite sprite{ this->_moveSprites[spriteId] };

				sprite.setScale({
					(INPUT_DISPLAY_SIZE - 4.f) / sprite.getTextureSize().x,
					(INPUT_DISPLAY_SIZE - 4.f) / sprite.getTextureSize().y
				});
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
		std::string msgStart;
		auto dat1 = static_cast<Data *>(data1);
		auto dat2 = static_cast<Data *>(data2);
		char *ptr1 = static_cast<char *>(data1) + sizeof(Data);
		char *ptr2 = static_cast<char *>(data2) + sizeof(Data);

		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _random, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _currentFrame, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _lastObjectId, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _currentRound, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _roundEndTimer, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _nbObjects, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _nbIObjects, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _nbStageObjects, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _roundStartTimer, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.comboCtr, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.hitCtr, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.neutralLimit, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.voidLimit, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.spiritLimit, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.matterLimit, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.totalDamage, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.guardCrossTimer, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.overdriveCrossTimer, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.lifeBarEffect, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.penaltyTimer, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.proration, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.counter, DISP_BOOL);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _leftHUDData.score, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.comboCtr, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.hitCtr, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.neutralLimit, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.voidLimit, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.spiritLimit, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.matterLimit, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.totalDamage, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.guardCrossTimer, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.overdriveCrossTimer, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.lifeBarEffect, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.penaltyTimer, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.proration, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.counter, DISP_BOOL);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _rightHUDData.score, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _limitAnimTimer, std::to_string);
		OBJECT_CHECK_FIELD("BattleManager", "", dat1, dat2, _ended, DISP_BOOL);

		auto length = this->_leftCharacter->printDifference("Player1: ", ptr1, ptr2, sizeof(Data));

		if (!length)
			return;
		ptr1 += length;
		ptr2 += length;

		length = this->_rightCharacter->printDifference("Player2: ", ptr1, ptr2, reinterpret_cast<ptrdiff_t>(ptr1) - reinterpret_cast<ptrdiff_t>(data1));
		if (!length)
			return;
		ptr1 += length;
		ptr2 += length;

		if (dat1->_nbObjects != dat2->_nbObjects)
			return;

		for (size_t i = 0; i < dat1->_nbObjects; i++) {
			std::shared_ptr<Object> obj;
			auto id1 = *reinterpret_cast<unsigned *>(ptr1);
			auto id2 = *reinterpret_cast<unsigned *>(ptr2);

			if (id1 != id2)
				game->logger.fatal("BattleManager::object[" + std::to_string(i) + "]::objectId differs: " + std::to_string(id1) + " vs " + std::to_string(id2));
			ptr1 += sizeof(unsigned);
			ptr2 += sizeof(unsigned);

			auto cl1 = *reinterpret_cast<unsigned char *>(ptr1);
			auto cl2 = *reinterpret_cast<unsigned char *>(ptr2);

			if (cl1 != cl2) {
				game->logger.fatal("BattleManager::object[" + std::to_string(i) + "]::class differs: " + std::to_string(cl1) + " vs " + std::to_string(cl2));
				return;
			}
			ptr1 += sizeof(unsigned char);
			ptr2 += sizeof(unsigned char);

			switch (cl1) {
			case 0:
				obj = std::make_shared<Object>();
				break;
			case 1:
				obj = std::make_shared<Character>();
				break;
			case 2: {
				auto owner1 = *reinterpret_cast<bool *>(ptr1);
				auto owner2 = *reinterpret_cast<bool *>(ptr2);

				if (owner1 != owner2)
					game->logger.fatal("BattleManager::object[" + std::to_string(i) + "]::owner differs: " + std::to_string(owner1) + " vs " + std::to_string(owner2));
				ptr1 += sizeof(bool);
				ptr2 += sizeof(bool);

				auto subobjid1 = *reinterpret_cast<unsigned *>(ptr1);
				auto subobjid2 = *reinterpret_cast<unsigned *>(ptr2);

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

		if (dat1->_nbIObjects != dat2->_nbIObjects)
			return;

		for (size_t i = 0; i < dat1->_nbIObjects; i++) {
			std::shared_ptr<IObject> obj;
			auto id1 = *reinterpret_cast<unsigned *>(ptr1);
			auto id2 = *reinterpret_cast<unsigned *>(ptr2);

			if (id1 != id2)
				game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::objectId differs: " + std::to_string(id1) + " vs " + std::to_string(id2));
			ptr1 += sizeof(unsigned);
			ptr2 += sizeof(unsigned);

			auto cl1 = *reinterpret_cast<unsigned char *>(ptr1);
			auto cl2 = *reinterpret_cast<unsigned char *>(ptr2);

			if (cl1 != cl2) {
				game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::class differs: " + std::to_string(cl1) + " vs " + std::to_string(cl2));
				return;
			}
			ptr1 += sizeof(unsigned char);
			ptr2 += sizeof(unsigned char);

			switch (cl1) {
			case 10: {
				auto owner1 = *reinterpret_cast<unsigned char *>(ptr1);
				auto owner2 = *reinterpret_cast<unsigned char *>(ptr2);
				if (owner1 != owner2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::owner differs: " + std::to_string(owner1) + " vs " + std::to_string(owner2));
				ptr1 += sizeof(unsigned char);
				ptr2 += sizeof(unsigned char);

				auto target1 = *reinterpret_cast<unsigned char *>(ptr1);
				auto target2 = *reinterpret_cast<unsigned char *>(ptr2);
				if (target1 != target2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::target differs: " + std::to_string(target1) + " vs " + std::to_string(target2));
				ptr1 += sizeof(unsigned char);
				ptr2 += sizeof(unsigned char);

				auto spawner1 = *reinterpret_cast<unsigned char *>(ptr1);
				auto spawner2 = *reinterpret_cast<unsigned char *>(ptr2);
				if (spawner1 != spawner2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::spawner differs: " + std::to_string(spawner1) + " vs " + std::to_string(spawner2));
				ptr1 += sizeof(unsigned char);
				ptr2 += sizeof(unsigned char);

				auto index1 = *reinterpret_cast<unsigned *>(ptr1);
				auto index2 = *reinterpret_cast<unsigned *>(ptr2);
				if (index1 != index2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::index differs: " + std::to_string(index1) + " vs " + std::to_string(index2));
				ptr1 += sizeof(unsigned);
				ptr2 += sizeof(unsigned);
				if (spawner1 != spawner2 || index1 != index2)
					return;

				auto &genDat1 = (
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
				auto owner1 = *reinterpret_cast<unsigned char *>(ptr1);
				auto owner2 = *reinterpret_cast<unsigned char *>(ptr2);
				if (owner1 != owner2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::owner differs: " + std::to_string(owner1) + " vs " + std::to_string(owner2));
				ptr1 += sizeof(unsigned char);
				ptr2 += sizeof(unsigned char);

				auto spawner1 = *reinterpret_cast<unsigned char *>(ptr1);
				auto spawner2 = *reinterpret_cast<unsigned char *>(ptr2);
				if (spawner1 != spawner2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::spawner differs: " + std::to_string(spawner1) + " vs " + std::to_string(spawner2));
				ptr1 += sizeof(unsigned char);
				ptr2 += sizeof(unsigned char);

				auto genIndex1 = *reinterpret_cast<unsigned *>(ptr1);
				auto genIndex2 = *reinterpret_cast<unsigned *>(ptr2);
				if (genIndex1 != genIndex2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::genIndex differs: " + std::to_string(genIndex1) + " vs " + std::to_string(genIndex2));
				ptr1 += sizeof(unsigned);
				ptr2 += sizeof(unsigned);

				auto index1 = *reinterpret_cast<unsigned *>(ptr1);
				auto index2 = *reinterpret_cast<unsigned *>(ptr2);
				if (index1 != index2)
					game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::index differs: " + std::to_string(index1) + " vs " + std::to_string(index2));
				ptr1 += sizeof(unsigned);
				ptr2 += sizeof(unsigned);
				if (spawner1 != spawner2 || index1 != index2)
					return;

				auto &genDat1 = (
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
		return static_cast<Data *>(data)->_currentFrame;
	}

	void BattleManager::printContent(void *data, size_t size)
	{
		auto dat = static_cast<Data *>(data);
		char *ptr = static_cast<char *>(data) + sizeof(Data);
		std::string msgStart;

		if (sizeof(Data) >= size)
			game->logger.warn("Manager is " + std::to_string(sizeof(Data) - size) + " bytes bigger than input");
		DISPLAY_FIELD("BattleManager", "", dat, _random, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _currentFrame, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _lastObjectId, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _currentRound, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _roundEndTimer, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _nbObjects, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _nbIObjects, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _nbStageObjects, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _roundStartTimer, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.comboCtr, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.hitCtr, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.neutralLimit, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.voidLimit, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.spiritLimit, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.matterLimit, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.totalDamage, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.guardCrossTimer, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.overdriveCrossTimer, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.lifeBarEffect, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.penaltyTimer, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.proration, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.counter, DISP_BOOL);
		DISPLAY_FIELD("BattleManager", "", dat, _leftHUDData.score, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.comboCtr, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.hitCtr, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.neutralLimit, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.voidLimit, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.spiritLimit, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.matterLimit, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.totalDamage, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.guardCrossTimer, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.overdriveCrossTimer, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.lifeBarEffect, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.penaltyTimer, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.proration, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.counter, DISP_BOOL);
		DISPLAY_FIELD("BattleManager", "", dat, _rightHUDData.score, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _limitAnimTimer, std::to_string);
		DISPLAY_FIELD("BattleManager", "", dat, _ended, DISP_BOOL);
		if (sizeof(Data) >= size) {
			game->logger.fatal("Invalid input frame");
			return;
		}

		auto length = this->_leftCharacter->printContent("Player1: ", ptr, sizeof(Data), size);

		if (!length)
			return;
		ptr += length;

		length = this->_rightCharacter->printContent("Player2: ", ptr, reinterpret_cast<ptrdiff_t>(ptr) - reinterpret_cast<ptrdiff_t>(data), size);
		if (!length)
			return;
		ptr += length;

		for (size_t i = 0; i < dat->_nbObjects; i++) {
			if (reinterpret_cast<ptrdiff_t>(ptr) - reinterpret_cast<ptrdiff_t>(data) + sizeof(unsigned) + sizeof(unsigned char) >= size)
				game->logger.warn("Next object header is " + std::to_string(reinterpret_cast<ptrdiff_t>(ptr) - reinterpret_cast<ptrdiff_t>(data) + sizeof(unsigned) + sizeof(unsigned char) - size) + " bytes bigger than input");

			std::shared_ptr<Object> obj;
			auto id = *reinterpret_cast<unsigned *>(ptr);

			game->logger.info("BattleManager::object[" + std::to_string(i) + "]::objectId: " + std::to_string(id));
			ptr += sizeof(unsigned);

			auto cl = *reinterpret_cast<unsigned char *>(ptr);

			game->logger.info("BattleManager::object[" + std::to_string(i) + "]::class: " + std::to_string(cl));
			ptr += sizeof(unsigned char);

			switch (cl) {
			case 0:
				obj = std::make_shared<Object>();
				break;
			case 1:
				obj = std::make_shared<Character>();
				break;
			case 2: {
				auto owner = *reinterpret_cast<bool *>(ptr);

				game->logger.info("BattleManager::object[" + std::to_string(i) + "]::owner: " + std::to_string(owner));
				ptr += sizeof(bool);

				auto subobjid = *reinterpret_cast<unsigned *>(ptr);

				game->logger.info("BattleManager::object[" + std::to_string(i) + "]::subobjectId: " + std::to_string(subobjid));
				ptr += sizeof(unsigned);
				obj = (owner ? this->_rightCharacter : this->_leftCharacter)->_spawnSubObject(*this, subobjid, false).second;
				break;
			}
			default:
				game->logger.info("BattleManager::object[" + std::to_string(i) + "]::class invalid: " + std::to_string(cl));
				return;
			}

			length = obj->printContent(("BattleManager::object[" + std::to_string(i) + "]: ").c_str(), ptr, reinterpret_cast<ptrdiff_t>(ptr) - reinterpret_cast<ptrdiff_t>(data), size);
			if (length == 0)
				return;
			ptr += length;
		}

		for (size_t i = 0; i < dat->_nbIObjects; i++) {
			if (reinterpret_cast<ptrdiff_t>(ptr) - reinterpret_cast<ptrdiff_t>(data) + sizeof(unsigned) + sizeof(unsigned char) >= size)
				game->logger.warn("Next object header is " + std::to_string(reinterpret_cast<ptrdiff_t>(ptr) - reinterpret_cast<ptrdiff_t>(data) + sizeof(unsigned) + sizeof(unsigned char) - size) + " bytes bigger than input");

			std::shared_ptr<IObject> obj;
			auto id = *reinterpret_cast<unsigned *>(ptr);

			game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::objectId: " + std::to_string(id));
			ptr += sizeof(unsigned);

			auto cl = *reinterpret_cast<unsigned char *>(ptr);

			game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::class: " + std::to_string(cl));
			ptr += sizeof(unsigned char);

			switch (cl) {
			case 10: {
				auto owner = *reinterpret_cast<unsigned char *>(ptr);
				game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::owner: " + std::to_string(owner));
				ptr += sizeof(unsigned char);

				auto target = *reinterpret_cast<unsigned char *>(ptr);
				game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::target: " + std::to_string(target));
				ptr += sizeof(unsigned char);

				auto spawner = *reinterpret_cast<unsigned char *>(ptr);
				game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::spawner: " + std::to_string(spawner));
				ptr += sizeof(unsigned char);

				auto index = *reinterpret_cast<unsigned *>(ptr);
				game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::index: " + std::to_string(index));
				ptr += sizeof(unsigned);

				auto &genDat1 = (
					spawner == 2 ?
					this->_systemParticles :
					(
						spawner == 1 ?
						this->_rightCharacter :
						this->_leftCharacter
					)->_generators
				)[index];

				obj = std::make_shared<ParticleGenerator>(
					ParticleGenerator::Source{spawner, index},
					genDat1,
					*(owner ? this->_rightCharacter : this->_leftCharacter),
					*(target ? this->_rightCharacter : this->_leftCharacter)
				);
				break;
			}
			case 11: {
				auto owner = *reinterpret_cast<unsigned char *>(ptr);
				game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::owner: " + std::to_string(owner));
				ptr += sizeof(unsigned char);

				auto spawner = *reinterpret_cast<unsigned char *>(ptr);
				game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::spawner: " + std::to_string(spawner));
				ptr += sizeof(unsigned char);

				auto genIndex = *reinterpret_cast<unsigned *>(ptr);
				game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::genIndex: " + std::to_string(genIndex));
				ptr += sizeof(unsigned);

				auto index = *reinterpret_cast<unsigned *>(ptr);
				game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::index: " + std::to_string(index));
				ptr += sizeof(unsigned);

				auto &genDat1 = (
					spawner == 2 ?
					this->_systemParticles :
					(
						spawner == 1 ?
						this->_rightCharacter :
						this->_leftCharacter
					)->_generators
				)[genIndex];

				obj = std::make_shared<Particle>(
					Particle::Source{spawner, genIndex, index},
					genDat1.particles[index],
					*(owner ? this->_rightCharacter : this->_leftCharacter),
					genDat1.sprite,
					Vector2f{0, 0}
				);
				break;
			}
			default:
				game->logger.fatal("BattleManager::iobject[" + std::to_string(i) + "]::class invalid: " + std::to_string(cl));
				return;
			}

			length = obj->printContent(("BattleManager::iobject[" + std::to_string(i) + "]: ").c_str(), ptr, reinterpret_cast<ptrdiff_t>(ptr) - reinterpret_cast<ptrdiff_t>(data), size);
			if (length == 0)
				return;
			ptr += length;
		}

		if (dat->_nbStageObjects != this->_stageObjects.size()) {
			game->logger.fatal("BattleManager::_nbStageObjects invalid: " + std::to_string(dat->_nbStageObjects) + " != " + std::to_string(this->_stageObjects.size()));
			return;
		}
		for (size_t i = 0; i < this->_stageObjects.size(); i++) {
			length = this->_stageObjects[i]->printContent(("BattleManager::stageObjects[" + std::to_string(i) + "]: ").c_str(), ptr, reinterpret_cast<ptrdiff_t>(ptr) - reinterpret_cast<ptrdiff_t>(data), size);
			if (length == 0)
				return;
			ptr += length;
		}
		for (size_t i = 0; i < this->_nbPlatform; i++) {
			length = this->_platforms[i]->printContent(("BattleManager::platform[" + std::to_string(i) + "]: ").c_str(), ptr, reinterpret_cast<ptrdiff_t>(ptr) - reinterpret_cast<ptrdiff_t>(data), size);
			if (length == 0)
				return;
			ptr += length;
		}
	}

	static float getTextSize(const std::string &str, const sf::Text &txt)
	{
		float size = 0;
		auto &f = txt.getFont();

		for (char c : str)
			size += f.getGlyph(c, txt.getCharacterSize(), false).advance;
		return size;
	}

	static Vector2f getPos(Vector2f basePos, float size, bool side)
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
		return getPos(basePos, getTextSize(str, txt), side);
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

	void BattleManager::HUDData::renderMeterBar(sf::RenderTarget &output, Vector2i pos, float bar, Color minColor, Color maxColor) const
	{
		sf::VertexArray buffer{sf::PrimitiveType::TriangleStrip, 4};
		sf::Vertex vertex;
		sf::Text text{ this->mgr._font };

		vertex.color = minColor;
		vertex.position = pos;
		buffer[0] = vertex;
		vertex.position = {16.f + pos.x, 16.f + pos.y};
		buffer[2] = vertex;

		vertex.color = sf::Color{
			static_cast<uint8_t>(minColor.r + (maxColor.r - minColor.r) * bar),
			static_cast<uint8_t>(minColor.g + (maxColor.g - minColor.g) * bar),
			static_cast<uint8_t>(minColor.b + (maxColor.b - minColor.b) * bar),
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
			text.setScale({-1, 1});
			text.setPosition(Vector2f(pos.x + 190 + size, pos.y - 1));
		} else
			text.setPosition(Vector2f(pos.x + 190, pos.y - 1));
		text.setString(str);
		output.draw(text);
	}

	void BattleManager::HUDData::render(sf::RenderTarget &output) const
	{
		float side = this->side ? -1.f : 1.f;

		output.clear(sf::Color::Transparent);

		this->mgr._battleUi[BATTLEUI_HUD_SEAT].setPosition({20, 20});
		output.draw(this->mgr._battleUi[BATTLEUI_HUD_SEAT], sf::BlendNone);

		this->mgr._battleUi[BATTLEUI_LIFE_BAR_RED].setPosition({69, 40});
		this->mgr._battleUi[BATTLEUI_LIFE_BAR_RED].setTextureRect({
			0, 0,
			static_cast<int>(this->mgr._battleUi[BATTLEUI_LIFE_BAR_RED].getTextureSize().x * std::min<float>(
				this->base._hp + static_cast<float>(this->base._totalDamage), this->base._baseHp
			) / this->base._baseHp),
			static_cast<int>(this->mgr._battleUi[BATTLEUI_LIFE_BAR_RED].getTextureSize().y)
		});
		output.draw(this->mgr._battleUi[BATTLEUI_LIFE_BAR_RED], sf::BlendAlpha);

		this->mgr._battleUi[BATTLEUI_LIFE_BAR].setPosition({69, 40});
		this->mgr._battleUi[BATTLEUI_LIFE_BAR].setTextureRect({
			0, 0,
			static_cast<int>(this->mgr._battleUi[BATTLEUI_LIFE_BAR].getTextureSize().x * this->base._hp / this->base._baseHp),
			static_cast<int>(this->mgr._battleUi[BATTLEUI_LIFE_BAR].getTextureSize().y)
		});
		output.draw(this->mgr._battleUi[BATTLEUI_LIFE_BAR], sf::BlendAlpha);

		this->mgr._battleUi[BATTLEUI_LIFE_BAR_EFFECT].setPosition({69, 40});
		this->mgr._battleUi[BATTLEUI_LIFE_BAR_EFFECT].setTextureRect({
			static_cast<int>(this->lifeBarEffect), 0,
			static_cast<int>(this->mgr._battleUi[BATTLEUI_LIFE_BAR].getTextureSize().x * this->base._hp / this->base._baseHp),
			static_cast<int>(this->mgr._battleUi[BATTLEUI_LIFE_BAR].getTextureSize().y)
		});
		output.draw(this->mgr._battleUi[BATTLEUI_LIFE_BAR_EFFECT], sf::BlendMode{
			sf::BlendMode::Factor::DstColor, sf::BlendMode::Factor::Zero, sf::BlendMode::Equation::Add,
			sf::BlendMode::Factor::Zero,     sf::BlendMode::Factor::One,  sf::BlendMode::Equation::Add
		});

		auto guardVals = this->base._guardCooldown ?
		                 std::pair<int, int>(this->base._maxGuardCooldown - this->base._guardCooldown, this->base._maxGuardCooldown) :
		                 std::pair<int, int>(this->base._guardBar, this->base._maxGuardBar);
		auto guardId = this->base._guardCooldown ? BATTLEUI_GUARD_BAR_DISABLED : BATTLEUI_GUARD_BAR;
		auto width = this->mgr._battleUi[guardId].getTextureSize().x * guardVals.first / guardVals.second;

		this->mgr._battleUi[guardId].setPosition({260, 78});
		this->mgr._battleUi[guardId].setTextureRect({
			0, 0, static_cast<int>(width),
			static_cast<int>(this->mgr._battleUi[guardId].getTextureSize().y)
		});
		output.draw(this->mgr._battleUi[guardId], sf::BlendNone);

		if (!this->base._guardCooldown) {
			auto sizeX = this->mgr._battleUi[BATTLEUI_GUARD_BAR_TMP].getTextureSize().x;

			this->mgr._battleUi[BATTLEUI_GUARD_BAR_TMP].setPosition(Vector2f(260 + width, 78));
			this->mgr._battleUi[BATTLEUI_GUARD_BAR_TMP].setTextureRect({
				static_cast<int>(width), 0,
				static_cast<int>(sizeX * (this->base._guardBarTmp / 2 + this->base._guardBar) / this->base._maxGuardBar - width),
				static_cast<int>(this->mgr._battleUi[BATTLEUI_GUARD_BAR_TMP].getTextureSize().y)
			});
			output.draw(this->mgr._battleUi[BATTLEUI_GUARD_BAR_TMP], sf::BlendNone);
		}

		this->mgr._battleUi[BATTLEUI_GUARD_TEXT].setScale({side, 1});
		this->mgr._battleUi[BATTLEUI_GUARD_TEXT].setPosition({222.f + this->mgr._battleUi[BATTLEUI_GUARD_TEXT].getTextureSize().x / 2, 62});
		this->mgr._battleUi[BATTLEUI_GUARD_TEXT].setColor(this->base._guardCooldown ? sf::Color{0x40, 0x40, 0x40} : sf::Color::White);
		output.draw(this->mgr._battleUi[BATTLEUI_GUARD_TEXT], sf::BlendAlpha);

		if (this->base._guardCooldown && this->guardCrossTimer % 60 > 30) {
			sf::Sprite sprite{ this->mgr._cross };

			sprite.setPosition({237, 65});
			output.draw(sprite, sf::BlendAlpha);
		}

		auto size = this->mgr._battleUi[BATTLEUI_OVERDRIVE].getTextureSize();

		this->mgr._battleUi[BATTLEUI_OVERDRIVE].setScale({side, 1});
		this->mgr._battleUi[BATTLEUI_OVERDRIVE].setPosition({
			420.f + this->mgr._battleUi[BATTLEUI_OVERDRIVE].getTextureSize().x / 2,
			65.f + this->mgr._battleUi[BATTLEUI_OVERDRIVE].getTextureSize().y / 2
		});
		this->mgr._battleUi[BATTLEUI_OVERDRIVE].setTextureRect({
			0, 0,
			static_cast<int>(size.x),
			static_cast<int>(size.y)
		});
		this->mgr._battleUi[BATTLEUI_OVERDRIVE].setColor(this->base._odCooldown ? sf::Color{0x40, 0x40, 0x40} : sf::Color::White);
		output.draw(this->mgr._battleUi[BATTLEUI_OVERDRIVE], sf::BlendAlpha);

		if (this->base._odCooldown) {
			this->mgr._battleUi[BATTLEUI_OVERDRIVE].setScale({side, 1});
			this->mgr._battleUi[BATTLEUI_OVERDRIVE].setPosition({
				420.f + size.x / 2 + (this->side ? 0 : size.x * this->base._odCooldown / this->base._barMaxOdCooldown),
				65.f + size.y / 2
			});
			this->mgr._battleUi[BATTLEUI_OVERDRIVE].setTextureRect({
				(this->side ? 0 : static_cast<int>(size.x * this->base._odCooldown / this->base._barMaxOdCooldown)),
				0,
				static_cast<int>(size.x - size.x * this->base._odCooldown / this->base._barMaxOdCooldown),
				static_cast<int>(size.y)
			});
			this->mgr._battleUi[BATTLEUI_OVERDRIVE].setColor(sf::Color{0xFF, 0x80, 0x80});
			output.draw(this->mgr._battleUi[BATTLEUI_OVERDRIVE], sf::BlendAlpha);
		}

		this->mgr._battleUi[BATTLEUI_OVERDRIVE_OUTLINE].setScale({side, 1});
		this->mgr._battleUi[BATTLEUI_OVERDRIVE_OUTLINE].setPosition({
			420.f + this->mgr._battleUi[BATTLEUI_OVERDRIVE_OUTLINE].getTextureSize().x / 2,
			65.f + this->mgr._battleUi[BATTLEUI_OVERDRIVE_OUTLINE].getTextureSize().y / 2
		});
		output.draw(this->mgr._battleUi[BATTLEUI_OVERDRIVE_OUTLINE], sf::BlendAlpha);

		if (this->base._odCooldown && this->overdriveCrossTimer % 60 > 30) {
			sf::Sprite sprite{ this->mgr._cross };

			sprite.setScale({2, 2});
			sprite.setOrigin({8, 8});
			sprite.setPosition({420 + size.x / 2.f, 65 + size.y / 2.f});
			output.draw(sprite, sf::BlendAlpha);
		}
		for (int i = 0; i < FIRST_TO; i++) {
			this->mgr._battleUi[BATTLEUI_SCORE_SEAT].setPosition({162 - i * 46.f, 69});
			output.draw(this->mgr._battleUi[BATTLEUI_SCORE_SEAT], sf::BlendNone);
		}
		for (int i = 0; i < this->score; i++) {
			this->mgr._battleUi[BATTLEUI_SCORE_BULLET].setPosition({167 - i * 46.f, 72});
			output.draw(this->mgr._battleUi[BATTLEUI_SCORE_BULLET], sf::BlendNone);
		}

		this->mgr._battleUi[BATTLEUI_MANA_BAR].setPosition({130, 655});
		output.draw(this->mgr._battleUi[BATTLEUI_MANA_BAR], sf::BlendNone);
		output.draw(this->icon);

		if (LIMIT_EFFECT_TIMER(this->base._limitEffects)) {
			this->renderMeterBar(output, {134, 660}, (float)this->base._mana / this->base._manaMax, {50, 50, 50}, {50, 50, 0});
			this->mgr._battleUi[BATTLEUI_MANA_BAR_CROSS].setPosition({130, 655});
			output.draw(this->mgr._battleUi[BATTLEUI_MANA_BAR_CROSS], sf::BlendAlpha);
		} else
			this->renderMeterBar(output, {134, 660}, (float)this->base._mana / this->base._manaMax, {200, 200, 200}, {200, 200, 0});
		if (this->base._stallingFactor > STALLING_PENALTY_THRESHOLD) {
			this->mgr._stallDown.setPosition({320, 620});
			this->mgr._stallDown.setTextureRect({
				0,
				static_cast<int>(this->penaltyTimer / -200),
				static_cast<int>(this->mgr._stallDown.getTextureSize().x),
				static_cast<int>(this->mgr._stallDown.getTextureSize().y)
			});
			output.draw(this->mgr._stallDown, sf::BlendNone);
		} else if (this->base._stallingFactor > START_STALLING_THRESHOLD) {
			this->mgr._stallWarn.setPosition({320, 620});
			output.draw(this->mgr._stallWarn, sf::BlendNone);
		}
		this->base.drawSpecialHUD(output);
	}

	void BattleManager::HUDData::renderNoReverse(sf::RenderTarget &output) const
	{
		sf::Text text{ this->mgr._font };

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
				sf::BlendMode::Factor::DstColor,
				sf::BlendMode::Factor::Zero,
				sf::BlendMode::Equation::Add,
				sf::BlendMode::Factor::Zero,
				sf::BlendMode::Factor::DstColor,
				sf::BlendMode::Equation::Add
			});
			this->target.draw(this->mgr._oosBubble);
			this->target.display();

			sf::Sprite sprite(this->target.getTexture());
			auto pos = this->base._position;

			if (!this->base._direction) {
				sprite.setScale({-1, 1});
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
		this->lifeBarEffect %= this->mgr._battleUi[BATTLEUI_LIFE_BAR_EFFECT].getTextureSize().x;
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

	BattleManager::CharacterParams::CharacterParams(bool side, Character *character, unsigned icon, const nlohmann::json &data) :
		character(character),
		icon(icon),
		data{
			.side = side,
			.maxHp = data["hp"],
			.maxJumps = data["jump_count"],
			.maxAirDash = data["air_dash_count"],
			.maxAirMovement = data["air_movements"],
			.maxMana = data["mana_max"],
			.startMana = data["mana_start"],
			.manaRegen = data["mana_regen"],
			.maxGuardBar = data["guard_bar"],
			.maxGuardCooldown = data["guard_break_cooldown"],
			.neutralOdCooldown = data["neutral_overdrive_cooldown"],
			.spiritOdCooldown = data["spirit_overdrive_cooldown"],
			.matterOdCooldown = data["matter_overdrive_cooldown"],
			.voidOdCooldown = data["void_overdrive_cooldown"],
			.rcCooldown = data["roman_cancel_cooldown"],
			.groundDrag = data["ground_drag"],
			.airDrag = { data["air_drag"]["x"], data["air_drag"]["y"] },
			.gravity = { data["gravity"]["x"],  data["gravity"]["y"] },
			.upDrift = {
				data["air_drift"]["up"]["accel"],
				data["air_drift"]["up"]["max"]
			},
			.downDrift = {
				data["air_drift"]["down"]["accel"],
				data["air_drift"]["down"]["max"]
			},
			.backDrift = {
				data["air_drift"]["back"]["accel"],
				data["air_drift"]["back"]["max"]
			},
			.frontDrift = {
				data["air_drift"]["front"]["accel"],
				data["air_drift"]["front"]["max"]
			}
		}
	{
	}
}