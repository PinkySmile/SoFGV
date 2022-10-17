//
// Created by PinkySmile on 13/09/2022.
//

#ifndef SOFGV_SERVERINGAME_HPP
#define SOFGV_SERVERINGAME_HPP


#include "RollbackMachine.hpp"
#include <Inputs/RemoteInput.hpp>
#include <Scenes/InGame.hpp>

namespace SpiralOfFate
{
	class ServerInGame : public InGame {
	private:
		RollbackMachine _rMachine;
		std::shared_ptr<RemoteInput> _input;
#ifdef _DEBUG
		bool _displayInputs = false;
		Character *_leftChr;
		Character *_rightChr;
#endif

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
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
#ifdef _DEBUG
		void render() const override;
#endif
	};
}


#endif //SOFGV_SERVERINGAME_HPP
