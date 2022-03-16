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
#include "../Objects/Character.hpp"
#include "InGame.hpp"

namespace Battle
{
	struct CharacterEntry {
		nlohmann::json entry;
		int pos;
		unsigned _class;
		std::string name;
		std::string framedataPath;
		std::string subobjectDataPath;
		std::vector<std::vector<Color>> palettes;
		std::vector<Sprite> icon;
		std::map<unsigned, std::vector<std::vector<FrameData>>> data;

		CharacterEntry(const nlohmann::json &json);
		CharacterEntry(const CharacterEntry &entry);
		~CharacterEntry();
	};

	struct PlatformSkeleton {
		nlohmann::json entry;
		std::string framedata;
		FrameData data;
		unsigned _class;
		float width;
		unsigned hp;
		unsigned cd;
		Vector2f pos;

		PlatformSkeleton(const nlohmann::json &json);
	};

	struct StageEntry {
		nlohmann::json entry;
		std::string name;
		std::string credits;
		std::string objectPath;
		std::string imagePath;
		unsigned imageHandle;
		std::vector<std::vector<PlatformSkeleton>> platforms;

		StageEntry(const StageEntry &other);
		StageEntry(const nlohmann::json &json);
		~StageEntry();
	};

	class CharacterSelect : public IScene {
	protected:
		sf::Font _font;
		mutable Sprite _stageSprite;
		mutable Sprite _randomSprite;
		std::shared_ptr<IInput> _leftInput;
		std::shared_ptr<IInput> _rightInput;
		mutable std::vector<StageEntry> _stages;
		mutable std::vector<CharacterEntry> _entries;
		bool _practice = false;

		//Game State
		int _leftPos = 0;
		int _rightPos = 0;
		int _leftPalette = 0;
		int _rightPalette = 1;
		int _stage = 0;
		int _platform = 0;
		bool _selectingStage = false;

		Character *_createCharacter(int pos, int palette, std::shared_ptr<IInput> input);
		virtual InGame *_launchGame();
		void _selectCharacterRender() const;
		void _selectStageRender() const;
		IScene *_selectCharacterUpdate();
		IScene *_selectStageUpdate();
		void _displayPlatformPreview() const;

	public:
		static Character *createCharacter(const CharacterEntry &entry, int pos, int palette, std::shared_ptr<IInput> input);
		CharacterSelect(std::shared_ptr<IInput> leftInput, std::shared_ptr<IInput> rightInput, bool practice = false);
		CharacterSelect(std::shared_ptr<IInput> leftInput, std::shared_ptr<IInput> rightInput, int leftPos, int rightPos, int leftPalette, int rightPalette, bool practice = false);
		~CharacterSelect();
		void render() const override;
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_CHARACTERSELECT_HPP
