//
// Created by PinkySmile on 27/09/2022.
//

#ifndef SOFGV_MENU_HPP
#define SOFGV_MENU_HPP


#include <vector>
#include <memory>
#include "MenuItem.hpp"
#include "Inputs/InputEnum.hpp"
#include "Screen.hpp"
#include "Data/Vector.hpp"

namespace SpiralOfFate
{
	class Menu {
	private:
		sf::Text _descText;
		sf::Font _buttonFont;
		sf::Font _descFont;
		std::vector<std::vector<std::unique_ptr<MenuItem>>> _items;
		std::unique_ptr<PreparedShrunkRect> _descBox;
		mutable Sprite _separatorBody;
		mutable Sprite _separatorTopTip;
		mutable Sprite _separatorBotTip;
		std::vector<unsigned> _selected;
		unsigned _currentIndex = 0;
		unsigned _nextIndex = 0;
		unsigned _separatorSize = 0;
		unsigned _separatorDisableSize = 0;
		unsigned _separatorShrinkTimer = 0;
		Vector2u _descBoxSize = {0, 0};
		std::string _desc;
		Vector2u _startDescSize = {0, 0};
		Vector2u _descSize = {0, 0};
		unsigned _descTimer;

		void _computeExpectedDescBoxSize();

	public:
		Menu(const std::string &buttonFont, const std::string &descFont, const std::vector<std::vector<MenuItemSkeleton>> &&arr);
		~Menu();
		void update(InputStruct inputs);
		void render() const;
		unsigned getSelectedItem() const;
		void setSelectedItem(int item);
		unsigned getMenuSize() const;
		void setEnabledMenu(unsigned index, bool resetOldSelected = true);
		unsigned getEnabledMenu() const;
		const MenuItem &getCurrentItem() const;
	};
}


#endif //SOFGV_MENU_HPP
