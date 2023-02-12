//
// Created by PinkySmile on 15/08/2022.
//

#ifndef SOFGV_SYNCTESTINGAME_HPP
#define SOFGV_SYNCTESTINGAME_HPP


#include "../PracticeInGame.hpp"
#include "Resources/Network/RollbackMachine.hpp"

namespace SpiralOfFate
{
	class SyncTestInGame : public InGame {
	private:
		RollbackMachine _rollback;

	public:
		SyncTestInGame(
			const GameParams &params,
			const std::vector<struct PlatformSkeleton> &platforms,
			const struct StageEntry &stage,
			Character *leftChr,
			Character *rightChr,
			unsigned licon,
			unsigned ricon,
			const nlohmann::json &lJson,
			const nlohmann::json &rJson
		);

		IScene *update() override;

		void consumeEvent(const sf::Event &event) override;
	};

	InGame *createSyncTestInGameSceneIScene(const InGame::GameParams &params, const std::vector<struct PlatformSkeleton> &platforms, const struct StageEntry &stage, Character *leftChr, Character *rightChr, unsigned licon, unsigned ricon, const nlohmann::json &lJson, const nlohmann::json &rJson);
}


#endif //SOFGV_SYNCTESTINGAME_HPP
