//
// Created by PinkySmile on 13/09/2022.
//

#ifndef SOFGV_SERVERINGAME_HPP
#define SOFGV_SERVERINGAME_HPP


#include "Resources/Network/RollbackMachine.hpp"
#include "NetworkInGame.hpp"
#include "Inputs/RemoteInput.hpp"
#include "../InGame.hpp"

namespace SpiralOfFate
{
	class ServerInGame : public NetworkInGame {
	protected:
		void _onGameEnd() override;

	public:
		ServerInGame(
			std::shared_ptr<RemoteInput> input,
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

		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //SOFGV_SERVERINGAME_HPP
