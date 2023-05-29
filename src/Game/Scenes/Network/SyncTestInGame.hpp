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
		bool _step = false;
		bool _displayInputs = false;
		Character *_leftChr;
		Character *_rightChr;

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

		void update() override;
		void consumeEvent(const sf::Event &event) override;

		static SyncTestInGame *create(SceneArguments *);

		void render() const override;
	};
}


#endif //SOFGV_SYNCTESTINGAME_HPP
