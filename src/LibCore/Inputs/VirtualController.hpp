//
// Created by PinkySmile on 21/01/23
//

#ifndef SOFGV_VIRTUALCONTROLLER_HPP
#define SOFGV_VIRTUALCONTROLLER_HPP


#include "RollbackInput.hpp"
#include "Resources/Assets/Sprite.hpp"
#include "Data/Vector.hpp"

namespace SpiralOfFate
{
	class VirtualController : public RollbackInput {
	private:
		Sprite _stickTop;
		Sprite _stickBack;
		sf::RenderTexture _canvas;
		std::array<int, INPUT_NUMBER - 3> _indexes;

		void _onMoveStick(const Vector2f &location);
		void _onPress(const Vector2f &location, int index);
		void _onDrag(const Vector2f &location, int index);
		void _onRelease(const Vector2f &location, int index);

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
