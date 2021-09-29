//
// Created by Gegel85 on 24/09/2021.
//

#ifndef BATTLE_INGAME_HPP
#define BATTLE_INGAME_HPP


#include <memory>
#include "IScene.hpp"
#include "../Inputs/IInput.hpp"
#include "../Objects/ACharacter.hpp"

namespace Battle
{
	class InGame : public IScene {
	private:
		std::shared_ptr<IInput> leftInput;
		std::shared_ptr<IInput> rightInput;

	public:
		InGame(ACharacter *leftChr, ACharacter *rightChr, const nlohmann::json &lJson, const nlohmann::json &rJson);
		void render() const override;
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_INGAME_HPP
