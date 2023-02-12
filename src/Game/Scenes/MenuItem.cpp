//
// Created by PinkySmile on 27/09/2022.
//

#include "MenuItem.hpp"
#include "Resources/Game.hpp"

#define BUTTON_DISPLAY_ANIM_LENGTH 15
#define BUTTON_LENGTHEN_ANIM_LENGTH 30
#define BUTTON_RENDER_ANIM_LENGTH 30
#define BUTTON_ANIM_LENGTH (BUTTON_DISPLAY_ANIM_LENGTH + BUTTON_LENGTHEN_ANIM_LENGTH + BUTTON_RENDER_ANIM_LENGTH)
#define BUTTON_HIDE_SPEED_FACTOR 3
#define CURSOR_DISP_ANIM_LENGTH 15
#define SELECTED_BUTTON_LENGTH_EXTEND 60

namespace SpiralOfFate
{
	MenuItem::MenuItem(unsigned index, const MenuItemSkeleton &skeleton) :
		_index(index),
		_onClick(skeleton.onClick),
		description(skeleton.desc)
	{
		this->_btnImg.textureHandle = game->textureMgr.load("assets/ui/buttonbar.png");
		this->_btnImg.setPosition({842, 321 + 65.f * index});
		this->_btnImg.setColor(sf::Color::Transparent);

		this->_cursImg.textureHandle = game->textureMgr.load("assets/ui/cursor.png");
		this->_cursImg.setPosition({842, 339 + 65.f * index});
		this->_cursImg.setColor(sf::Color::Transparent);

		this->_textImg.textureHandle = game->textureMgr.load("assets/ui/text/" + std::string(skeleton.img) + ".png");
		this->_textImg.setPosition({868, 331 + 65.f * index});
		this->_textImg.setColor(sf::Color::Transparent);

		this->_textImgBlur.textureHandle = game->textureMgr.load("assets/ui/text/" + std::string(skeleton.img) + "_blur.png");
		this->_textImgBlur.setPosition(this->_textImg.getPosition());
		this->_textImgBlur.setColor(sf::Color::Transparent);
	}

	MenuItem::~MenuItem()
	{
		game->textureMgr.remove(this->_btnImg.textureHandle);
		game->textureMgr.remove(this->_cursImg.textureHandle);
		game->textureMgr.remove(this->_textImg.textureHandle);
		game->textureMgr.remove(this->_textImgBlur.textureHandle);
	}

	void MenuItem::update(bool pressed)
	{
		if (pressed && this->enabled) {
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			this->_onClick();
		}
		if (this->displayed * BUTTON_ANIM_LENGTH != this->_displayTimer) {
			this->_displayTimer += this->displayed && this->_displayTimer < BUTTON_ANIM_LENGTH;
			this->_displayTimer -= (!this->displayed && this->_displayTimer > 0) * BUTTON_HIDE_SPEED_FACTOR;
			this->_enableTimer = this->enabled * CURSOR_DISP_ANIM_LENGTH;

			if (this->_displayTimer > BUTTON_DISPLAY_ANIM_LENGTH + BUTTON_LENGTHEN_ANIM_LENGTH) {
				sf::Color tint{
					255, 255, 255,
					static_cast<sf::Uint8>(255 * (this->_displayTimer - BUTTON_DISPLAY_ANIM_LENGTH - BUTTON_LENGTHEN_ANIM_LENGTH) / BUTTON_RENDER_ANIM_LENGTH)
				};

				this->_textImg.setColor(tint);
				if (this->enabled) {
					this->_cursImg.setColor(tint);
					this->_textImgBlur.setColor(tint);
				}
			} else if (this->_displayTimer > BUTTON_DISPLAY_ANIM_LENGTH) {
				this->_btnImg.setTextureRect({
					(
						-543 - this->_enableTimer * SELECTED_BUTTON_LENGTH_EXTEND / CURSOR_DISP_ANIM_LENGTH
					) * (
						this->_displayTimer - BUTTON_DISPLAY_ANIM_LENGTH
					) / BUTTON_LENGTHEN_ANIM_LENGTH,
					0,
					604 + this->_enableTimer * SELECTED_BUTTON_LENGTH_EXTEND / CURSOR_DISP_ANIM_LENGTH,
					60,
				});
				this->_textImg.setColor(sf::Color::Transparent);
				this->_cursImg.setColor(sf::Color::Transparent);
				this->_textImgBlur.setColor(sf::Color::Transparent);
			} else {
				this->_btnImg.setTextureRect({0, 0, 60, 60});
				this->_btnImg.setColor(sf::Color{255, 255, 255, static_cast<sf::Uint8>(204 * this->_displayTimer / BUTTON_DISPLAY_ANIM_LENGTH)});
				this->_textImg.setColor(sf::Color::Transparent);
				this->_cursImg.setColor(sf::Color::Transparent);
				this->_textImgBlur.setColor(sf::Color::Transparent);
			}
		} else {
			this->_enableTimer += this->enabled && this->_enableTimer < CURSOR_DISP_ANIM_LENGTH;
			this->_enableTimer -= !this->enabled && this->_enableTimer > 0;
			this->_btnImg.setTextureRect({
				-543 - this->_enableTimer * SELECTED_BUTTON_LENGTH_EXTEND / CURSOR_DISP_ANIM_LENGTH,
				0,
				604 + this->_enableTimer * SELECTED_BUTTON_LENGTH_EXTEND / CURSOR_DISP_ANIM_LENGTH,
				60,
			});
			this->_cursImg.setColor(sf::Color{255, 255, 255, static_cast<sf::Uint8>(255 * this->_enableTimer / CURSOR_DISP_ANIM_LENGTH)});
			this->_textImgBlur.setColor(sf::Color{255, 255, 255, static_cast<sf::Uint8>(255 * this->_enableTimer / CURSOR_DISP_ANIM_LENGTH)});
		}
	}

	void MenuItem::render() const
	{
		game->textureMgr.render(this->_btnImg);
		game->textureMgr.render(this->_cursImg);
		game->textureMgr.render(this->_textImg);
		game->textureMgr.render(this->_textImgBlur);
	}
}