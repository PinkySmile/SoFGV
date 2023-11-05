//
// Created by PinkySmile on 04/11/2023.
//

#include <SDL2/SDL_image.h>
#include "Logger.hpp"
#include "Screen.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	Screen::Screen(const std::string &title)
	{
		game->logger.info("Opening game window \"" + title + "\"");
		SDL_GetCurrentDisplayMode(0, &this->_videoMode);
		my_assert2(this->_window = SDL_CreateWindow(
			title.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			this->_videoMode.w,
			this->_videoMode.h,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
		), SDL_GetError());
		my_assert2(this->_renderer = SDL_CreateRenderer(this->_window, -1, 0), SDL_GetError());
		SDL_RenderSetClipRect(this->_renderer, nullptr);
	}

	Screen::~Screen()
	{
		game->logger.info("Destroying game window \"" + this->_title + "\"");
	}

	const std::string& Screen::getTitle() const
	{
		return this->_title;
	}

	void Screen::setTitle(const std::string &title)
	{
		this->_title = title;
		SDL_SetWindowTitle(this->_window, title.c_str());
	}

	void Screen::displayElement(sf::IntRect rect, sf::Color color)
	{
		//this->_rect.setPosition(sf::Vector2f(rect.left, rect.top));
		//this->_rect.setSize(sf::Vector2f(rect.width, rect.height));
		//this->_rect.setFillColor(color);
		//this->draw(this->_rect);
	}

	void Screen::borderColor(float thickness, const sf::Color &color)
	{
		//this->_rect.setOutlineColor(color);
		//this->_text.setOutlineColor(color);
		//this->_rect.setOutlineThickness(thickness);
		//this->_text.setOutlineThickness(thickness);
	}

	void Screen::fillColor(const sf::Color &color)
	{
		//this->_rect.setFillColor(color);
		//this->_text.setFillColor(color);
	}

	void Screen::setFont(const sf::Font &font)
	{
		//this->_text.setFont(font);
	}

	void Screen::textSize(const size_t &size)
	{
		//this->_text.setCharacterSize(size);
	}

	void Screen::displayElement(const sf::String &str, sf::Vector2f pos, float boxSize, TextAlign align)
	{
		switch (align) {
		case ALIGN_RIGHT:
			pos.x += boxSize - this->getTextSize(str);
			break;
		case ALIGN_CENTER:
			pos.x += (boxSize - this->getTextSize(str)) / 2;
			break;
		default:
			break;
		}
		//this->_text.setPosition(pos);
		//this->_text.setString(str);
		//this->draw(this->_text);
	}

	void Screen::displayElement(sf::Sprite &sprite, sf::Vector2f pos)
	{
		sprite.setPosition(pos);
		this->displayElement(sprite);
	}

	void Screen::displayElement(const sf::Sprite &sprite)
	{
		//this->draw(sprite);
	}

	void Screen::displayElement(const Sprite &sprite)
	{
		if (!sprite.texture)
			return;

		SDL_Point center{
			static_cast<int>(sprite.getOrigin().x),
			static_cast<int>(sprite.getOrigin().y)
		};
		SDL_Rect src{
			sprite.getTextureRect().left,
			sprite.getTextureRect().top,
			sprite.getTextureRect().width,
			sprite.getTextureRect().height,
		};
		Vector2i size;

		if (SDL_GetRendererOutputSize(this->_renderer, &size.x, &size.y) < 0)
			game->logger.error("Failed to get renderer output size: " + std::string(SDL_GetError()));

		SDL_Rect dst{
			static_cast<int>(sprite.getPosition().x - center.x + this->_view.x) * size.x / this->_view.w,
			static_cast<int>(sprite.getPosition().y - center.y + this->_view.y) * size.y / this->_view.h,
			static_cast<int>(sprite.getScale().x * sprite.surface->w) * size.x / this->_view.w,
			static_cast<int>(sprite.getScale().y * sprite.surface->h) * size.y / this->_view.h,
		};

		if (!src.w)
			src.w = sprite.surface->w;
		if (!src.h)
			src.h = sprite.surface->h;
		// TODO: handle tinting textures
		// TODO: handle repeating textures
		// TODO: handle negative coordinates
		SDL_RenderCopyEx(this->_renderer, sprite.texture, &src, &dst, sprite.getRotation(), &center, SDL_FLIP_NONE);
	}

	void Screen::displayElement(const sf::Texture &texture, sf::Vector2f pos)
	{
		sf::Sprite sprite;

		sprite.setTexture(texture);
		this->displayElement(sprite, pos);
	}

	float Screen::getTextSize(const std::string &txt) const
	{
		float size = 0;

		//for (char c : txt)
		//	size += this->_text.getFont()->getGlyph(c, this->_text.getCharacterSize(), false).advance;
		return size;
	}

	void Screen::clear(const Color &color)
	{
		SDL_SetRenderDrawColor(this->_renderer, color.r, color.g, color.b, color.a);
		SDL_RenderClear(this->_renderer);
	}

	void Screen::display()
	{
		SDL_RenderPresent(this->_renderer);

		//auto ticks = SDL_GetTicks();

		//if (1000 / FRAMES_PER_SECOND > (ticks - this->start))
		//	SDL_Delay(1000 / FRAMES_PER_SECOND - (ticks - this->start));
		//this->start = SDL_GetTicks();
	}

	void Screen::draw(const sf::Sprite &)
	{
	}

	void Screen::draw(const sf::VertexArray &, const sf::Texture *)
	{
	}

	void Screen::draw(const sf::CircleShape &)
	{
	}

	void Screen::draw(const sf::RectangleShape &)
	{
	}

	void Screen::setSize(const sf::Vector2i &size)
	{
		SDL_SetWindowSize(this->_window, size.x, size.y);
	}

	void Screen::setIcon(size_t, size_t, const void *)
	{
	}

	void Screen::setFramerateLimit(int)
	{
	}

	bool Screen::isOpen()
	{
		return this->_window != nullptr;
	}

	void Screen::close()
	{
		SDL_DestroyWindow(this->_window);
		this->_window = nullptr;
	}

	void Screen::setView(const ViewPort &view)
	{
		this->_view = view.rect;
	}

	bool Screen::pollEvent(Event &event)
	{
		return SDL_PollEvent(&event);
	}

	SDL_Window *Screen::getSDLWindow()
	{
		return this->_window;
	}

	SDL_Renderer *Screen::getSDLRenderer()
	{
		return this->_renderer;
	}
}

void libraryInit()
{
	my_assert2(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != -1, SDL_GetError());
	my_assert2(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG, IMG_GetError());
}

void libraryUnInit()
{
	IMG_Quit();
	SDL_Quit();
}