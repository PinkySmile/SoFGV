//
// Created by PinkySmile on 21/01/23
//

#include "VirtualController.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	VirtualController::VirtualController()
	{
		this->_stickBack.textureHandle = game->textureMgr.load("assets/icons/inputs/stick_back.png");
		this->_stickBack.setPosition(20, 940 - game->textureMgr.getTextureSize(this->_stickBack.textureHandle).y * 6);
		this->_stickBack.setScale(6, 6);
		this->_stickBack.setColor(sf::Color{255, 255, 255, 100});
		game->textureMgr.setTexture(this->_stickBack)->setSmooth(true);

		this->_stickTop.textureHandle = game->textureMgr.load("assets/icons/inputs/stick_top.png");
		this->_stickTop.setPosition(20, 940 - game->textureMgr.getTextureSize(this->_stickTop.textureHandle).y * 6);
		this->_stickTop.setScale(6, 6);
		this->_stickTop.setColor(sf::Color{255, 255, 255, 100});
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
		default:
			break;
		}
	}

	void VirtualController::render()
	{
		return;
		sf::Sprite sprite;
		auto view = game->screen->getView();
		auto size = view.getSize();
		auto top = view.getCenter();

		top.x -= size.x / 2;
		top.y -= size.y / 2;
		this->_canvas.clear(sf::Color::Transparent);
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
			this->_stickBack.setColor(sf::Color{255, 255, 255, 150});
			this->_stickTop.setColor(sf::Color{255, 255, 255, 150});
		}
	}

	void VirtualController::_onDrag(const Vector2f &location, int index)
	{

	}

	void VirtualController::_onRelease(const Vector2f &location, int index)
	{
		if (this->_indexes[INPUT_LEFT] == index) {
			this->_indexes[INPUT_LEFT] = -30;
			this->_stickBack.setColor(sf::Color{255, 255, 255, 100});
			this->_stickTop.setColor(sf::Color{255, 255, 255, 100});
		}
	}
}