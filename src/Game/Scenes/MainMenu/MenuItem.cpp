//
// Created by PinkySmile on 27/09/2022.
//

#include "MenuItem.hpp"
#include "Resources/Game.hpp"

#define CURSOR_DISP_ANIM_LENGTH 15
#define SELECTED_BUTTON_LENGTH_EXTEND 60
#define BUTTON_TEXT_TEXTURE_SIZE Vector2u{600, 60}
#define DISABLE_DIM 100

namespace SpiralOfFate
{
	MenuItem::MenuItem(const sf::Font &font, unsigned index, const MenuItemSkeleton &skeleton) :
		_index(index),
		_onClick(skeleton.onClick),
		description(skeleton.desc)
	{
		sf::Text text;
		sf::Shader shader;

		text.setPosition({0, -12});
		text.setFont(font);
		text.setOutlineThickness(0);
		text.setCharacterSize(50);
		text.setString(skeleton.button);
		this->disabled = !skeleton.onClick;

		this->_normalText.create(BUTTON_TEXT_TEXTURE_SIZE.x, BUTTON_TEXT_TEXTURE_SIZE.y);
		this->_normalText.clear(sf::Color{255, 255, 255, 0});
		this->_normalText.draw(text);
		this->_normalText.display();

		shader.loadFromFile("assets/ui/blur.frag", sf::Shader::Fragment);
		shader.setUniform("offsetFactor", sf::Vector2f{0.0025, 0.0025});
		shader.setUniform("source", sf::Shader::CurrentTexture);
		this->_blurredText.create(BUTTON_TEXT_TEXTURE_SIZE.x, BUTTON_TEXT_TEXTURE_SIZE.y);
		this->_blurredText.clear(sf::Color{255, 255, 255, 0});
		this->_blurredText.draw(text, &shader);
		this->_blurredText.display();

		this->_btnImg.textureHandle = game->textureMgr.load("assets/ui/buttonbar.png");
		this->_btnImg.setPosition({842, 321.f + MENU_ITEM_SPACING * index});
		this->_btnImg.setColor(sf::Color::Transparent);

		this->_cursImg.textureHandle = game->textureMgr.load("assets/ui/cursor.png");
		this->_cursImg.setPosition({842, 339.f + MENU_ITEM_SPACING * index});
		this->_cursImg.setColor(sf::Color::Transparent);

		this->_textImg.setTexture(this->_normalText.getTexture());
		this->_textImg.setPosition({868, 331.f + MENU_ITEM_SPACING * index});
		this->_textImg.setColor(sf::Color::Transparent);

		this->_textImgBlur.setTexture(this->_blurredText.getTexture());
		this->_textImgBlur.setPosition(this->_textImg.getPosition());
		this->_textImgBlur.setColor(sf::Color::Transparent);
	}

	MenuItem::~MenuItem()
	{
		game->textureMgr.remove(this->_btnImg.textureHandle);
		game->textureMgr.remove(this->_cursImg.textureHandle);
	}

	void MenuItem::update(bool pressed)
	{
		if (this->isAnimationFinished() && pressed && this->selected) {
			if (!this->disabled) {
				assert_exp(this->_onClick);
				game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
				this->_onClick();
			} else
				game->soundMgr.play(BASICSOUND_MENU_CANCEL);
		}
		if (this->displayed * BUTTON_ANIM_LENGTH != this->_displayTimer) {
			this->_displayTimer += (this->displayed && this->_displayTimer < BUTTON_ANIM_LENGTH) * animationSpeed;
			this->_displayTimer -= (!this->displayed && this->_displayTimer > 0) * animationSpeed;
			this->_enableTimer = this->selected * CURSOR_DISP_ANIM_LENGTH;

			if (this->_displayTimer < 0)
				this->_displayTimer = 0;
			if (this->_displayTimer > BUTTON_ANIM_LENGTH)
				this->_displayTimer = BUTTON_ANIM_LENGTH;
			if (this->_displayTimer > BUTTON_DISPLAY_ANIM_LENGTH + BUTTON_LENGTHEN_ANIM_LENGTH) {
				sf::Color tint{
					255, 255, 255,
					static_cast<sf::Uint8>(255 * (this->_displayTimer - BUTTON_DISPLAY_ANIM_LENGTH - BUTTON_LENGTHEN_ANIM_LENGTH) / BUTTON_RENDER_ANIM_LENGTH)
				};

				if (this->disabled) {
					tint.r -= DISABLE_DIM;
					tint.g -= DISABLE_DIM;
					tint.b -= DISABLE_DIM;
				}
				this->_textImg.setColor(tint);
				if (this->selected) {
					this->_cursImg.setColor(tint);
					this->_textImgBlur.setColor(tint);
				} else if (this->_cursImg.getColor().a) {
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
			sf::Color tint{255, 255, 255, static_cast<sf::Uint8>(255 * this->_enableTimer / CURSOR_DISP_ANIM_LENGTH)};

			if (this->disabled) {
				tint.r -= DISABLE_DIM;
				tint.g -= DISABLE_DIM;
				tint.b -= DISABLE_DIM;
			}
			this->_enableTimer += this->selected && this->_enableTimer < CURSOR_DISP_ANIM_LENGTH;
			this->_enableTimer -= !this->selected && this->_enableTimer > 0;
			this->_btnImg.setTextureRect({
				-543 - this->_enableTimer * SELECTED_BUTTON_LENGTH_EXTEND / CURSOR_DISP_ANIM_LENGTH,
				0,
				604 + this->_enableTimer * SELECTED_BUTTON_LENGTH_EXTEND / CURSOR_DISP_ANIM_LENGTH,
				60,
			});
			this->_cursImg.setColor(tint);
			this->_textImgBlur.setColor(tint);
		}

		if (this->_forceRefresh) {
			sf::Color tint{
				255, 255, 255,
				static_cast<sf::Uint8>(255 * this->_enableTimer / CURSOR_DISP_ANIM_LENGTH)
			};
			sf::Color tint2{
				255, 255, 255,
				static_cast<sf::Uint8>(255 * std::max(this->_displayTimer - BUTTON_DISPLAY_ANIM_LENGTH - BUTTON_LENGTHEN_ANIM_LENGTH, 0) / BUTTON_RENDER_ANIM_LENGTH)
			};

			if (this->disabled) {
				tint.r -= DISABLE_DIM;
				tint.g -= DISABLE_DIM;
				tint.b -= DISABLE_DIM;
				tint2.r -= DISABLE_DIM;
				tint2.g -= DISABLE_DIM;
				tint2.b -= DISABLE_DIM;
			}
			this->_btnImg.setColor(sf::Color{255, 255, 255, static_cast<sf::Uint8>(204 * std::min(this->_displayTimer, BUTTON_DISPLAY_ANIM_LENGTH) / BUTTON_DISPLAY_ANIM_LENGTH)});
			this->_btnImg.setTextureRect({
				(
					-543 - this->_enableTimer * SELECTED_BUTTON_LENGTH_EXTEND / CURSOR_DISP_ANIM_LENGTH
				) * (
					std::min(this->_displayTimer, BUTTON_LENGTHEN_ANIM_LENGTH)
				) / BUTTON_LENGTHEN_ANIM_LENGTH,
				0,
				604 + this->_enableTimer * SELECTED_BUTTON_LENGTH_EXTEND / CURSOR_DISP_ANIM_LENGTH,
				60,
			});
			this->_cursImg.setColor(tint);
			this->_textImgBlur.setColor(tint);
			this->_textImg.setColor(tint2);
			this->_forceRefresh = false;
		}
	}

	void MenuItem::render() const
	{
		if (this->cursorDisplayed) {
			game->textureMgr.render(this->_btnImg);
			game->textureMgr.render(this->_cursImg);
		}
		game->screen->draw(this->_textImg);
		game->screen->draw(this->_textImgBlur);
	}

	void MenuItem::resetAnimation(bool skipButtonGrowAnimation, bool wasEnabled)
	{
		if (skipButtonGrowAnimation)
			this->_displayTimer = this->displayed * BUTTON_ANIM_LENGTH;
		else
			this->_displayTimer = !this->displayed * BUTTON_ANIM_LENGTH;
		this->_enableTimer = wasEnabled * CURSOR_DISP_ANIM_LENGTH;
		this->_forceRefresh = true;
	}

	bool MenuItem::isAnimationFinished() const
	{
		return this->displayed * BUTTON_ANIM_LENGTH == this->_displayTimer &&
		       this->selected * CURSOR_DISP_ANIM_LENGTH == this->_enableTimer;
	}
}