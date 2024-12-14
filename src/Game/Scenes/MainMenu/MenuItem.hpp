//
// Created by PinkySmile on 27/09/2022.
//

#ifndef SOFGV_MENUITEM_HPP
#define SOFGV_MENUITEM_HPP


#include "Sprite.hpp"
#include <functional>

#define MENU_ITEM_SPACING 65
#define BUTTON_DISPLAY_ANIM_LENGTH 15
#define BUTTON_LENGTHEN_ANIM_LENGTH 30
#define BUTTON_RENDER_ANIM_LENGTH 30
#define BUTTON_ANIM_LENGTH (BUTTON_DISPLAY_ANIM_LENGTH + BUTTON_LENGTHEN_ANIM_LENGTH + BUTTON_RENDER_ANIM_LENGTH)
#define BUTTON_HIDE_SPEED_FACTOR 3

namespace SpiralOfFate
{
	struct MenuItemSkeleton {
		const char *button;
		const char *desc;
		const std::function<void ()> onClick;
	};

	class MenuItem {
	private:
		bool _forceRefresh = false;
		sf::RenderTexture _normalText;
		sf::RenderTexture _blurredText;
		mutable Sprite _btnImg;
		mutable Sprite _cursImg;
		mutable Sprite _textImg;
		mutable Sprite _textImgBlur;
		unsigned _index;
		int _enableTimer = 0;
		int _displayTimer = 0;
		std::function<void ()> _onClick;

	public:
		bool selected = false;
		bool disabled = false;
		bool displayed = false;
		bool cursorDisplayed = true;
		float animationSpeed = 1;
		const char *description;

		MenuItem(const sf::Font &font, unsigned index, const MenuItemSkeleton &skeleton);
		~MenuItem();
		void update(bool pressed);
		void render() const;
		void resetAnimation(bool skipButtonGrowAnimation, bool wasEnabled);
		bool isAnimationFinished() const;
	};
}


#endif //SOFGV_MENUITEM_HPP
