//
// Created by PinkySmile on 27/09/2022.
//

#include "MenuItem.hpp"
#include "Resources/Game.hpp"

#define CURSOR_DISP_ANIM_LENGTH 15
#define SELECTED_BUTTON_LENGTH_EXTEND 60
#define BUTTON_TEXT_TEXTURE_SIZE Vector2u{600, 60}
#define BLUR_EXTRA 20
#define DISABLE_DIM 100
#include <GLES2/gl2.h> // Ensure you have the GLES2 headers for Emscripten

namespace SpiralOfFate
{
	MenuItem::MenuItem(const sf::Font &font, unsigned index, const MenuItemSkeleton &skeleton) :
		_normalText{BUTTON_TEXT_TEXTURE_SIZE},
		_blurredText{BUTTON_TEXT_TEXTURE_SIZE + Vector2u{BLUR_EXTRA * 2, BLUR_EXTRA * 2}},
		_btnImg{ game->textureMgr.load("assets/ui/buttonbar.png") },
		_cursImg{ game->textureMgr.load("assets/ui/cursor.png") },
		_textImg{ game->textureMgr.getTexture(0) },
		_textImgBlur{ game->textureMgr.getTexture(0) },
		_index(index),
		_onClick(skeleton.onClick),
		description(skeleton.desc)
	{
		sf::Text text{font};
		sf::Shader shader;

		text.setPosition({0, -12});
		text.setOutlineThickness(0);
		text.setCharacterSize(50);
		text.setString(skeleton.button);
		if (!skeleton.onClick) {
			this->disabled = true;
			this->description += "\n\nComing soon!";
		}

		this->_normalText.clear(Color{255, 255, 255, 0});
		this->_normalText.draw(text);
		this->_normalText.display();

#ifdef __EMSCRIPTEN__
		assert_exp(shader.loadFromFile("assets/ui/blur_web.vert", "assets/ui/blur_web.frag"));
#else
		assert_exp(shader.loadFromFile("assets/ui/blur.frag", sf::Shader::Type::Fragment));
		shader.setUniform("source", sf::Shader::CurrentTexture);
#endif
		shader.setUniform("offsetFactor", sf::Vector2f{0.0035f, 0.0035f});

		auto pos = text.getPosition();
		pos.x += BLUR_EXTRA;
		pos.y += BLUR_EXTRA;
		text.setPosition(pos);
		this->_blurredText.clear(Color{255, 255, 255, 0});
		this->_blurredText.draw(text, &shader);
		this->_blurredText.display();

		this->_btnImg.setPosition({842, 321.f + MENU_ITEM_SPACING * index});
		this->_btnImg.setColor(Color::Transparent);

		this->_cursImg.setPosition({842, 339.f + MENU_ITEM_SPACING * index});
		this->_cursImg.setColor(Color::Transparent);

		this->_textImg.setTexture(this->_normalText.getTexture(), true);
		this->_textImg.setPosition({868, 331.f + MENU_ITEM_SPACING * index});
		this->_textImg.setColor(Color::Transparent);

		pos = this->_textImg.getPosition();
		pos.x -= BLUR_EXTRA;
		pos.y -= BLUR_EXTRA;
		this->_textImgBlur.setTexture(this->_blurredText.getTexture(), true);
		this->_textImgBlur.setPosition(pos);
		this->_textImgBlur.setColor(Color::Transparent);
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
				Color tint{
					255, 255, 255,
					static_cast<uint8_t>(255 * (this->_displayTimer - BUTTON_DISPLAY_ANIM_LENGTH - BUTTON_LENGTHEN_ANIM_LENGTH) / BUTTON_RENDER_ANIM_LENGTH)
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
				this->_textImg.setColor(Color::Transparent);
				this->_cursImg.setColor(Color::Transparent);
				this->_textImgBlur.setColor(Color::Transparent);
			} else {
				this->_btnImg.setTextureRect({0, 0, 60, 60});
				this->_btnImg.setColor(Color{255, 255, 255, static_cast<uint8_t>(204 * this->_displayTimer / BUTTON_DISPLAY_ANIM_LENGTH)});
				this->_textImg.setColor(Color::Transparent);
				this->_cursImg.setColor(Color::Transparent);
				this->_textImgBlur.setColor(Color::Transparent);
			}
		} else {
			Color tint{255, 255, 255, static_cast<uint8_t>(255 * this->_enableTimer / CURSOR_DISP_ANIM_LENGTH)};

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
			Color tint{
				255, 255, 255,
				static_cast<uint8_t>(255 * this->_enableTimer / CURSOR_DISP_ANIM_LENGTH)
			};
			Color tint2{
				255, 255, 255,
				static_cast<uint8_t>(255 * std::max(this->_displayTimer - BUTTON_DISPLAY_ANIM_LENGTH - BUTTON_LENGTHEN_ANIM_LENGTH, 0) / BUTTON_RENDER_ANIM_LENGTH)
			};

			if (this->disabled) {
				tint.r -= DISABLE_DIM;
				tint.g -= DISABLE_DIM;
				tint.b -= DISABLE_DIM;
				tint2.r -= DISABLE_DIM;
				tint2.g -= DISABLE_DIM;
				tint2.b -= DISABLE_DIM;
			}
			this->_btnImg.setColor(Color{255, 255, 255, static_cast<uint8_t>(204 * std::min(this->_displayTimer, BUTTON_DISPLAY_ANIM_LENGTH) / BUTTON_DISPLAY_ANIM_LENGTH)});
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
			game->screen->displayElement(this->_btnImg);
			game->screen->displayElement(this->_cursImg);
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
		return this->displayed * BUTTON_ANIM_LENGTH == this->_displayTimer;
	}
}