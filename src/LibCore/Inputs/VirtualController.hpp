//
// Created by andgel on 21/01/23
//

#ifndef SOFGV_VIRTUALCONTROLLER_HPP
#define SOFGV_VIRTUALCONTROLLER_HPP


#include "RollbackInput.hpp"
#include "Resources/Assets/Sprite.hpp"

namespace SpiralOfFate
{
	class VirtualController : public RollbackInput {
	private:
		Sprite _stickBack;
		Sprite _stickTop;

	public:
		enum SceneState {
			SCENESTATE_TITLE_SCREEN,
			SCENESTATE_IN_GAME,
			SCENESTATE_PRACTICE
		};

		SceneState currentState = SCENESTATE_TITLE_SCREEN;

		VirtualController();
		~VirtualController();
		void consumeEvent(const sf::Event &event) override;
		void render();
	};
} // SpiralOfFate


#endif //SOFGV_VIRTUALCONTROLLER_HPP
