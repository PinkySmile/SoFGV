//
// Created by PinkySmile on 21/01/23
//

#include "VirtualController.hpp"
#include "Resources/Game.hpp"

#define MIN_ALPHA 0
#define MAX_ALPHA 150

namespace SpiralOfFate
{
	VirtualController::VirtualController()
	{
		this->_stickBack.textureHandle = game->textureMgr.load("assets/icons/inputs/stick_back.png");
		this->_stickBack.setPosition(20, 940 - game->textureMgr.getTextureSize(this->_stickBack.textureHandle).y * 6);
		this->_stickBack.setScale(6, 6);
		this->_stickBack.setColor(sf::Color{255, 255, 255, MIN_ALPHA});
		game->textureMgr.setTexture(this->_stickBack)->setSmooth(true);

		this->_stickTop.textureHandle = game->textureMgr.load("assets/icons/inputs/stick_top.png");
		this->_stickTop.setPosition(20, 940 - game->textureMgr.getTextureSize(this->_stickTop.textureHandle).y * 6);
		this->_stickTop.setScale(6, 6);
		this->_stickTop.setColor(sf::Color{255, 255, 255, MIN_ALPHA});
		game->textureMgr.setTexture(this->_stickTop)->setSmooth(true);

		this->_canvas.create(1680, 960);
	}

	VirtualController::~VirtualController()
	{
		game->textureMgr.remove(this->_stickBack.textureHandle);
		game->textureMgr.remove(this->_stickTop.textureHandle);
	}

	void VirtualController::consumeEvent(const sf::Event &event)
	{
		static bool _mousePressed = false;

		switch (event.type) {
		case sf::Event::MouseButtonPressed:
			(this->*(_mousePressed ? &VirtualController::_onDrag : &VirtualController::_onPress))(game->screen->mapPixelToCoords({event.mouseButton.x, event.mouseButton.y}), -1);
			_mousePressed = true;
			break;
		case sf::Event::MouseButtonReleased:
			this->_onRelease(game->screen->mapPixelToCoords({event.mouseButton.x, event.mouseButton.y}), -1);
			_mousePressed = false;
			break;
		case sf::Event::MouseMoved:
			if (_mousePressed)
				this->_onDrag(game->screen->mapPixelToCoords({event.mouseMove.x, event.mouseMove.y}), -1);
			break;
		default:
			break;
		}
	}

	void VirtualController::render()
	{
		sf::Sprite sprite;
		auto view = game->screen->getView();
		auto size = view.getSize();
		auto top = view.getCenter();

		top.x -= size.x / 2;
		top.y -= size.y / 2;
		this->_canvas.clear(sf::Color{0xFF, 0xFF, 0xFF, 0x00});
		this->_canvas.draw(this->_stickBack, sf::BlendNone);
		this->_canvas.draw(this->_stickTop);
		this->_canvas.display();
		sprite.setTexture(this->_canvas.getTexture());
		sprite.setPosition(top);
		sprite.setScale(size.x / 1680, size.y / 960);
		game->screen->draw(sprite);
	}

	void VirtualController::_onPress(const Vector2f &location, int index)
	{
		Vector2u center{
			20 + this->_stickBack.getTexture()->getSize().x * 3,
			940 - this->_stickBack.getTexture()->getSize().y * 3
		};
		auto sqr = this->_stickBack.getTexture()->getSize().x * 3;

		sqr *= sqr;
		if (center.distance2(location) < sqr) {
			this->_indexes[INPUT_LEFT] = index;
			this->_stickBack.setColor(sf::Color{255, 255, 255, MAX_ALPHA});
			this->_stickTop.setColor(sf::Color{255, 255, 255, MAX_ALPHA});
			this->_onMoveStick(location);
		}
	}

	void VirtualController::_onDrag(const Vector2f &location, int index)
	{
		if (this->_indexes[INPUT_LEFT] == index)
			this->_onMoveStick(location);
	}

	void VirtualController::_onRelease(const Vector2f &location, int index)
	{
		if (this->_indexes[INPUT_LEFT] == index) {
			this->_indexes[INPUT_LEFT] = -30;
			this->_stickBack.setColor(sf::Color{255, 255, 255, MIN_ALPHA});
			this->_stickTop.setColor(sf::Color{255, 255, 255, MIN_ALPHA});
			this->_stickTop.setPosition(20, 940 - this->_stickTop.getTexture()->getSize().y * 6);
			this->_keyStates[INPUT_LEFT] = false;
			this->_keyStates[INPUT_RIGHT] = false;
			this->_keyStates[INPUT_UP] = false;
			this->_keyStates[INPUT_DOWN] = false;
		}
	}

	void VirtualController::_onMoveStick(const Vector2f &location)
	{
		auto backSize = Vector2(this->_stickBack.getTexture()->getSize());
		auto size = Vector2(this->_stickTop.getTexture()->getSize());
		auto result = location - size * 3;
		auto base = Vector2f{20, 940.f - size.y * 6};
		auto dist = result.distance2(base);
		auto maxDist = backSize.x * 2;

		if (dist >= maxDist * maxDist)
			result = base + (result - base).normalized() * maxDist;
		this->_stickTop.setPosition(result);
		if (dist < maxDist / 2.) {
			this->_keyStates[INPUT_LEFT] = false;
			this->_keyStates[INPUT_RIGHT] = false;
			this->_keyStates[INPUT_UP] = false;
			this->_keyStates[INPUT_DOWN] = false;
			return;
		}
		this->_keyStates[INPUT_LEFT] = result.x < base.x;
		this->_keyStates[INPUT_RIGHT] = result.x > base.x;
		this->_keyStates[INPUT_UP] = result.y < base.y;
		this->_keyStates[INPUT_DOWN] = result.y > base.y;
	}
}