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

		auto &leftSprite  = this->_entries[this->_leftPos == -1  ? dist(game.random) : this->_leftPos ].icon;
		auto &rightSprite = this->_entries[this->_rightPos == -1 ? dist(game.random) : this->_rightPos].icon;
		auto leftTexture = game.textureMgr.getTextureSize(leftSprite.textureHandle);
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

		if (lInputs.n == 1) {
			std::uniform_int_distribution<size_t> dist{0, this->_entries.size() - 1};

			if (this->_leftPos < 0)
				this->_leftPos = dist(game.random);
			if (this->_rightPos < 0)
				this->_rightPos = dist(game.random);
			return new InGame(
				this->_createCharacter(this->_leftPos, this->_leftInput),
				this->_createCharacter(this->_rightPos, this->_rightInput),
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

	ACharacter *CharacterSelect::_createCharacter(int pos, std::shared_ptr<IInput> input)
	{
		auto &entry = this->_entries[pos];

		switch (entry._class) {
		default:
			return new ACharacter{
				entry.framedataPath,
				std::move(input)
			};
		}
	}

	CharacterSelect::CharacterEntry::CharacterEntry(const nlohmann::json &json) :
		entry(json)
	{
		if (!json.contains("pos"))
			throw std::invalid_argument("pos");
		if (!json.contains("class"))
			throw std::invalid_argument("class");
		if (!json.contains("name"))
			throw std::invalid_argument("name");
		if (!json.contains("framedata"))
			throw std::invalid_argument("framedata");
		if (!json.contains("framedata_char_select"))
			throw std::invalid_argument("framedata_char_select");
		if (!json.contains("hp"))
			throw std::invalid_argument("hp");
		if (!json.contains("gravity"))
			throw std::invalid_argument("gravity");
		if (!json["gravity"].contains("x"))
			throw std::invalid_argument("gravity.x");
		if (!json["gravity"].contains("y"))
			throw std::invalid_argument("gravity.y");
		if (!json.contains("jump_count"))
			throw std::invalid_argument("jump_count");
		if (!json.contains("icon"))
			throw std::invalid_argument("icon");

		this->pos = json["pos"];
		this->_class = json["class"];
		this->name = json["name"];
		this->framedataPath = json["framedata"];
		this->data = FrameData::loadFile(json["framedata_char_select"]);
		this->icon.textureHandle = game.textureMgr.load(json["icon"]);
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
		game.textureMgr.addRef(this->icon.textureHandle);
	}

	CharacterSelect::CharacterEntry::~CharacterEntry()
	{
		game.textureMgr.remove(this->icon.textureHandle);
	}
}