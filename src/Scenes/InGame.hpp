//
// Created by Gegel85 on 24/09/2021.
//

#ifndef BATTLE_INGAME_HPP
#define BATTLE_INGAME_HPP


#include "IScene.hpp"

namespace Battle
{
	class InGame : public IScene {
	public:
		InGame();
		void render() const override;
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_INGAME_HPP
