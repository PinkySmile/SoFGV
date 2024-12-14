//
// Created by PinkySmile on 27/09/2022.
//

#ifndef SOFGV_MENU_HPP
#define SOFGV_MENU_HPP


#include <vector>
#include <memory>
#include "MenuItem.hpp"
#include "Inputs/InputEnum.hpp"

namespace SpiralOfFate
{
	class Menu {
	private:
		sf::Font _font;
		std::vector<std::vector<std::unique_ptr<MenuItem>>> _items;
		mutable Sprite _separatorBody;
		mutable Sprite _separatorTopTip;
		mutable Sprite _separatorBotTip;
		std::vector<unsigned> _selected;
		unsigned _currentIndex = 0;
		unsigned _nextIndex = 0;
		unsigned _separatorSize = 0;
		unsigned _separatorDisableSize = 0;
		unsigned _separatorShrinkTimer = 0;

	public:
		Menu(const std::string &font, const std::vector<std::vector<MenuItemSkeleton>> &&arr);
		~Menu();
		void update(InputStruct inputs);
		void render() const;
		unsigned getSelectedItem() const;
		void setSelectedItem(int item);
		unsigned getMenuSize() const;
		void setEnabledMenu(unsigned index, bool resetOldSelected = true);
		unsigned getEnabledMenu() const;
	};
}


#endif //SOFGV_MENU_HPP
