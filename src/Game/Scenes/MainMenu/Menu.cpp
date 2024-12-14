//
// Created by PinkySmile on 27/09/2022.
//

#include "Menu.hpp"
#include "Resources/Game.hpp"

#define SEPARATOR_GROW_SPEED 15
#define SEPARATOR_END_SPACE 15

namespace SpiralOfFate
{
	Menu::Menu(const std::string &font, const std::vector<std::vector<MenuItemSkeleton>> &&arr)
	{
		this->_font.loadFromFile(font);

		this->_separatorBody.textureHandle = game->textureMgr.load("assets/ui/separator.png");
		this->_separatorBody.setTextureRect({47, 0, 47, 5});
		this->_separatorBody.setPosition({817, 312});

		this->_separatorTopTip.textureHandle = game->textureMgr.load("assets/ui/separator.png");
		this->_separatorTopTip.setTextureRect({0, 0, 47, 63});
		this->_separatorTopTip.setPosition({817, 249});

		this->_separatorBotTip.textureHandle = game->textureMgr.load("assets/ui/separator.png");
		this->_separatorBotTip.setPosition({840, 339});
		this->_separatorBotTip.setOrigin({23, 32});
		this->_separatorBotTip.setRotation(180);
		this->_separatorBotTip.setTextureRect({0, 0, 47, 63});

		this->_selected.resize(arr.size());
		this->_items.reserve(arr.size());
		for (auto &subArr : arr) {
			this->_items.emplace_back();

			unsigned i = 0;
			auto &back = this->_items.back();

			back.reserve(subArr.size());
			for (auto &elem : subArr)
				back.emplace_back(new MenuItem(this->_font, i++, elem));
		}
		this->_items.front().front()->selected = true;

	}

	Menu::~Menu()
	{
		game->textureMgr.remove(this->_separatorBody.textureHandle);
		game->textureMgr.remove(this->_separatorTopTip.textureHandle);
		game->textureMgr.remove(this->_separatorBotTip.textureHandle);
	}

	void Menu::update(InputStruct inputs)
	{
		auto &oldItems = this->_items[this->_currentIndex];
		auto &newItems = this->_items[this->_nextIndex];
		size_t expectedSeparatorSize = newItems.size() * MENU_ITEM_SPACING + SEPARATOR_END_SPACE;
		bool needRedraw = this->_separatorSize != expectedSeparatorSize || this->_separatorDisableSize != expectedSeparatorSize;

		if (std::abs(static_cast<int>(this->_separatorSize - expectedSeparatorSize)) < SEPARATOR_GROW_SPEED)
			this->_separatorSize = expectedSeparatorSize;
		else if (this->_separatorSize < expectedSeparatorSize)
			this->_separatorSize += SEPARATOR_GROW_SPEED;
		else if (this->_separatorSize > expectedSeparatorSize) {
			if (this->_separatorShrinkTimer == 0)
				this->_separatorSize -= SEPARATOR_GROW_SPEED;
			else
				this->_separatorShrinkTimer--;
		}
		if (std::abs(static_cast<int>(this->_separatorDisableSize - expectedSeparatorSize)) < SEPARATOR_GROW_SPEED)
			this->_separatorDisableSize = expectedSeparatorSize;
		else if (this->_separatorDisableSize < expectedSeparatorSize)
			this->_separatorDisableSize += SEPARATOR_GROW_SPEED;
		else if (this->_separatorDisableSize > expectedSeparatorSize)
			this->_separatorDisableSize -= SEPARATOR_GROW_SPEED;

		this->_separatorShrinkTimer = std::min<unsigned>(this->_separatorShrinkTimer + 1, BUTTON_ANIM_LENGTH / BUTTON_HIDE_SPEED_FACTOR);
		if (needRedraw) {
			this->_separatorBody.setTextureRect({47, 0, 47, static_cast<int>(this->_separatorSize)});
			this->_separatorBotTip.setPosition({840, 339.f + this->_separatorSize});
			for (size_t i = 0; i < oldItems.size(); i++)
				oldItems[i]->displayed = i < this->_separatorDisableSize / MENU_ITEM_SPACING && i >= newItems.size();
			for (size_t i = 0; i < newItems.size(); i++) {
				bool old = newItems[i]->displayed;

				newItems[i]->displayed = i < this->_separatorDisableSize / MENU_ITEM_SPACING;
				if (!old && newItems[i]->displayed)
					this->_separatorShrinkTimer = 0;
			}
		}

		if (this->_currentIndex != this->_nextIndex) {
			bool done = true;

			for (auto &elem: oldItems) {
				if (elem->isAnimationFinished())
					continue;
				elem->update(false);
				done = false;
			}
			if (done)
				this->_currentIndex = this->_nextIndex;
		}
		for (auto &elem : newItems)
			elem->update(inputs.n == 1);
	}

	void Menu::render() const
	{
		game->textureMgr.render(this->_separatorBody);
		game->textureMgr.render(this->_separatorTopTip);
		game->textureMgr.render(this->_separatorBotTip);
		for (auto &elem : this->_items[this->_nextIndex])
			elem->render();
		if (this->_currentIndex != this->_nextIndex)
			for (auto &elem : this->_items[this->_currentIndex])
				elem->render();
	}

	unsigned Menu::getSelectedItem() const
	{
		return this->_selected[this->_nextIndex];
	}

	void Menu::setSelectedItem(int item)
	{
		while (item < 0)
			item += this->_items[this->_nextIndex].size();

		auto &items = this->_items[this->_nextIndex];
		auto &selected = this->_selected[this->_nextIndex];

		items[selected]->selected = false;
		selected = item % items.size();
		items[selected]->selected = true;
	}

	unsigned Menu::getMenuSize() const
	{
		return this->_items[this->_nextIndex].size();
	}

	void Menu::setEnabledMenu(unsigned index, bool resetOldSelected)
	{
		this->_currentIndex = this->_nextIndex;
		this->_nextIndex = index;

		auto &oldItems = this->_items[this->_currentIndex];
		auto &newItems = this->_items[this->_nextIndex];

		for (size_t i = 0; i < oldItems.size(); i++) {
			oldItems[i]->cursorDisplayed = i >= newItems.size();
			oldItems[i]->displayed = false;
			oldItems[i]->selected = false;
			oldItems[i]->animationSpeed = BUTTON_HIDE_SPEED_FACTOR;
		}
		for (size_t i = 0; i < newItems.size(); i++) {
			newItems[i]->selected = i == this->_selected[this->_nextIndex];
			newItems[i]->cursorDisplayed = true;
			newItems[i]->displayed = true;
			newItems[i]->resetAnimation(i < oldItems.size(), this->_selected[this->_currentIndex] == i);
			newItems[i]->animationSpeed = BUTTON_HIDE_SPEED_FACTOR;
		}
		if (resetOldSelected)
			this->_selected[this->_currentIndex] = 0;
		if (this->_currentIndex != this->_nextIndex)
			for (auto &elem: oldItems)
				elem->update(false);
		for (auto &elem : newItems)
			elem->update(false);
	}

	unsigned Menu::getEnabledMenu() const
	{
		return this->_nextIndex;
	}
}