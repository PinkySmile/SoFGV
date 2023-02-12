//
// Created by PinkySmile on 27/09/2022.
//

#ifndef SOFGV_MENU_HPP
#define SOFGV_MENU_HPP


#include <vector>
#include "MenuItem.hpp"
#include "Inputs/InputEnum.hpp"

namespace SpiralOfFate
{
	class Menu {
	private:
		std::vector<MenuItem> _items;
		mutable Sprite _separatorBody;
		mutable Sprite _separatorTopTip;
		mutable Sprite _separatorBotTip;
		unsigned _displayTimer = 0;
		unsigned _selected = 0;

	public:
		bool displayed = false;

		Menu(const std::vector<MenuItemSkeleton> &&arr);
		~Menu();
		void update(InputStruct inputs);
		void render() const;
		unsigned getSelectedItem() const;
		void setSelectedItem(int item);
	};
}


#endif //SOFGV_MENU_HPP
