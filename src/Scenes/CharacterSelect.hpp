//
// Created by Gegel85 on 29/09/2021.
//

#ifndef BATTLE_CHARACTERSELECT_HPP
#define BATTLE_CHARACTERSELECT_HPP


#include <memory>
#include <nlohmann/json.hpp>
#include "IScene.hpp"
#include "../Inputs/IInput.hpp"
#include "../Resources/FrameData.hpp"
#include "../Objects/ACharacter.hpp"

namespace Battle
{
	class CharacterSelect : public IScene {
	private:
		struct CharacterEntry {
			nlohmann::json entry;
			int pos;
			unsigned _class;
			std::string name;
			std::string framedataPath;
			Sprite icon;
			std::map<unsigned, std::vector<std::vector<FrameData>>> data;

			CharacterEntry(const nlohmann::json &json);
			CharacterEntry(const CharacterEntry &entry);
			~CharacterEntry();
		};

		sf::Font _font;
		mutable Sprite _randomSprite;
		std::shared_ptr<IInput> _leftInput;
		std::shared_ptr<IInput> _rightInput;
		mutable std::vector<CharacterEntry> _entries;
		int _leftPos = -1;
		int _rightPos = -1;

		ACharacter *_createCharacter(int pos, std::shared_ptr<IInput> input);

	public:
		CharacterSelect(std::shared_ptr<IInput> leftInput, std::shared_ptr<IInput> rightInput);
		void render() const override;
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_CHARACTERSELECT_HPP
