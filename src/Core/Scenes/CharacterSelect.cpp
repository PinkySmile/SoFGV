//
// Created by Gegel85 on 29/09/2021.
//

#include <utility>
#include <fstream>
#include "CharacterSelect.hpp"
#include "InGame.hpp"
#include "../Resources/Game.hpp"
#include "PracticeInGame.hpp"
#include "../Objects/Characters/Stickman.hpp"
#include "TitleScreen.hpp"

namespace SpiralOfFate
{
	CharacterSelect::CharacterSelect(std::shared_ptr<IInput> leftInput, std::shared_ptr<IInput> rightInput, bool practice)	:
		_leftInput(std::move(leftInput)),
		_rightInput(std::move(rightInput)),
		_practice(practice)
	{
		sf::View view{{0, 0, 1680, 960}};
		std::ifstream stream{"assets/characters/list.json"};
		std::ifstream stream2{"assets/stages/list.json"};
		nlohmann::json json;
		nlohmann::json json2;

		game->screen->setView(view);
		game->logger.info("CharacterSelect scene created");
		stream >> json;
		for (auto &elem : json)
			this->_entries.emplace_back(elem);
		stream2 >> json2;
		for (auto &elem: json2)
			this->_stages.emplace_back(elem);
		this->_randomSprite.textureHandle = game->textureMgr.load("assets/stages/random.png");
	}

	CharacterSelect::CharacterSelect(std::shared_ptr<IInput> leftInput, std::shared_ptr<IInput> rightInput, int _leftPos, int _rightPos, int _leftPalette, int _rightPalette, bool practice) :
		CharacterSelect(std::move(leftInput), std::move(rightInput), practice)
	{
		this->_leftPos = _leftPos;
		this->_rightPos = _rightPos;
		this->_leftPalette = _leftPalette;
		this->_rightPalette = _rightPalette;
	}

	CharacterSelect::~CharacterSelect()
	{
		game->textureMgr.remove(this->_randomSprite.textureHandle);
	}

	void CharacterSelect::render() const
	{
		this->_selectingStage ? this->_selectStageRender() : this->_selectCharacterRender();
	}

	IScene *CharacterSelect::update()
	{
		this->_leftInput->update();
		this->_rightInput->update();

		if (this->_quit)
			return new TitleScreen(game->P1, game->P2);
		return this->_selectingStage ? this->_selectStageUpdate() : this->_selectCharacterUpdate();
	}

	void CharacterSelect::consumeEvent(const sf::Event &event)
	{
		this->_leftInput->consumeEvent(event);
		this->_rightInput->consumeEvent(event);
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
			this->_quit = true;
	}

	Character *CharacterSelect::_createCharacter(int pos, int palette, std::shared_ptr<IInput> input)
	{
		return createCharacter(this->_entries[pos], pos, palette, std::move(input));
	}

	Character *CharacterSelect::createCharacter(const CharacterEntry &entry, int pos, int palette, std::shared_ptr<IInput> input)
	{
		Character *chr;
		std::pair<std::vector<Color>, std::vector<Color>> palettes;

		if (!entry.palettes.empty() && palette) {
			palettes.first = entry.palettes.front();
			palettes.second = entry.palettes[palette];
		}
		switch (entry._class) {
		case 0:
			chr = new Stickman{
				static_cast<unsigned>(palette << 16 | pos),
				entry.framedataPath,
				entry.subobjectDataPath,
				palettes,
				std::move(input)
			};
			break;
		default:
			chr = new Character{
				static_cast<unsigned>(palette << 16 | pos),
				entry.framedataPath,
				entry.subobjectDataPath,
				palettes,
				std::move(input)
			};
			break;
		}

		chr->name = entry.name;
		return chr;
	}

	InGame *CharacterSelect::_launchGame()
	{
		std::uniform_int_distribution<size_t> dist{0, this->_entries.size() - 1};
		std::uniform_int_distribution<size_t> dist2{0, this->_stages.size() - 1};

		if (this->_stage == -1) {
			this->_platform = -1;
			this->_stage = dist2(game->battleRandom);
		}

		std::uniform_int_distribution<size_t> dist3{0, this->_stages[this->_stage].platforms.size() - 1};
		auto &stage = this->_stages[this->_stage];

		if (this->_platform == -1)
			this->_platform = dist3(game->battleRandom);
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
		} else if (this->_leftPos == this->_rightPos && this->_entries[this->_leftPos].palettes.size() == 2 && this->_leftPalette == this->_rightPalette) {
			this->_leftPalette = 0;
			this->_rightPalette = 1;
		}
		if (this->_leftPos == this->_rightPos && this->_leftPalette == this->_rightPalette && this->_entries[this->_leftPos].palettes.size() > 1) {
			this->_rightPalette++;
			this->_rightPalette %= this->_entries[this->_leftPos].palettes.size();
		}
		game->soundMgr.play(BASICSOUND_GAME_LAUNCH);

		auto lchr = this->_createCharacter(this->_leftPos,  this->_leftPalette,  this->_leftInput);
		auto rchr = this->_createCharacter(this->_rightPos, this->_rightPalette, this->_rightInput);

		if (this->_practice)
			return new PracticeInGame(
				{static_cast<unsigned>(this->_stage), 0, static_cast<unsigned>(this->_platform)},
				stage.platforms[this->_platform],
				stage,
				lchr,
				rchr,
				this->_entries[this->_leftPos].entry,
				this->_entries[this->_rightPos].entry
			);
		return new InGame(
			{static_cast<unsigned>(this->_stage), 0, static_cast<unsigned>(this->_platform)},
			stage.platforms[this->_platform],
			stage,
			lchr,
			rchr,
			this->_entries[this->_leftPos].entry,
			this->_entries[this->_rightPos].entry
		);
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

		game->screen->displayElement(this->_leftPos == -1 ? "Random select" : this->_entries[this->_leftPos].name, {0, 480}, 560, Screen::ALIGN_CENTER);
		game->screen->displayElement(this->_rightPos == -1 ? "Random select" : this->_entries[this->_rightPos].name, {1120, 480}, 560, Screen::ALIGN_CENTER);

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

	IScene *CharacterSelect::_selectCharacterUpdate()
	{
		auto lInputs = this->_leftInput->getInputs();
		auto rInputs = this->_rightInput->getInputs();

		game->battleRandom();
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
				} while (this->_entries[this->_leftPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette);
			} else if (lInputs.verticalAxis == 1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				do {
					this->_leftPalette++;
					if (this->_leftPalette == static_cast<int>(this->_entries[this->_leftPos].palettes.size()))
						this->_leftPalette = 0;
				} while (this->_entries[this->_leftPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette);
			}
		}
		if (this->_rightPos >= 0) {
			if (rInputs.verticalAxis == -1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				do {
					if (this->_rightPalette == 0)
						this->_rightPalette = static_cast<int>(this->_entries[this->_rightPos].palettes.size());
					this->_rightPalette--;
				} while (this->_entries[this->_rightPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette);
			} else if (rInputs.verticalAxis == 1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				do {
					this->_rightPalette++;
					if (this->_rightPalette == static_cast<int>(this->_entries[this->_rightPos].palettes.size()))
						this->_rightPalette = 0;
				} while (this->_entries[this->_rightPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette);
			}
		}

		if (lInputs.n == 1 || rInputs.n == 1) {
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			this->_selectingStage = true;
			this->_stageSprite.textureHandle = this->_stages[0].imageHandle;
		}
		return nullptr;
	}

	IScene *CharacterSelect::_selectStageUpdate()
	{
		auto lInputs = this->_leftInput->getInputs();
		auto rInputs = this->_rightInput->getInputs();

		game->battleRandom();
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
		return nullptr;
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
				static_cast<float>(platform.data.size.x) / platform.data.textureBounds.size.x,
				static_cast<float>(platform.data.size.y) / platform.data.textureBounds.size.y
			};
			auto result = platform.data.offset + platform.pos;

			result.y *= -1;
			result += Vector2f{
				platform.data.size.x / -2.f,
				-static_cast<float>(platform.data.size.y)
			};
			result += Vector2f{
				platform.data.textureBounds.size.x * scale2.x / 2,
				platform.data.textureBounds.size.y * scale2.y / 2
			};
			sprite.setOrigin(platform.data.textureBounds.size / 2.f);
			sprite.setPosition({(result.x + 50) * scale.x, (result.y + 600) * scale.y});
			sprite.setScale({scale.x * scale2.x, scale.y * scale2.y});
			sprite.textureHandle = platform.data.textureHandle;
			sprite.setTextureRect(platform.data.textureBounds);
			game->textureMgr.render(sprite);
		}
	}

	CharacterEntry::CharacterEntry(const nlohmann::json &json) :
		entry(json)
	{
		if (!json.contains("pos"))
			throw std::invalid_argument("pos is missing");
		if (!json.contains("class"))
			throw std::invalid_argument("class is missing");
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
		if (!json.contains("framedata"))
			throw std::invalid_argument("framedata is missing");
		if (!json.contains("subobjects"))
			throw std::invalid_argument("subobjects is missing");
		if (!json.contains("framedata_char_select"))
			throw std::invalid_argument("framedata_char_select is missing");
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
		if (!json.contains("void_mana_max"))
			throw std::invalid_argument("void_mana_max is missing");
		if (!json.contains("spirit_mana_max"))
			throw std::invalid_argument("spirit_mana_max is missing");
		if (!json.contains("matter_mana_max"))
			throw std::invalid_argument("matter_mana_max is missing");
		if (!json.contains("mana_regen"))
			throw std::invalid_argument("mana_regen is missing");
		if (!json.contains("icon"))
			throw std::invalid_argument("icon is missing");
		if (!json.contains("palettes"))
			throw std::invalid_argument("palettes is missing");

		for (auto &j : json["palettes"]) {
			this->palettes.emplace_back();
			for (auto &c : j) {
				if (!c.contains("r"))
					throw std::invalid_argument("c.r is missing");
				if (!c.contains("g"))
					throw std::invalid_argument("c.g is missing");
				if (!c.contains("b"))
					throw std::invalid_argument("c.b is missing");
				this->palettes.back().push_back(Color{{
					c["r"],
					c["g"],
					c["b"],
					255
				}});
			}
			my_assert(this->palettes.size() == 1 || this->palettes[this->palettes.size() - 2].size() == this->palettes.back().size());
		}

		this->pos = json["pos"];
		this->_class = json["class"];
		this->name = json["name"];
		this->framedataPath = json["framedata"];
		this->subobjectDataPath = json["subobjects"];
		this->data = FrameData::loadFile(json["framedata_char_select"]);
		for (size_t i = 0; i < this->name.size(); i++) {
			if ((this->name[i] & 0b11100000) == 0b11000000) {
				int c = ((this->name[i] & 0b00011111) << 6) | (this->name[i + 1] & 0b00111111);

				if (c <= 255)
					this->name = this->name.substr(0, i) + (char)c + this->name.substr(i + 2);
				else
					this->name = this->name.substr(0, i) + '?' + this->name.substr(i + 2);
			} else if ((this->name[i] & 0b11110000) == 0b11100000)
				this->name = this->name.substr(0, i) + '?' + this->name.substr(i + 3);
			else if ((this->name[i] & 0b11111000) == 0b11110000)
				this->name = this->name.substr(0, i) + '?' + this->name.substr(i + 4);
		}
		if (this->palettes.empty())
			this->icon.emplace_back(), this->icon.back().textureHandle = game->textureMgr.load(json["icon"]);
		else {
			for (auto &palette : this->palettes)
				this->icon.emplace_back(), this->icon.back().textureHandle = game->textureMgr.load(json["icon"], {this->palettes[0], palette});
		}
	}

	CharacterEntry::CharacterEntry(const CharacterEntry &entry) :
		entry(entry.entry),
		pos(entry.pos),
		_class(entry._class),
		name(entry.name),
		framedataPath(entry.framedataPath),
		subobjectDataPath(entry.subobjectDataPath),
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
		this->data = FrameData::loadFile(this->framedata)[0].front().front();
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
