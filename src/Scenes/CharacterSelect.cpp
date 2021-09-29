//
// Created by Gegel85 on 29/09/2021.
//

#include <utility>
#include <fstream>
#include "CharacterSelect.hpp"
#include "InGame.hpp"

namespace Battle
{
	CharacterSelect::CharacterSelect(std::shared_ptr<IInput> leftInput, std::shared_ptr<IInput> rightInput)	:
		_leftInput(std::move(leftInput)),
		_rightInput(std::move(rightInput))
	{
		std::ifstream stream{"assets/characters/list.json"};
		nlohmann::json json;

		stream >> json;
		for (auto &elem : json)
			this->_entries.emplace_back(elem);
	}

	void CharacterSelect::render() const
	{
	}

	IScene *CharacterSelect::update()
	{
		this->_leftInput->update();
		this->_rightInput->update();

		auto lInputs = this->_leftInput->getInputs();
		auto rInputs = this->_rightInput->getInputs();

		if (lInputs.n == 1)
			return new InGame(
				this->_createCharacter(this->_leftPos,  this->_leftInput),
				this->_createCharacter(this->_rightPos, this->_rightInput),
				this->_entries[this->_leftPos].entry,
				this->_entries[this->_rightPos].entry
			);
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
		if (!json.contains("air_jump_count"))
			throw std::invalid_argument("air_jump_count");

		this->pos = json["pos"];
		this->_class = json["class"];
		this->name = json["name"];
		this->framedataPath = json["framedata"];
		this->data = FrameData::loadFile(json["framedata_char_select"]);
	}
}
