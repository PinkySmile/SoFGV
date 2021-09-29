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
			std::map<unsigned, std::vector<std::vector<FrameData>>> data;

			CharacterEntry(const nlohmann::json &json);
		};

		std::shared_ptr<IInput> _leftInput;
		std::shared_ptr<IInput> _rightInput;
		std::vector<CharacterEntry> _entries;
		int _leftPos = 0;
		int _rightPos = 0;

		ACharacter *_createCharacter(int pos, std::shared_ptr<IInput> input);

	public:
		CharacterSelect(std::shared_ptr<IInput> leftInput, std::shared_ptr<IInput> rightInput);
		void render() const override;
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_CHARACTERSELECT_HPP
