//
// Created by PinkySmile on 29/09/2021.
//

#include <utility>
#include <fstream>
#include "CharacterSelect.hpp"
#include "InGame.hpp"
#include "Resources/Game.hpp"
#include "Objects/Characters/Stickman/Stickman.hpp"
#include "Utils.hpp"
#include "Objects/Characters/VictoriaStar/VictoriaStar.hpp"

namespace SpiralOfFate
{
	CharacterSelect::CharacterSelect(
		std::shared_ptr<IInput> leftInput,
		std::shared_ptr<IInput> rightInput,
		const std::string &inGameName
	) :
		_leftInput(std::move(leftInput)),
		_rightInput(std::move(rightInput)),
		_inGameName(inGameName)
	{
		ViewPort view{{0, 0, 1680, 960}};
		nlohmann::json json;
		auto chrList = game->getCharacters();

		game->screen->setView(view);
		game->logger.info("CharacterSelect scene created");
		this->_entries.reserve(chrList.size());
		for (auto &entry : chrList) {
			auto file = entry + "/chr.json";

			game->logger.debug("Loading character from " + file);

			auto data = game->fileMgr.readFull(file);

			json = nlohmann::json::parse(data);
#ifndef _DEBUG
			if (json.contains("hidden") && json["hidden"])
				continue;
#endif
			this->_entries.emplace_back(json, entry);
		}
		std::sort(this->_entries.begin(), this->_entries.end(), [](CharacterEntry &a, CharacterEntry &b){
			return a.pos < b.pos;
		});

		auto data = game->fileMgr.readFull("assets/stages/list.json");

		json = nlohmann::json::parse(data);
		this->_stages.reserve(json.size());
		for (auto &elem: json)
			this->_stages.emplace_back(elem);
		this->_randomSprite.textureHandle = game->textureMgr.load("assets/stages/random.png");
	}

	CharacterSelect::CharacterSelect(
		std::shared_ptr<IInput> leftInput, std::shared_ptr<IInput> rightInput,
		int leftPos, int rightPos,
		int leftPalette, int rightPalette,
		int stage, int platformCfg,
		const std::string &inGameName
	) :
		CharacterSelect(std::move(leftInput), std::move(rightInput), inGameName)
	{
		this->_leftPos = leftPos;
		this->_rightPos = rightPos;
		this->_leftPalette = leftPalette;
		this->_rightPalette = rightPalette;
		this->_stage = stage;
		this->_platform = platformCfg;
	}

	CharacterSelect::CharacterSelect(const CharacterSelect::Arguments &arg) :
		CharacterSelect(
			std::move(arg.leftInput),
			std::move(arg.rightInput),
			arg.leftPos,
			arg.rightPos,
			arg.leftPalette,
			arg.rightPalette,
			arg.stage,
			arg.platformCfg,
			arg.inGameName
		)
	{
	}

	CharacterSelect::~CharacterSelect()
	{
		game->textureMgr.remove(this->_randomSprite.textureHandle);
	}

	void CharacterSelect::render() const
	{
		this->_selectingStage ? this->_selectStageRender() : this->_selectCharacterRender();
	}

	void CharacterSelect::update()
	{
		this->_leftInput->update();
		this->_rightInput->update();

		if (this->_quit)
			return game->scene.switchScene("title_screen");
		if (this->_selectingStage)
			this->_selectStageUpdate();
		else
			this->_selectCharacterUpdate();
	}

	void CharacterSelect::consumeEvent(const sf::Event &event)
	{
		this->_leftInput->consumeEvent(event);
		this->_rightInput->consumeEvent(event);
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
			this->_quit = true;
	}

	Character *CharacterSelect::_createCharacter(int pos, int posOp, int palette, std::shared_ptr<IInput> input)
	{
		return createCharacter(this->_entries[pos], this->_entries[posOp], pos, palette, std::move(input));
	}

	Character *CharacterSelect::createCharacter(const CharacterEntry &entry, const CharacterEntry &entryOp, int pos, int palette, std::shared_ptr<IInput> input)
	{
		Character *chr;
		std::pair<std::vector<Color>, std::vector<Color>> palettes;

		if (!entry.palettes.empty() && palette) {
			palettes.first = entry.palettes.front();
			palettes.second = entry.palettes[palette];
		}
		switch (entry._class) {
		case 2:
			chr = new VictoriaStar{
				static_cast<unsigned>(palette << 16 | pos),
				entry.folder,
				palettes,
				std::move(input),
				std::filesystem::path(entryOp.folder).filename().string()
			};
			break;
		case 1:
			chr = new Stickman{
				static_cast<unsigned>(palette << 16 | pos),
				entry.folder,
				palettes,
				std::move(input)
			};
			break;
		default:
			chr = new Character{
				static_cast<unsigned>(palette << 16 | pos),
				entry.folder,
				palettes,
				std::move(input)
			};
			break;
		}

		chr->name = entry.name;
		return chr;
	}

	void CharacterSelect::_launchGame()
	{
		game->soundMgr.play(BASICSOUND_GAME_LAUNCH);
		if (this->_inGameName.empty())
			return;

		auto args = new InGame::Arguments();

		args->saveReplay = true;
		args->endScene = game->scene.getCurrentScene().first;
		args->characterSelectScene = this;
		game->scene.switchScene(this->_inGameName, args);
	}

	void CharacterSelect::_selectCharacterRender() const
	{
		std::uniform_int_distribution<size_t> dist{0, this->_entries.size() - 1};
		size_t left  = this->_leftPos == -1  ? dist(game->random) : this->_leftPos;
		size_t right = this->_rightPos == -1 ? dist(game->random) : this->_rightPos;
		auto lInputs = this->_leftInput->getInputs();
		auto rInputs = this->_rightInput->getInputs();

		game->screen->fillColor(sf::Color::Black);
		game->screen->displayElement({0, 0, 1680, 960}, sf::Color{
			static_cast<sf::Uint8>(lInputs.d),
			static_cast<sf::Uint8>(rInputs.d),
			static_cast<sf::Uint8>((lInputs.d + rInputs.d) / 2)
		});
		game->screen->displayElement({0, 0, 560, 480}, sf::Color{0xA0, 0xA0, 0xA0, 0xFF});
		game->screen->displayElement({0, 480, 560, 480}, sf::Color::White);

		game->screen->displayElement({1120, 0, 560, 480}, sf::Color{0xA0, 0xA0, 0xA0, 0xFF});
		game->screen->displayElement({1120, 480, 560, 480}, sf::Color::White);

		game->screen->displayElement(this->_leftPos == -1 ? L"Random select" : this->_entries[this->_leftPos].name, {0, 480}, 560, Screen::ALIGN_CENTER);
		game->screen->displayElement(this->_rightPos == -1 ? L"Random select" : this->_entries[this->_rightPos].name, {1120, 480}, 560, Screen::ALIGN_CENTER);

		auto &leftSprites  = this->_entries[left].icon;
		auto &rightSprites = this->_entries[right].icon;
		std::uniform_int_distribution<size_t> ldist{0, this->_entries[left].palettes.size() - 1};
		std::uniform_int_distribution<size_t> rdist{0, this->_entries[right].palettes.size() - 1};
		int realLPal = this->_leftPos == -1  ? 0 : this->_leftPalette;
		int realRPal = this->_rightPos == -1 ? 0 : this->_rightPalette;
		auto &leftSprite  = leftSprites[realLPal % leftSprites.size()];
		auto &rightSprite = rightSprites[realRPal % rightSprites.size()];
		auto leftTexture  = game->textureMgr.getTextureSize(leftSprite.textureHandle);
		auto rightTexture = game->textureMgr.getTextureSize(rightSprite.textureHandle);

		leftSprite.setPosition(0, 0);
		leftSprite.setScale(560.f / leftTexture.x, 480.f / leftTexture.y);
		game->textureMgr.render(leftSprite);

		rightSprite.setPosition(1680, 0);
		rightSprite.setScale(-560.f / rightTexture.x, 480.f / rightTexture.y);
		game->textureMgr.render(rightSprite);
		game->screen->displayElement({540, 0, 600, 40}, sf::Color{0xB0, 0xB0, 0xB0, 0xFF});
		game->screen->displayElement("Character select", {540, 0}, 600, Screen::ALIGN_CENTER);
	}

	void CharacterSelect::_selectStageRender() const
	{
		Sprite &sprite = this->_stage == -1 ? this->_randomSprite : this->_stageSprite;

		sprite.setPosition(0, 0);
		sprite.setScale({
			1680.f / game->textureMgr.getTextureSize(sprite.textureHandle).x,
			960.f / game->textureMgr.getTextureSize(sprite.textureHandle).y
		});
		game->textureMgr.render(sprite);
		this->_displayPlatformPreview();
		game->screen->displayElement({540, 0, 600, 40}, sf::Color{0xB0, 0xB0, 0xB0, 0xFF});
		game->screen->displayElement("Stage select", {540, 0}, 600, Screen::ALIGN_CENTER);
		game->screen->textSize(20);
		game->screen->displayElement({540, 250, 600, 50}, sf::Color{0xB0, 0xB0, 0xB0, 0xA0});
		game->screen->displayElement("^", {540, 250}, 600, Screen::ALIGN_CENTER);
		game->screen->displayElement(this->_stage == -1 ? "Random select" : this->_stages[this->_stage].name, {540, 260}, 600, Screen::ALIGN_CENTER);
		game->screen->displayElement("v", {540, 280}, 600, Screen::ALIGN_CENTER);
		if (this->_stage != -1) {
			auto txt = "Credits to " + this->_stages[this->_stage].credits;
			auto size = game->screen->getTextSize(txt);

			game->screen->displayElement({static_cast<int>(1676 - size), 930, static_cast<int>(size + 4), 30}, sf::Color{0xB0, 0xB0, 0xB0, 0xFF});
			game->screen->displayElement(txt, {1678 - size, 932});
		}
		game->screen->textSize(30);
	}

	void CharacterSelect::_selectCharacterUpdate()
	{
		auto lInputs = this->_leftInput->getInputs();
		auto rInputs = this->_rightInput->getInputs();

		if (lInputs.horizontalAxis == -1) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			if (this->_leftPos == -1)
				this->_leftPos = static_cast<int>(this->_entries.size());
			this->_leftPos--;
		} else if (lInputs.horizontalAxis == 1) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			this->_leftPos++;
			if (this->_leftPos == static_cast<int>(this->_entries.size()))
				this->_leftPos = -1;
		}

		if (rInputs.horizontalAxis == -1) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			if (this->_rightPos == -1)
				this->_rightPos = static_cast<int>(this->_entries.size());
			this->_rightPos--;
		} else if (rInputs.horizontalAxis == 1) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			this->_rightPos++;
			if (this->_rightPos == static_cast<int>(this->_entries.size()))
				this->_rightPos = -1;
		}

		if (this->_leftPos >= 0) {
			if (lInputs.verticalAxis == -1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				do {
					if (this->_leftPalette == 0)
						this->_leftPalette = static_cast<int>(this->_entries[this->_leftPos].palettes.size());
					this->_leftPalette--;
				} while (this->_entries[this->_leftPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette && this->_leftPos == this->_rightPos);
			} else if (lInputs.verticalAxis == 1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				do {
					this->_leftPalette++;
					if (this->_leftPalette == static_cast<int>(this->_entries[this->_leftPos].palettes.size()))
						this->_leftPalette = 0;
				} while (this->_entries[this->_leftPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette && this->_leftPos == this->_rightPos);
			}
		}
		if (this->_rightPos >= 0) {
			if (rInputs.verticalAxis == -1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				do {
					if (this->_rightPalette == 0)
						this->_rightPalette = static_cast<int>(this->_entries[this->_rightPos].palettes.size());
					this->_rightPalette--;
				} while (this->_entries[this->_rightPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette && this->_leftPos == this->_rightPos);
			} else if (rInputs.verticalAxis == 1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				do {
					this->_rightPalette++;
					if (this->_rightPalette == static_cast<int>(this->_entries[this->_rightPos].palettes.size()))
						this->_rightPalette = 0;
				} while (this->_entries[this->_rightPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette && this->_leftPos == this->_rightPos);
			}
		}

		if (lInputs.n == 1 || rInputs.n == 1) {
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			this->_selectingStage = true;
			this->_stageSprite.textureHandle = this->_stages[0].imageHandle;
		}
	}

	void CharacterSelect::_selectStageUpdate()
	{
		auto lInputs = this->_leftInput->getInputs();
		auto rInputs = this->_rightInput->getInputs();

		if (this->_stage != -1) {
			if (lInputs.horizontalAxis == -1 || rInputs.horizontalAxis == -1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				if (this->_platform == -1)
					this->_platform = static_cast<int>(this->_stages[this->_stage].platforms.size());
				this->_platform--;
			} else if (lInputs.horizontalAxis == 1 || rInputs.horizontalAxis == 1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				this->_platform++;
				if (this->_platform == static_cast<int>(this->_stages[this->_stage].platforms.size()))
					this->_platform = -1;
			}
		}
		if (lInputs.verticalAxis == -1 || rInputs.verticalAxis == -1) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			if (this->_stage == -1)
				this->_stage = static_cast<int>(this->_stages.size());
			this->_stage--;
			if (this->_stage != -1)
				this->_stageSprite.textureHandle = this->_stages[this->_stage].imageHandle;
			this->_platform = 0;
		} else if (lInputs.verticalAxis == 1 || rInputs.verticalAxis == 1) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			this->_stage++;
			if (this->_stage == static_cast<int>(this->_stages.size()))
				this->_stage = -1;
			if (this->_stage != -1)
				this->_stageSprite.textureHandle = this->_stages[this->_stage].imageHandle;
			this->_platform = 0;
		}

		if (lInputs.n == 1 || rInputs.n == 1)
			return this->_launchGame();
		if (lInputs.s == 1 || rInputs.s == 1) {
			this->_selectingStage = false;
			game->soundMgr.play(BASICSOUND_MENU_CANCEL);
		}
	}

	void CharacterSelect::_displayPlatformPreview() const
	{
		if (this->_stage == -1)
			return;

		std::uniform_int_distribution<size_t> dist{0, this->_stages[this->_stage].platforms.size() - 1};
		auto plat = this->_platform == -1 ? dist(game->random) : this->_platform;
		const Vector2f scale = {
			1680.f / 1100,
			960.f / 700
		};
		Sprite sprite;

		for (auto &platform : this->_stages[this->_stage].platforms[plat]) {
			auto scale2 = Vector2f{
				platform.data.scale.x,
				platform.data.scale.y
			};
			auto size2 = Vector2f{
				platform.data.textureBounds.size.x * platform.data.scale.x,
				platform.data.textureBounds.size.y * platform.data.scale.y
			};
			auto result = platform.data.offset + platform.pos;

			result.y *= -1;
			result += Vector2f{
				size2.x / -2.f,
				-size2.y
			};
			result += Vector2f{
				platform.data.textureBounds.size.x * scale2.x / 2,
				platform.data.textureBounds.size.y * scale2.y / 2
			};
			sprite.setOrigin(platform.data.textureBounds.size / 2.f);
			sprite.setPosition({(result.x + 50 - STAGE_X_MIN) * scale.x, (result.y + 600) * scale.y});
			sprite.setScale({scale.x * scale2.x, scale.y * scale2.y});
			sprite.textureHandle = platform.data.textureHandle;
			sprite.setTextureRect(platform.data.textureBounds);
			game->textureMgr.render(sprite);
		}
	}

	CharacterSelect *CharacterSelect::create(SceneArguments *args)
	{
		checked_cast(realArgs, CharacterSelect::Arguments, args);

		if (realArgs->reportProgressA)
			realArgs->reportProgressA("Loading assets...");
		return new CharacterSelect(*realArgs);
	}

	InGame::InitParams CharacterSelect::createParams(SceneArguments *args)
	{
		if (args->reportProgressA)
			args->reportProgressA("Generating data...");

		std::uniform_int_distribution<size_t> dist{0, this->_entries.size() - 1};
		std::uniform_int_distribution<size_t> dist2{0, this->_stages.size() - 1};

		if (this->_stage == -1) {
			this->_platform = -1;
			this->_stage = dist2(game->random);
		}

		std::uniform_int_distribution<size_t> dist3{0, this->_stages[this->_stage].platforms.size() - 1};
		auto &stage = this->_stages[this->_stage];

		if (this->_platform == -1)
			this->_platform = dist3(game->random);
		if (this->_leftPos < 0)
			this->_leftPalette = 0;
		if (this->_rightPos < 0)
			this->_rightPalette = 0;
		if (this->_leftPos < 0)
			this->_leftPos = dist(game->random);
		if (this->_rightPos < 0)
			this->_rightPos = dist(game->random);
		if (this->_leftPos == this->_rightPos && this->_entries[this->_leftPos].palettes.size() <= 1) {
			this->_leftPalette = 0;
			this->_rightPalette = 0;
		} else if (
			this->_leftPos == this->_rightPos &&
			this->_entries[this->_leftPos].palettes.size() == 2 &&
			this->_leftPalette == this->_rightPalette
		) {
			this->_leftPalette = 0;
			this->_rightPalette = 1;
		}
		if (
			this->_leftPos == this->_rightPos &&
			this->_leftPalette == this->_rightPalette &&
			this->_entries[this->_leftPos].palettes.size() > 1
		) {
			this->_rightPalette++;
			this->_rightPalette %= this->_entries[this->_leftPos].palettes.size();
		}

		auto &lentry = this->_entries[this->_leftPos];
		auto &rentry = this->_entries[this->_rightPos];
		auto &licon = lentry.icon[this->_leftPalette];
		auto &ricon = rentry.icon[this->_rightPalette];

		if (args->reportProgressW)
			args->reportProgressW(L"Loading P1's character (" + this->_entries[this->_leftPos].name + L")");

		auto lchr = this->_createCharacter(this->_leftPos, this->_rightPos, this->_leftPalette, this->_leftInput);

		if (args->reportProgressW)
			args->reportProgressW(L"Loading P2's character (" + this->_entries[this->_rightPos].name + L")");

		auto rchr = this->_createCharacter(this->_rightPos, this->_leftPos, this->_rightPalette, this->_rightInput);

		return {
			{static_cast<unsigned>(this->_stage), 0, static_cast<unsigned>(this->_platform)},
			stage.platforms[this->_platform],
			stage,
			lchr,
			rchr,
			licon.textureHandle,
			ricon.textureHandle,
			lentry.entry,
			rentry.entry
		};
	}

	InGame::InitParams CharacterSelect::staticCreateParams(std::vector<StageEntry> &stages, std::vector<CharacterEntry> &entries, InGameArguments *args, std::shared_ptr<IInput> leftInput, std::shared_ptr<IInput> rightInput)
	{
		std::uniform_int_distribution<size_t> dist{0, entries.size() - 1};
		std::uniform_int_distribution<size_t> dist2{0, stages.size() - 1};
		int leftPos = args->startParams.p1chr;
		int rightPos = args->startParams.p2chr;
		int leftPalette = args->startParams.p1pal;
		int rightPalette = args->startParams.p2pal;
		int _stage = args->startParams.stage;
		int platform = args->startParams.platformConfig;

		if (_stage == -1) {
			platform = -1;
			_stage = dist2(game->random);
		}

		std::uniform_int_distribution<size_t> dist3{0, stages[_stage].platforms.size() - 1};
		auto &stage = stages[_stage];

		if (platform == -1)
			platform = dist3(game->random);
		if (leftPos < 0)
			leftPalette = 0;
		if (rightPos < 0)
			rightPalette = 0;
		if (leftPos < 0)
			leftPos = dist(game->random);
		if (rightPos < 0)
			rightPos = dist(game->random);
		if (leftPos == rightPos && entries[leftPos].palettes.size() <= 1) {
			leftPalette = 0;
			rightPalette = 0;
		} else if (
			leftPos == rightPos &&
			entries[leftPos].palettes.size() == 2 &&
			leftPalette == rightPalette
		) {
			leftPalette = 0;
			rightPalette = 1;
		}
		if (
			leftPos == rightPos &&
			leftPalette == rightPalette &&
			entries[leftPos].palettes.size() > 1
		) {
			rightPalette++;
			rightPalette %= entries[leftPos].palettes.size();
		}

		auto &lentry = entries[leftPos];
		auto &rentry = entries[rightPos];
		auto &licon = lentry.icon[leftPalette];
		auto &ricon = rentry.icon[rightPalette];

		if (args->reportProgressW)
			args->reportProgressW(L"Loading P1's character (" + entries[leftPos].name + L")");

		auto lchr = createCharacter(entries[leftPos], entries[rightPos], leftPos, leftPalette, std::move(leftInput));

		if (args->reportProgressW)
			args->reportProgressW(L"Loading P2's character (" + entries[rightPos].name + L")");

		auto rchr = createCharacter(entries[rightPos], entries[leftPos], rightPos, rightPalette, std::move(rightInput));

		return {
			{static_cast<unsigned>(_stage), 0, static_cast<unsigned>(platform)},
			stage.platforms[platform],
			stage,
			lchr,
			rchr,
			licon.textureHandle,
			ricon.textureHandle,
			lentry.entry,
			rentry.entry
		};
	}

	std::pair<std::vector<StageEntry>, std::vector<CharacterEntry>> CharacterSelect::loadData()
	{
		auto chrList = game->getCharacters();
		std::vector<StageEntry> stages;
		std::vector<CharacterEntry> entries;
		nlohmann::json json;

		entries.reserve(chrList.size());
		for (auto &entry : chrList) {
			auto file = entry + "/chr.json";

			game->logger.debug("Loading character from " + file);

			auto data = game->fileMgr.readFull(file);

			json = nlohmann::json::parse(data);
#ifndef _DEBUG
			if (json.contains("hidden") && json["hidden"])
				continue;
#endif
			entries.emplace_back(json, entry);
		}
		std::sort(entries.begin(), entries.end(), [](CharacterEntry &a, CharacterEntry &b){
			return a.pos < b.pos;
		});

		auto data = game->fileMgr.readFull("assets/stages/list.json");

		json = nlohmann::json::parse(data);
		stages.reserve(json.size());
		for (auto &elem: json)
			stages.emplace_back(elem);
		return { stages, entries };
	}

	CharacterEntry::CharacterEntry(const nlohmann::json &json, const std::string &folder) :
		entry(json)
	{
		if (!json.contains("pos"))
			throw std::invalid_argument("pos is missing");
		if (!json.contains("name"))
			throw std::invalid_argument("name is missing");
		if (!json.contains("ground_drag"))
			throw std::invalid_argument("ground_drag is missing");
		if (!json.contains("air_drag"))
			throw std::invalid_argument("air_drag is missing");
		if (!json["air_drag"].contains("x"))
			throw std::invalid_argument("air_drag.x is missing");
		if (!json["air_drag"].contains("y"))
			throw std::invalid_argument("air_drag.y is missing");
		if (!json.contains("hp"))
			throw std::invalid_argument("hp is missing");
		if (!json.contains("guard_bar"))
			throw std::invalid_argument("guard_bar is missing");
		if (!json.contains("guard_break_cooldown"))
			throw std::invalid_argument("guard_break_cooldown is missing");
		if (!json.contains("overdrive_cooldown"))
			throw std::invalid_argument("overdrive_cooldown is missing");
		if (!json.contains("gravity"))
			throw std::invalid_argument("gravity is missing");
		if (!json["gravity"].contains("x"))
			throw std::invalid_argument("gravity.x is missing");
		if (!json["gravity"].contains("y"))
			throw std::invalid_argument("gravity.y is missing");
		if (!json.contains("jump_count"))
			throw std::invalid_argument("jump_count is missing");
		if (!json.contains("air_dash_count"))
			throw std::invalid_argument("air_dash_count is missing");
		if (!json.contains("air_movements"))
			throw std::invalid_argument("air_movements is missing");
		if (!json.contains("mana_max"))
			throw std::invalid_argument("mana_max is missing");
		if (!json.contains("mana_start"))
			throw std::invalid_argument("mana_start is missing");
		if (!json.contains("mana_regen"))
			throw std::invalid_argument("mana_regen is missing");
		if (!json.contains("palettes"))
			throw std::invalid_argument("palettes is missing");
		if (!json.contains("airdrift"))
			throw std::invalid_argument("gravity is missing");
		if (!json["airdrift"].contains("up"))
			throw std::invalid_argument("airdrift.up is missing");
		if (!json["airdrift"].contains("down"))
			throw std::invalid_argument("airdrift.down is missing");
		if (!json["airdrift"].contains("back"))
			throw std::invalid_argument("airdrift.back is missing");
		if (!json["airdrift"].contains("front"))
			throw std::invalid_argument("airdrift.front is missing");
		if (!json["airdrift"]["up"].contains("accel"))
			throw std::invalid_argument("airdrift.up.accel is missing");
		if (!json["airdrift"]["up"].contains("max"))
			throw std::invalid_argument("airdrift.up.max is missing");
		if (!json["airdrift"]["down"].contains("accel"))
			throw std::invalid_argument("airdrift.down.accel is missing");
		if (!json["airdrift"]["down"].contains("max"))
			throw std::invalid_argument("airdrift.down.max is missing");
		if (!json["airdrift"]["back"].contains("accel"))
			throw std::invalid_argument("airdrift.back.accel is missing");
		if (!json["airdrift"]["back"].contains("max"))
			throw std::invalid_argument("airdrift.back.max is missing");
		if (!json["airdrift"]["front"].contains("accel"))
			throw std::invalid_argument("airdrift.front.accel is missing");
		if (!json["airdrift"]["front"].contains("max"))
			throw std::invalid_argument("airdrift.front.max is missing");

		for (auto &j : json["palettes"]) {
			this->palettes.emplace_back();
			for (auto &c : j) {
				if (!c.contains("r"))
					throw std::invalid_argument("c.r is missing");
				if (!c.contains("g"))
					throw std::invalid_argument("c.g is missing");
				if (!c.contains("b"))
					throw std::invalid_argument("c.b is missing");
				this->palettes.back().push_back(Color{
					c["r"],
					c["g"],
					c["b"],
					255
				});
			}
			assert_exp(this->palettes.size() == 1 || this->palettes[this->palettes.size() - 2].size() == this->palettes.back().size());
		}

		this->pos = json["pos"];
		this->name = Utils::utf8ToUtf16(json["name"].get<std::string>());
		//FIXME: Temporary hack
		if (json.contains("class"))
			this->_class = json["class"];
		this->folder = folder;
		this->data = FrameData::loadFile(folder + "/charSelect.json", folder);
		if (this->palettes.empty())
			this->icon.emplace_back(), this->icon.back().textureHandle = game->textureMgr.load(folder + "/icon.png");
		else
			for (auto &palette : this->palettes) {
				this->icon.emplace_back();
				this->icon.back().textureHandle = game->textureMgr.load(
					folder + "/icon.png", {this->palettes[0], palette}
				);
			}
	}

	CharacterEntry::CharacterEntry(const CharacterEntry &entry) :
		entry(entry.entry),
		pos(entry.pos),
		_class(entry._class),
		name(entry.name),
		folder(entry.folder),
		palettes(entry.palettes),
		icon(entry.icon),
		data(entry.data)
	{
		for (auto &_icon : this->icon)
			game->textureMgr.addRef(_icon.textureHandle);
	}

	CharacterEntry::~CharacterEntry()
	{
		for (auto &_icon : this->icon)
			game->textureMgr.remove(_icon.textureHandle);
	}

	PlatformSkeleton::PlatformSkeleton(const nlohmann::json &json) :
		entry(json)
	{
		this->framedata = json["framedata"];
		this->_class = json["class"];
		this->width = json["width"];
		this->hp = json["hp"];
		this->cd = json["cd"];
		this->pos.x = json["pos"]["x"];
		this->pos.y = json["pos"]["y"];
		//TODO
		this->data = FrameData::loadFile(this->framedata, this->framedata.substr(0, this->framedata.find_last_of('/')))[0].front().front();
	}

	StageEntry::StageEntry(const nlohmann::json &json) :
		entry(json)
	{
		this->name = json["name"];
		this->credits = json["credits"];
		this->imagePath = json["image"];
		this->imageHandle = game->textureMgr.load(this->imagePath);
		if (json.contains("objects"))
			this->objectPath = json["objects"];
		for (auto &platformArray : json["platforms"]) {
			this->platforms.emplace_back();
			for (auto &platform : platformArray)
				this->platforms.back().emplace_back(platform);
		}
	}

	StageEntry::StageEntry(const StageEntry &other)
	{
		this->entry = other.entry;
		this->name = other.name;
		this->credits = other.credits;
		this->objectPath = other.objectPath;
		this->imagePath = other.imagePath;
		this->imageHandle = other.imageHandle;
		this->platforms = other.platforms;
		game->textureMgr.addRef(this->imageHandle);
	}

	StageEntry::~StageEntry()
	{
		game->textureMgr.remove(this->imageHandle);
	}
}
