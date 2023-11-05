//
// Created by PinkySmile on 04/11/2023.
//

#ifndef THFGAME_RESSOURCES_HPP
#define THFGAME_RESSOURCES_HPP


#include <SDL2/SDL.h>
#include <SFML/Graphics.hpp>
#include "Data/Color.hpp"
#include "Sprite.hpp"

namespace SpiralOfFate
{
#define EVENT_WINDOW_CLOSED SDL_QUIT

	typedef SDL_Event Event;

	struct ViewPort {
		SDL_Rect rect;
	};

	class Screen {
	private:
		std::string _title;
		Uint32 start = 0;
		SDL_Window *_window;
		SDL_Renderer *_renderer;
		int _frameCounter = 0;
		SDL_DisplayMode _videoMode;
		SDL_Rect _view;

	public:
		explicit Screen(const std::string &title = "FG");
		~Screen();

		enum TextAlign {
			ALIGN_LEFT,
			ALIGN_CENTER,
			ALIGN_RIGHT
		};

		SDL_Window *getSDLWindow();
		SDL_Renderer *getSDLRenderer();

		float getTextSize(const std::string &txt) const;
		const std::string &getTitle() const;
		void setTitle(const std::string &);
		void clear(const Color &color = {255, 255, 255});
		void display();
		void setSize(const sf::Vector2i &);
		void borderColor(float thickness = 0, const sf::Color &color = sf::Color(0, 0, 0, 255));
		void fillColor(const sf::Color &color = sf::Color(255, 255, 255, 255));
		void setFont(const sf::Font &font);
		void textSize(const size_t &size);
		void displayElement(sf::IntRect rect, sf::Color color);
		void displayElement(const sf::String &str, sf::Vector2f pos, float size = 0, TextAlign = ALIGN_LEFT);
		void displayElement(const sf::Texture &texture, sf::Vector2f);
		void displayElement(sf::Sprite &sprite, sf::Vector2f);
		void displayElement(const sf::Sprite &sprite);
		void displayElement(const Sprite &sprite);
		void draw(const sf::Sprite &);
		void draw(const sf::CircleShape &);
		void draw(const sf::RectangleShape &);
		void draw(const sf::VertexArray &, const sf::Texture * = nullptr);
		void setIcon(size_t, size_t, const void *);
		void setFramerateLimit(int);
		bool isOpen();
		void close();
		void setView(const ViewPort &view);
		bool pollEvent(Event &);
	};
}

void libraryInit();
void libraryUnInit();

#endif //THFGAME_RESSOURCES_HPP
