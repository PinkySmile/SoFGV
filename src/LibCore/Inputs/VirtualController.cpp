//
// Created by PinkySmile on 21/01/23
//

#include "VirtualController.hpp"
#include "Resources/Game.hpp"

#ifdef __ANDROID__
#define MIN_ALPHA 100
#else
#define MIN_ALPHA 0
#endif
#define MAX_ALPHA 150
#define STICK_SCALE 6
#define BUTTON_SCALE 3

namespace SpiralOfFate
{
	const char *buttonAssets[] = {
		"assets/icons/inputs/neutral.png",
		"assets/icons/inputs/spirit.png",
		"assets/icons/inputs/matter.png",
		"assets/icons/inputs/void.png",
		"assets/icons/inputs/ascend.png",
		"assets/icons/inputs/dash.png",
		"assets/icons/inputs/pause.png",
	};
	const Vector2i buttonPos[] = {
		{static_cast<int>(1660 - 64 * BUTTON_SCALE * 1.75), static_cast<int>(940 - 64 * BUTTON_SCALE * 1.00)},
		{static_cast<int>(1660 - 64 * BUTTON_SCALE * 1.00), static_cast<int>(940 - 64 * BUTTON_SCALE * 1.75)},
		{static_cast<int>(1660 - 64 * BUTTON_SCALE * 2.50), static_cast<int>(940 - 64 * BUTTON_SCALE * 1.75)},
		{static_cast<int>(1660 - 64 * BUTTON_SCALE * 1.75), static_cast<int>(940 - 64 * BUTTON_SCALE * 2.50)},
		{20, 20},
		{1660 - 64 * BUTTON_SCALE, 20},
		{(1660 - 64 * BUTTON_SCALE) / 2, 20},
	};

	VirtualController::VirtualController()
	{
		my_assert(this->_stickBack.textureHandle = game->textureMgr.load("assets/icons/inputs/stick_back.png"));
		this->_stickBack.setPosition(20, 940 - game->textureMgr.getTextureSize(this->_stickBack.textureHandle).y * STICK_SCALE);
		this->_stickBack.setScale(STICK_SCALE, STICK_SCALE);
		this->_stickBack.setColor(sf::Color{255, 255, 255, MIN_ALPHA});
		game->textureMgr.setTexture(this->_stickBack)->setSmooth(true);

		my_assert(this->_stickTop.textureHandle = game->textureMgr.load("assets/icons/inputs/stick_top.png"));
		this->_stickTop.setPosition(20, 940 - game->textureMgr.getTextureSize(this->_stickTop.textureHandle).y * STICK_SCALE);
		this->_stickTop.setScale(STICK_SCALE, STICK_SCALE);
		this->_stickTop.setColor(sf::Color{255, 255, 255, MIN_ALPHA});
		game->textureMgr.setTexture(this->_stickTop)->setSmooth(true);

		for (int i = 0; i < 7; i++) {
			my_assert(this->_buttons[i].textureHandle = game->textureMgr.load(buttonAssets[i]));
			this->_buttons[i].setPosition(buttonPos[i]);
			this->_buttons[i].setScale(BUTTON_SCALE, BUTTON_SCALE);
			this->_buttons[i].setColor(sf::Color{255, 255, 255, MIN_ALPHA});
			game->textureMgr.setTexture(this->_buttons[i])->setSmooth(true);
		}

		this->_canvas.create(1680, 960);
		this->_indexes.fill(-30);
	}

	VirtualController::~VirtualController()
	{
		game->textureMgr.remove(this->_stickBack.textureHandle);
		game->textureMgr.remove(this->_stickTop.textureHandle);
	}

	static Vector2f getScreenCoords(sf::Vector2i coords)
	{
		auto size = game->screen->getSize();

		return {
			coords.x * 1680.f / size.x,
			coords.y * 960.f / size.y,
		};
	}

	void VirtualController::consumeEvent(const sf::Event &event)
	{
		static bool _mousePressed = false;

		switch (event.type) {
		case sf::Event::MouseButtonPressed:
			(this->*(_mousePressed ? &VirtualController::_onDrag : &VirtualController::_onPress))(getScreenCoords({event.mouseButton.x, event.mouseButton.y}), -1);
			_mousePressed = true;
			break;
		case sf::Event::MouseButtonReleased:
			this->_onRelease(getScreenCoords({event.mouseButton.x, event.mouseButton.y}), -1);
			_mousePressed = false;
			break;
		case sf::Event::MouseMoved:
			if (_mousePressed)
				this->_onDrag(getScreenCoords({event.mouseMove.x, event.mouseMove.y}), -1);
			break;
		case sf::Event::TouchBegan:
			this->_onPress(getScreenCoords({event.touch.x, event.touch.y}), event.touch.finger);
			break;
		case sf::Event::TouchEnded:
			this->_onRelease(getScreenCoords({event.touch.x, event.touch.y}), event.touch.finger);
			break;
		case sf::Event::TouchMoved:
			this->_onDrag(getScreenCoords({event.touch.x, event.touch.y}), event.touch.finger);
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
		for (auto &button : this->_buttons)
			this->_canvas.draw(button);
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
			20 + this->_stickBack.getTexture()->getSize().x * STICK_SCALE / 2,
			940 - this->_stickBack.getTexture()->getSize().y * STICK_SCALE / 2
		};
		auto sqr = this->_stickBack.getTexture()->getSize().x * STICK_SCALE / 2;
		auto distance2 = center.distance2(location);

		sqr *= sqr;
		if (distance2 < sqr) {
			this->_indexes[0] = index;
			this->_stickBack.setColor(sf::Color{255, 255, 255, MAX_ALPHA});
			this->_stickTop.setColor(sf::Color{255, 255, 255, MAX_ALPHA});
			this->_onMoveStick(location);
		}
		for (int i = 0; i < 7; i++) {
			auto &button = this->_buttons[i];
			auto sqr2 = button.getTexture()->getSize().x * BUTTON_SCALE / 2;
			auto center2 = buttonPos[i] + Vector2u{sqr2, sqr2};

			sqr2 *= sqr2;
			if (center2.distance2(location) < sqr2) {
				this->_indexes[1 + i] = index;
				this->_keyStates[INPUT_N + i] = true;
				button.setColor(sf::Color{255, 255, 255, MAX_ALPHA});
			}
		}
	}

	void VirtualController::_onDrag(const Vector2f &location, int index)
	{
		if (this->_indexes[0] == index)
			this->_onMoveStick(location);
		for (int i = 0; i < 7; i++) {
			if (this->_indexes[1 + i] != index)
				continue;

			auto &button = this->_buttons[i];
			auto sqr2 = button.getTexture()->getSize().x * BUTTON_SCALE / 2;
			auto center2 = buttonPos[i] + Vector2u{sqr2, sqr2};

			sqr2 *= sqr2;
			if (center2.distance2(location) < sqr2) {
				button.setColor(sf::Color{255, 255, 255, MAX_ALPHA});
				this->_keyStates[INPUT_N + i] = true;
			} else {
				button.setColor(sf::Color{255, 255, 255, MIN_ALPHA});
				this->_keyStates[INPUT_N + i] = false;
			}
		}
	}

	void VirtualController::_onRelease(const Vector2f &location, int index)
	{
		if (this->_indexes[0] == index) {
			this->_indexes[0] = -30;
			this->_stickBack.setColor(sf::Color{255, 255, 255, MIN_ALPHA});
			this->_stickTop.setColor(sf::Color{255, 255, 255, MIN_ALPHA});
			this->_stickTop.setPosition(20, 940 - this->_stickTop.getTexture()->getSize().y * 6);
			this->_keyStates[INPUT_LEFT] = false;
			this->_keyStates[INPUT_RIGHT] = false;
			this->_keyStates[INPUT_UP] = false;
			this->_keyStates[INPUT_DOWN] = false;
		}
		for (int i = 0; i < 7; i++) {
			if (this->_indexes[1 + i] != index)
				continue;

			auto &button = this->_buttons[i];

			this->_indexes[1 + i] = -30;
			this->_keyStates[INPUT_N + i] = false;
			button.setColor(sf::Color{255, 255, 255, MIN_ALPHA});
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
		this->_keyStates[INPUT_LEFT] = result.x < base.x - 30;
		this->_keyStates[INPUT_RIGHT] = result.x > base.x + 30;
		this->_keyStates[INPUT_UP] = result.y < base.y - 30;
		this->_keyStates[INPUT_DOWN] = result.y > base.y + 30;
	}

	std::string VirtualController::getName() const
	{
		return "Virtual Controller";
	}

	void VirtualController::update()
	{
		if (this->_frameStart)
			RollbackInput::update();
		this->_frameStart = false;
	}

	void VirtualController::onFrameStart()
	{
		this->_frameStart = true;
	}
}