//
// Created by PinkySmile on 15/01/2019.
//

#include "../Logger.hpp"
#include "Screen.hpp"
#include "Game.hpp"

namespace SpiralOfFate
{
	Screen::Screen(const std::string &title) :
#ifdef __ANDROID__
		sf::RenderWindow(sf::VideoMode::getDesktopMode(), title), _title(title)
#else
		sf::RenderWindow(sf::VideoMode(1680, 960), title), _title(title)
#endif
	{
		game->logger.info("Opening game window \"" + title + "\"");
		this->setFramerateLimit(60);
	}

	Screen::Screen(const Screen &other) :
		sf::RenderWindow(sf::VideoMode(other.getSize().x, other.getSize().y), other.getTitle())
	{
		game->logger.info("Opening game window \"" + other.getTitle() + "\"");
		this->_title = other.getTitle();
		this->setFramerateLimit(60);
		this->setSize(other.getSize());
		this->setPosition(other.getPosition());
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
		sf::RenderWindow::setTitle(title);
	}

	void	Screen::handleEvents()
	{
		sf::Event	event;

		while (this->pollEvent(event))
			if (event.type == sf::Event::Closed)
				this->close();
	}

	void	Screen::displayElement(sf::IntRect rect, sf::Color color)
	{
		this->_rect.setPosition(sf::Vector2f(rect.left, rect.top));
		this->_rect.setSize(sf::Vector2f(rect.width, rect.height));
		this->_rect.setFillColor(color);
		this->draw(this->_rect);
	}

	void    Screen::borderColor(float thickness, const sf::Color &color)
	{
		this->_rect.setOutlineColor(color);
		this->_text.setOutlineColor(color);
		this->_rect.setOutlineThickness(thickness);
		this->_text.setOutlineThickness(thickness);
	}

	void    Screen::fillColor(const sf::Color &color)
	{
		this->_rect.setFillColor(color);
		this->_text.setFillColor(color);
	}

	void	Screen::setFont(const sf::Font &font)
	{
		this->_text.setFont(font);
	}

	void	Screen::textSize(const size_t &size)
	{
		this->_text.setCharacterSize(size);
	}

	void	Screen::displayElement(const std::string &str, sf::Vector2f pos, float boxSize, TextAlign align)
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
		this->_text.setPosition(pos);
		this->_text.setString(str);
		this->draw(this->_text);
	}

	void	Screen::displayElement(sf::Sprite &sprite, sf::Vector2f pos)
	{
		sprite.setPosition(pos);
		this->displayElement(sprite);
	}

	void	Screen::displayElement(sf::Sprite &sprite)
	{
		this->draw(sprite);
	}

	void	Screen::displayElement(const sf::Texture &texture, sf::Vector2f pos)
	{
		sf::Sprite sprite;

		sprite.setTexture(texture);
		this->displayElement(sprite, pos);
	}

	float Screen::getTextSize(const std::string &txt) const
	{
		float size = 0;

		for (char c : txt)
			size += this->_text.getFont()->getGlyph(c, this->_text.getCharacterSize(), false).advance;
		return size;
	}
}