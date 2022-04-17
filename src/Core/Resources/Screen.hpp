//
// Created by PinkySmile on 15/01/2019.
//

#ifndef THFGAME_RESSOURCES_HPP
#define THFGAME_RESSOURCES_HPP


#include <SFML/Graphics.hpp>

namespace SpiralOfFate
{
	class Screen : public sf::RenderWindow {
	private:
		sf::RectangleShape _rect;
		sf::Text           _text;
		sf::Clock          _clock;
		sf::Sprite         _sprite;
		std::string        _title;

	public:
		explicit Screen(const std::string &title = "FG");
		Screen(const Screen &);
		~Screen() override;

		enum TextAlign {
			ALIGN_LEFT,
			ALIGN_CENTER,
			ALIGN_RIGHT
		};

		float             getTextSize(const std::string &txt) const;
		void              handleEvents();
		const std::string &getTitle() const;
		void              setTitle(const std::string &);
		void              borderColor(float thickness = 0, const sf::Color &color = sf::Color(0, 0, 0, 255));
		void              fillColor(const sf::Color &color = sf::Color(255, 255, 255, 255));
		void              setFont(const sf::Font &font);
		void              textSize(const size_t &size);
		void              displayElement(sf::IntRect rect, sf::Color color);
		void              displayElement(const std::string &str, sf::Vector2f pos, float size = 0, TextAlign = ALIGN_LEFT);
		void              displayElement(const sf::Texture &texture, sf::Vector2f);
		void              displayElement(sf::Sprite &sprite, sf::Vector2f);
		void              displayElement(sf::Sprite &sprite);
	};
}


#endif //THFGAME_RESSOURCES_HPP
