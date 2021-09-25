//
// Created by Gegel85 on 24/09/2021.
//

#include "InGame.hpp"
#include "../Objects/ACharacter.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"
#include "../Inputs/KeyboardInput.hpp"
#include "../Inputs/ControllerInput.hpp"

namespace Battle
{
	InGame::InGame()
	{
		sf::View view{{-50, -500, 1100, 618.75}};

		logger.info("InGame scene created");
		Battle::game.screen->setView(view);
		game.battleMgr = std::make_unique<BattleManager>(
			new ACharacter{
				"assets/characters/test/framedata.json",
				new KeyboardInput({
					{ sf::Keyboard::Left,  INPUT_LEFT },
					{ sf::Keyboard::Right, INPUT_RIGHT },
					{ sf::Keyboard::Up,    INPUT_UP },
					{ sf::Keyboard::Down,  INPUT_DOWN },
					{ sf::Keyboard::W,     INPUT_NEUTRAL },
					{ sf::Keyboard::X,     INPUT_MATTER },
					{ sf::Keyboard::C,     INPUT_SPIRIT },
					{ sf::Keyboard::V,     INPUT_VOID },
					{ sf::Keyboard::B,     INPUT_ASCEND }
				})
			},
			new ACharacter{
				"assets/characters/test/framedata.json",
				new ControllerInput({
					{ INPUT_LEFT, new ControllerAxis(0, sf::Joystick::Axis::X, -30) },
					{ INPUT_RIGHT, new ControllerAxis(0, sf::Joystick::Axis::X, 30) },
					{ INPUT_UP, new ControllerAxis(0, sf::Joystick::Axis::Y, -30) },
					{ INPUT_DOWN, new ControllerAxis(0, sf::Joystick::Axis::Y, 30) },
					{ INPUT_NEUTRAL, new ControllerButton(0, 0) },
					{ INPUT_MATTER, new ControllerButton(0, 2) },
					{ INPUT_SPIRIT, new ControllerButton(0, 1) },
					{ INPUT_VOID, new ControllerButton(0, 3) },
					{ INPUT_ASCEND, new ControllerButton(0, 5) },
				})
			}
		);
	}

	void InGame::render() const
	{
		Battle::game.battleMgr->render();
	}

	IScene *InGame::update()
	{
		Battle::game.battleMgr->update();
		return nullptr;
	}

	void InGame::consumeEvent(const sf::Event &event)
	{
		game.battleMgr->consumeEvent(event);
	}
}