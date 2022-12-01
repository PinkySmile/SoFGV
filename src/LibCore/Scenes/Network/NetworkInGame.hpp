//
// Created by PinkySmile on 11/11/2022.
//

#ifndef SOFGV_NETWORKINGAME_HPP
#define SOFGV_NETWORKINGAME_HPP


#include "RollbackMachine.hpp"
#include "Inputs/DelayInput.hpp"
#include <Inputs/RemoteInput.hpp>
#include <Scenes/InGame.hpp>

namespace SpiralOfFate
{
	class NetworkInGame : public InGame {
	protected:
		DelayInput *_leftDInput;
		DelayInput *_rightDInput;
		RollbackMachine _rMachine;
		std::shared_ptr<RemoteInput> _input;
#ifdef _DEBUG
		unsigned _currentFrame = 0;
		bool _displayInputs = false;
		Character *_leftChr;
		Character *_rightChr;
#endif
		virtual void _onGameEnd() = 0;

	public:
		NetworkInGame(
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
		void render() const override;
	};
}


#endif //SOFGV_NETWORKINGAME_HPP
