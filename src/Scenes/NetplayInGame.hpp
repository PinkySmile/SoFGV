//
// Created by Gegel85 on 28/09/2021.
//

#ifndef BATTLE_NETPLAYINGAME_HPP
#define BATTLE_NETPLAYINGAME_HPP


#include "InGame.hpp"
#include "../Inputs/IInput.hpp"
#include "../Inputs/RemoteInput.hpp"
#include "../Objects/ACharacter.hpp"
#include "NetplayScene.hpp"

namespace Battle
{
	class NetplayInGame : public InGame, public NetplayScene {
	private:
		void _saveState(void *data, int *len) override;
		void _loadState(void *data) override;
		IScene *_realUpdate() override;

	public:
		NetplayInGame(ACharacter *leftChr, ACharacter *rightChr, const nlohmann::json &lJson, const nlohmann::json &rJson);
		~NetplayInGame();
		IScene *update() override;
		void render() const override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_NETPLAYINGAME_HPP
