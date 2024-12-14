//
// Created by PinkySmile on 27/09/2022.
//

#include "Menu.hpp"
#include "Resources/Game.hpp"

#define SEPARATOR_GROW_SPEED 15
#define SEPARATOR_END_SPACE 15
#define MAX_DESC_WIDTH 650
#define DESC_FONT_SIZE 25
#define DESC_POS Vector2i{150, 320}
#define DESC_FADE_TIME 5

namespace SpiralOfFate
{
	Menu::Menu(const std::string &buttonFont, const std::string &descFont, const std::vector<std::vector<MenuItemSkeleton>> &&arr)
	{
		Sprite descBox;

		assert_exp(this->_buttonFont.loadFromFile(buttonFont));
		assert_exp(this->_descFont.loadFromFile(descFont));

		descBox.textureHandle = game->textureMgr.load("assets/ui/box.png");
		game->textureMgr.setTexture(descBox);
		this->_descBox = game->screen->prepareShrunkRect(descBox);
		game->textureMgr.remove(descBox.textureHandle);

		this->_descTimer = DESC_FADE_TIME;

		this->_descText.setFont(this->_descFont);
		this->_descText.setCharacterSize(DESC_FONT_SIZE);
		this->_descText.setOutlineThickness(0);
		this->_descText.setFillColor(sf::Color::White);
		this->_descText.setOutlineColor(sf::Color::Transparent);

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
				back.emplace_back(new MenuItem(this->_buttonFont, i++, elem));
		}
		this->_items.front().front()->selected = true;
		this->_computeExpectedDescBoxSize();
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

		if (this->_descTimer < DESC_FADE_TIME) {
			this->_descTimer++;
			this->_descText.setFillColor(Color{255, 255, 255, static_cast<unsigned char>(255 - 255 * this->_descTimer / DESC_FADE_TIME)});
			if (this->_descTimer == DESC_FADE_TIME)
				this->_computeExpectedDescBoxSize();
		} else if (this->_descTimer < DESC_FADE_TIME * 2) {
			this->_descTimer++;
			this->_descBoxSize = this->_startDescSize + (this->_descSize.to<float>() - this->_startDescSize) * (1 - (DESC_FADE_TIME * 2.f - this->_descTimer) / DESC_FADE_TIME);
			this->_descText.setFillColor(Color{255, 255, 255, static_cast<unsigned char>(255 - 255 * (DESC_FADE_TIME * 2 - this->_descTimer) / DESC_FADE_TIME)});
		}

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
		game->screen->displayShrunkRect(*this->_descBox, {
			static_cast<int>(DESC_POS.x + MAX_DESC_WIDTH - this->_descBoxSize.x), DESC_POS.y,
			static_cast<int>(this->_descBoxSize.x + this->_descBox->texSize.x + 2), static_cast<int>(this->_descBoxSize.y + this->_descBox->texSize.y)});
		game->screen->draw(this->_descText);
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
		if (this->_descTimer >= DESC_FADE_TIME)
			this->_descTimer = DESC_FADE_TIME * 2 - this->_descTimer;
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
		if (this->_descTimer >= DESC_FADE_TIME)
			this->_descTimer = DESC_FADE_TIME * 2 - this->_descTimer;
	}

	unsigned Menu::getMenuSize() const
	{
		return this->_items[this->_nextIndex].size();
	}

	unsigned Menu::getEnabledMenu() const
	{
		return this->_nextIndex;
	}

	void Menu::_computeExpectedDescBoxSize()
	{
		auto &text = this->getCurrentItem().description;
		float x = 0;
		float y = 0;
		float wordSize = 0;
		char s = 0;
		std::string word;

		this->_startDescSize = this->_descBoxSize;
		this->_desc.clear();
		this->_descSize.x = 0;
		for (auto c : text) {
			if (std::isspace(c)) {
				if (x + wordSize > MAX_DESC_WIDTH) {
					this->_desc.push_back('\n');
					this->_descSize.x = std::max<unsigned>(this->_descSize.x, x);
					x = 0;
					s = 0;
					y += this->_descFont.getLineSpacing(DESC_FONT_SIZE);
				}
				x += wordSize;
				wordSize = 0;
				if (s != 0) {
					x += this->_descFont.getGlyph(s, DESC_FONT_SIZE, false).advance;
					if (!word.empty())
						x += this->_descFont.getKerning(word.back(), s, DESC_FONT_SIZE);
					this->_desc.push_back(s);
				}
				this->_desc.append(word);
				word.clear();
				if (c == '\n') {
					this->_descSize.x = std::max<unsigned>(this->_descSize.x, x);
					x = 0;
					y += this->_descFont.getLineSpacing(DESC_FONT_SIZE);
					s = 0;
					this->_desc.push_back('\n');
				} else
					s = c;
			} else {
				wordSize += this->_descFont.getGlyph(c, DESC_FONT_SIZE, false).advance;
				if (!word.empty())
					wordSize += this->_descFont.getKerning(word.back(), c, DESC_FONT_SIZE);
				word.push_back(c);
			}
		}


		if (x + wordSize > MAX_DESC_WIDTH) {
			this->_desc.push_back('\n');
			this->_descSize.x = std::max<unsigned>(this->_descSize.x, x);
			x = 0;
			y += this->_descFont.getLineSpacing(DESC_FONT_SIZE);
		}
		x += wordSize;
		if (s != 0) {
			x += this->_descFont.getGlyph(s, DESC_FONT_SIZE, false).advance;
			if (!word.empty())
				x += this->_descFont.getKerning(word.back(), s, DESC_FONT_SIZE);
			this->_desc.push_back(s);
		}
		this->_desc.append(word);
		word.clear();

		this->_descSize.x = std::max<unsigned>(this->_descSize.x, x);
		this->_descSize.y = y + this->_descFont.getLineSpacing(DESC_FONT_SIZE);
		this->_descText.setString(this->_desc);
		this->_descText.setPosition(DESC_POS + this->_descBox->texSize / 2 + Vector2i{2 + MAX_DESC_WIDTH - static_cast<int>(this->_descSize.x), -1});
	}

	const MenuItem &Menu::getCurrentItem() const
	{
		return *this->_items[this->_nextIndex][this->_selected[this->_nextIndex]];
	}
}