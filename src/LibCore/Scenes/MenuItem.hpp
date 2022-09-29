//
// Created by PinkySmile on 27/09/2022.
//

#ifndef SOFGV_MENUITEM_HPP
#define SOFGV_MENUITEM_HPP


#include "Resources/Sprite.hpp"
#include <functional>

namespace SpiralOfFate
{
	struct MenuItemSkeleton {
		const char *img;
		const char *desc;
		const std::function<void ()> onClick;
	};

	class MenuItem {
	private:
		mutable Sprite _btnImg;
		mutable Sprite _cursImg;
		mutable Sprite _textImg;
		mutable Sprite _textImgBlur;
		unsigned _index;
		int _enableTimer = 0;
		int _displayTimer = 0;
		std::function<void ()> _onClick;

	public:
		bool enabled = false;
		bool displayed = false;
		const char *description;

		MenuItem(unsigned index, const MenuItemSkeleton &skeleton);
		~MenuItem();
		void update(bool pressed);
		void render() const;
	};
}


#endif //SOFGV_MENUITEM_HPP
