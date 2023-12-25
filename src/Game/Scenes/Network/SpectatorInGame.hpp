//
// Created by PinkySmile on 20/03/2022.
//

#ifndef SOFGV_SPECTATORINGAME_HPP
#define SOFGV_SPECTATORINGAME_HPP


#include "Scenes/PracticeInGame.hpp"
#include "Scenes/CharacterSelect.hpp"
#include "Resources/Network/SpectatorInputManager.hpp"

namespace SpiralOfFate
{
	class SpectatorInGame : public PracticeInGame {
	protected:
		constexpr static const char *_menuStrings[] = {
			"Resume",
			"P1 Move list",
			"P2 Move list",
			"Replay options",
			"Return to title screen"
		};
		constexpr static const char *_practiceMenuStrings[] = {
			"Hitboxes: %s",
			"Debug: %s",
			"Input display: %s",
		};

		SpectatorConnection *_connection;
		std::shared_ptr<SpectatorInputManager> _inputManager;
		Character *_chr;

		void _pauseUpdate() override;
		void _practiceUpdate() override;
		void _renderPause() const override;
		bool _pauseConfirm() override;
		void _practiceRender() const override;
		bool _practiceConfirm() override;

	public:
		struct Arguments : public SceneArguments {
			CharacterEntry lentry;
			int lpos;
			int lpalette;
			std::shared_ptr<IInput> linput;
			CharacterEntry rentry;
			int rpos;
			int rpalette;
			std::shared_ptr<IInput> rinput;
			GameParams params;
			unsigned frameCount;
			std::vector<struct PlatformSkeleton> platforms;
			StageEntry stage;
			unsigned licon;
			unsigned ricon;
			nlohmann::json lJson;
			nlohmann::json rJson;

			Arguments(CharacterEntry lentry, CharacterEntry rentry, StageEntry stage);
		};

		SpectatorInGame(
			const GameParams &params,
			const std::vector<struct PlatformSkeleton> &platforms,
			const struct StageEntry &stage,
			Character *leftChr, Character *rightChr,
			unsigned licon, unsigned ricon,
			const nlohmann::json &lJson, const nlohmann::json &rJson,
			std::shared_ptr<SpectatorInputManager> inputManager,
			SpectatorConnection *connection
		);
		void update() override;
		void consumeEvent(const sf::Event &event) override;
		void render() const override;

		static SpectatorInGame *create(SceneArguments *);
	};
}


#endif //SOFGV_SPECTATORINGAME_HPP
