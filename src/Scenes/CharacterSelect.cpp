//
// Created by Gegel85 on 29/09/2021.
//

#include <utility>
#include <fstream>
#include "CharacterSelect.hpp"
#include "InGame.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"

namespace Battle
{
	CharacterSelect::CharacterSelect(std::shared_ptr<IInput> leftInput, std::shared_ptr<IInput> rightInput)	:
		_leftInput(std::move(leftInput)),
		_rightInput(std::move(rightInput))
	{
		sf::View view{{0, 0, 1680, 960}};
		std::ifstream stream{"assets/characters/list.json"};
		nlohmann::json json;

		this->_font.loadFromFile("assets/fonts/comicsansms.ttf");
		game.screen->setView(view);
		logger.info("CharacterSelect scene created");
		stream >> json;
		for (auto &elem : json)
			this->_entries.emplace_back(elem);
		std::sort(this->_entries.begin(), this->_entries.end(), [](const CharacterEntry &e1, const CharacterEntry &e2){
			return e1.pos < e2.pos;
		});
	}

	void CharacterSelect::render() const
	{
		std::uniform_int_distribution<size_t> dist{0, this->_entries.size() - 1};
		size_t left  = this->_leftPos == -1  ? dist(game.random) : this->_leftPos;
		size_t right = this->_rightPos == -1 ? dist(game.random) : this->_rightPos;
		auto lInputs = this->_leftInput->getInputs();
		auto rInputs = this->_rightInput->getInputs();

		game.screen->fillColor(sf::Color::Black);
		game.screen->setFont(this->_font);
		game.screen->displayElement({0, 0, 1680, 960}, sf::Color{
			static_cast<sf::Uint8>(lInputs.d),
			static_cast<sf::Uint8>(rInputs.d),
			static_cast<sf::Uint8>((lInputs.d + rInputs.d) / 2)
		});
		game.screen->displayElement({0, 0, 560, 480}, sf::Color{0xA0, 0xA0, 0xA0, 0xFF});
		game.screen->displayElement({0, 480, 560, 480}, sf::Color::White);

		game.screen->displayElement({1120, 0, 560, 480}, sf::Color{0xA0, 0xA0, 0xA0, 0xFF});
		game.screen->displayElement({1120, 480, 560, 480}, sf::Color::White);

		game.screen->displayElement(this->_leftPos == -1 ? "Random select" : this->_entries[this->_leftPos].name, {0, 480}, 560, Screen::ALIGN_CENTER);
		game.screen->displayElement(this->_rightPos == -1 ? "Random select" : this->_entries[this->_rightPos].name, {1120, 480}, 560, Screen::ALIGN_CENTER);

		auto &leftSprites  = this->_entries[left].icon;
		auto &rightSprites = this->_entries[right].icon;
		std::uniform_int_distribution<size_t> ldist{0, this->_entries[left].palettes.size() - 1};
		std::uniform_int_distribution<size_t> rdist{0, this->_entries[right].palettes.size() - 1};
		int realLPal = this->_leftPos == -1  ? -1 : this->_leftPalette;
		int realRPal = this->_rightPos == -1 ? -1 : this->_rightPalette;
		size_t lpal = realLPal == -1 ? ldist(game.random) : this->_leftPalette;
		size_t rpal = realRPal == -1 ? ldist(game.random) : this->_rightPalette;
		auto &leftSprite  = leftSprites[lpal];
		auto &rightSprite = rightSprites[rpal];
		auto leftTexture  = game.textureMgr.getTextureSize(leftSprite.textureHandle);
		auto rightTexture = game.textureMgr.getTextureSize(rightSprite.textureHandle);

		leftSprite.setPosition(0, 0);
		leftSprite.setScale(560.f / leftTexture.x, 480.f / leftTexture.y);
		game.textureMgr.render(leftSprite);

		rightSprite.setPosition(1680, 0);
		rightSprite.setScale(-560.f / rightTexture.x, 480.f / rightTexture.y);
		game.textureMgr.render(rightSprite);
	}

	IScene *CharacterSelect::update()
	{
		this->_leftInput->update();
		this->_rightInput->update();

		auto lInputs = this->_leftInput->getInputs();
		auto rInputs = this->_rightInput->getInputs();

		if (lInputs.horizontalAxis == -1) {
			if (this->_leftPos == -1)
				this->_leftPos = static_cast<int>(this->_entries.size());
			this->_leftPos--;
		} else if (lInputs.horizontalAxis == 1) {
			this->_leftPos++;
			if (this->_leftPos == static_cast<int>(this->_entries.size()))
				this->_leftPos = -1;
		}

		if (rInputs.horizontalAxis == -1) {
			if (this->_rightPos == -1)
				this->_rightPos = static_cast<int>(this->_entries.size());
			this->_rightPos--;
		} else if (rInputs.horizontalAxis == 1) {
			this->_rightPos++;
			if (this->_rightPos == static_cast<int>(this->_entries.size()))
				this->_rightPos = -1;
		}

		if (this->_leftPos >= 0) {
			if (lInputs.verticalAxis == -1) {
				if (this->_leftPalette == -1)
					this->_leftPalette = static_cast<int>(this->_entries[this->_leftPos].palettes.size());
				this->_leftPalette--;
			} else if (lInputs.verticalAxis == 1) {
				this->_leftPalette++;
				if (this->_leftPalette == static_cast<int>(this->_entries[this->_leftPos].palettes.size()))
					this->_leftPalette = -1;
			}
		}
		if (this->_rightPos >= 0) {
			if (rInputs.verticalAxis == -1) {
				if (this->_rightPalette == -1)
					this->_rightPalette = static_cast<int>(this->_entries[this->_rightPos].palettes.size());
				this->_rightPalette--;
			} else if (rInputs.verticalAxis == 1) {
				this->_rightPalette++;
				if (this->_rightPalette == static_cast<int>(this->_entries[this->_rightPos].palettes.size()))
					this->_rightPalette = -1;
			}
		}

		if (lInputs.n == 1) {
			std::uniform_int_distribution<size_t> dist{0, this->_entries.size() - 1};

			if (this->_leftPos < 0)
				this->_leftPalette = -1;
			if (this->_rightPos < 0)
				this->_rightPalette = -1;
			if (this->_leftPos < 0)
				this->_leftPos = dist(game.random);
			if (this->_rightPos < 0)
				this->_rightPos = dist(game.random);
			if (this->_leftPos == this->_rightPos && this->_entries[this->_leftPos].palettes.size() <= 1) {
				this->_leftPalette = 0;
				this->_rightPalette = 0;
			} else if (this->_leftPos == this->_rightPos && this->_entries[this->_leftPos].palettes.size() == 2 && this->_leftPalette == this->_rightPalette) {
				this->_leftPalette = 0;
				this->_rightPalette = 1;
			} else {
				if (this->_leftPalette < 0) {
					std::uniform_int_distribution<size_t> dist2{0, this->_entries[this->_leftPos].palettes.size() - 1};

					this->_leftPalette = dist2(game.random);
				}
				if (this->_rightPalette < 0) {
					std::uniform_int_distribution<size_t> dist2{0, this->_entries[this->_rightPos].palettes.size() - 1};

					this->_rightPalette = dist2(game.random);
				}
			}
			if (this->_leftPos == this->_rightPos && this->_leftPalette == this->_rightPalette && this->_entries[this->_leftPos].palettes.size() > 1) {
				this->_rightPalette++;
				this->_rightPalette %= this->_entries[this->_leftPos].palettes.size();
			}

			auto lchr = this->_createCharacter(this->_leftPos,  this->_leftPalette,  this->_leftInput);
			auto rchr = this->_createCharacter(this->_rightPos, this->_rightPalette, this->_rightInput);

			return new InGame(
				lchr,
				rchr,
				this->_entries[this->_leftPos].entry,
				this->_entries[this->_rightPos].entry
			);
		}
		return nullptr;
	}

	void CharacterSelect::consumeEvent(const sf::Event &event)
	{
		this->_leftInput->consumeEvent(event);
		this->_rightInput->consumeEvent(event);
	}

	ACharacter *CharacterSelect::_createCharacter(int pos, int palette, std::shared_ptr<IInput> input)
	{
		auto &entry = this->_entries[pos];
		std::pair<std::vector<Color>, std::vector<Color>> palettes;

		if (!entry.palettes.empty() && palette) {
			palettes.first = entry.palettes.front();
			palettes.second = entry.palettes[palette];
		}
		switch (entry._class) {
		default:
			return new ACharacter{
				entry.framedataPath,
				palettes,
				std::move(input)
			};
		}
	}

	CharacterSelect::CharacterEntry::CharacterEntry(const nlohmann::json &json) :
		entry(json)
	{
		if (!json.contains("pos"))
			throw std::invalid_argument("pos is missing");
		if (!json.contains("class"))
			throw std::invalid_argument("class is missing");
		if (!json.contains("name"))
			throw std::invalid_argument("name is missing");
		if (!json.contains("framedata"))
			throw std::invalid_argument("framedata is missing");
		if (!json.contains("framedata_char_select"))
			throw std::invalid_argument("framedata_char_select is missing");
		if (!json.contains("hp"))
			throw std::invalid_argument("hp is missing");
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
			assert(this->palettes.size() == 1 || this->palettes[this->palettes.size() - 2].size() == this->palettes.back().size());
		}

		this->pos = json["pos"];
		this->_class = json["class"];
		this->name = json["name"];
		this->framedataPath = json["framedata"];
		this->data = FrameData::loadFile(json["framedata_char_select"]);
		if (this->palettes.empty())
			this->icon.emplace_back(), this->icon.back().textureHandle = game.textureMgr.load(json["icon"]);
		else {
			for (auto &palette : this->palettes)
				this->icon.emplace_back(), this->icon.back().textureHandle = game.textureMgr.load(json["icon"], {this->palettes[0], palette});
		}
	}

	CharacterSelect::CharacterEntry::CharacterEntry(const CharacterSelect::CharacterEntry &entry) :
		entry(entry.entry),
		pos(entry.pos),
		_class(entry._class),
		name(entry.name),
		framedataPath(entry.framedataPath),
		icon(entry.icon),
		data(entry.data)
	{
		for (auto &_icon : this->icon)
			game.textureMgr.addRef(_icon.textureHandle);
	}

	CharacterSelect::CharacterEntry::~CharacterEntry()
	{
		for (auto &_icon : this->icon)
			game.textureMgr.remove(_icon.textureHandle);
	}
}
