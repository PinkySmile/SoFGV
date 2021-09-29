//
// Created by Gegel85 on 28/09/2021.
//

#ifndef BATTLE_NETPLAYGAME_HPP
#define BATTLE_NETPLAYGAME_HPP


#include "IScene.hpp"
#include "../Inputs/IInput.hpp"
#include "../Inputs/RemoteInput.hpp"
#include "../Inputs/NetworkInput.hpp"

namespace Battle
{
	class NetplayGame : public IScene {
	private:
		RemoteInput *_remote;
		NetworkInput *_input;
		bool _hosts;

	public:
		NetplayGame(RemoteInput *remote, NetworkInput *input, IInput *leftInput, IInput *rightInput);
		void render() const override;
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_NETPLAYGAME_HPP
