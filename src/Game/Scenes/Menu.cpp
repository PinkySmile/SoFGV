//
// Created by PinkySmile on 27/09/2022.
//

#include "Menu.hpp"
#include "Resources/Game.hpp"

#define DISPLAY_ANIM_LENGTH 60
#define HIDE_SPEED_FACTOR 3

namespace SpiralOfFate
{
	Menu::Menu(const std::vector<MenuItemSkeleton> &&arr)
	{
		unsigned i = 0;

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

		this->_items.reserve(arr.size());
		for (auto &elem : arr)
			this->_items.emplace_back(i++, elem);
		this->_items[0].enabled = true;
	}

	Menu::~Menu()
	{
		game->textureMgr.remove(this->_separatorBody.textureHandle);
		game->textureMgr.remove(this->_separatorTopTip.textureHandle);
		game->textureMgr.remove(this->_separatorBotTip.textureHandle);
	}

	void Menu::update(InputStruct inputs)
	{
		if (this->_displayTimer != this->displayed * DISPLAY_ANIM_LENGTH) {
			memset(&inputs, 0, sizeof(inputs));
			this->_displayTimer += this->displayed && this->_displayTimer < DISPLAY_ANIM_LENGTH;
			this->_displayTimer -= (!this->displayed && this->_displayTimer > 0) * HIDE_SPEED_FACTOR;

			int size = 15 + this->_items.size() * 65 * this->_displayTimer / DISPLAY_ANIM_LENGTH;

			this->_separatorBody.setTextureRect({47, 0, 47, size});
			this->_separatorBotTip.setPosition({840, 339.f + size});
			for (size_t i = 0; i < this->_items.size() * this->_displayTimer / DISPLAY_ANIM_LENGTH; i++)
				this->_items[i].displayed = true;
		}
		for (auto &elem : this->_items)
			elem.update(inputs.n == 1);
	}

	void Menu::render() const
	{
		if (this->_displayTimer == 0 && !this->displayed)
			return;
		game->textureMgr.render(this->_separatorBody);
		game->textureMgr.render(this->_separatorTopTip);
		game->textureMgr.render(this->_separatorBotTip);
		for (auto &elem : this->_items)
			elem.render();
	}

	unsigned Menu::getSelectedItem() const
	{
		return this->_selected;
	}

	void Menu::setSelectedItem(int item)
	{
		while (item < 0)
			item += this->_items.size();
		this->_items[this->_selected].enabled = false;
		this->_selected = item % this->_items.size();
		this->_items[this->_selected].enabled = true;
	}
}