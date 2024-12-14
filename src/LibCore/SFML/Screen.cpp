//
// Created by PinkySmile on 15/01/2019.
//

#include "Logger.hpp"
#include "Screen.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	Screen::Screen(const std::string &title) :
#ifdef __ANDROID__
		sf::RenderWindow(sf::VideoMode::getDesktopMode(), title), _title(title)
#else
		sf::RenderWindow([]{
			auto desktop = sf::VideoMode::getDesktopMode();

			desktop.height -= 40;
			return desktop;
		}(), title), _title(title)
#endif
	{
		this->setPosition(sf::Vector2i(0, 0));
		game->logger.info("Opening game window \"" + title + "\"");
	}

	Screen::Screen(const Screen &other) :
		sf::RenderWindow(sf::VideoMode(other.getSize().x, other.getSize().y), other.getTitle())
	{
		game->logger.info("Opening game window \"" + other.getTitle() + "\"");
		this->_title = other.getTitle();
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

	void Screen::displayElement(sf::IntRect rect, sf::Color color)
	{
		this->_rect.setPosition(sf::Vector2f(rect.left, rect.top));
		this->_rect.setSize(sf::Vector2f(rect.width, rect.height));
		this->_rect.setFillColor(color);
		this->draw(this->_rect);
	}

	void Screen::borderColor(float thickness, const sf::Color &color)
	{
		this->_rect.setOutlineColor(color);
		this->_text.setOutlineColor(color);
		this->_rect.setOutlineThickness(thickness);
		this->_text.setOutlineThickness(thickness);
	}

	void Screen::fillColor(const sf::Color &color)
	{
		this->_rect.setFillColor(color);
		this->_text.setFillColor(color);
	}

	void Screen::setFont(const sf::Font &font)
	{
		this->_text.setFont(font);
	}

	void Screen::textSize(const size_t &size)
	{
		this->_text.setCharacterSize(size);
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
		this->_text.setPosition(pos);
		this->_text.setString(str);
		this->draw(this->_text);
	}

	void Screen::displayElement(sf::Sprite &sprite, sf::Vector2f pos)
	{
		sprite.setPosition(pos);
		this->displayElement(sprite);
	}

	void Screen::displayElement(const sf::Sprite &sprite)
	{
		this->draw(sprite);
	}

	void Screen::displayElement(const sf::Texture &texture, sf::Vector2f pos)
	{
		sf::Sprite sprite;

		sprite.setTexture(texture);
		this->displayElement(sprite, pos);
	}

	void Screen::displayShrunkRect(const PreparedShrunkRect &sprite, sf::IntRect rect)
	{
		sf::Sprite temp;
		int w1 = std::floor(rect.width / 2);
		int w2 = rect.width - w1;
		int h1 = std::floor(rect.height / 2);
		int h2 = rect.height - h1;

		temp.setPosition(rect.left, rect.top);
		temp.setTexture(sprite.topLeft.getTexture());
		temp.setTextureRect({
			0, 0,
			w1, h1
		});
		this->draw(temp);

		temp.setPosition(rect.left + w1, rect.top);
		temp.setTexture(sprite.topRight.getTexture());
		temp.setTextureRect({
			static_cast<int>(sprite.texSize.x / 2 - w2), 0,
			w2, h1
		});
		this->draw(temp);

		temp.setPosition(rect.left, rect.top + h1);
		temp.setTexture(sprite.bottomLeft.getTexture());
		temp.setTextureRect({
			0, static_cast<int>(sprite.texSize.y / 2 - h2),
			w1, h2
		});
		this->draw(temp);

		temp.setPosition(rect.left + w1, rect.top + h1);
		temp.setTexture(sprite.bottomRight.getTexture());
		temp.setTextureRect({
			static_cast<int>(sprite.texSize.x / 2 - w2), static_cast<int>(sprite.texSize.y / 2 - h2),
			w2, h2
		});
		this->draw(temp);
	}

	float Screen::getTextSize(const std::string &txt) const
	{
		float size = 0;

		for (size_t i = 0; i < txt.size(); i++) {
			if (i != 0)
				size += this->_text.getFont()->getKerning(txt[i - 1], txt[i], this->_text.getCharacterSize());
			size += this->_text.getFont()->getGlyph(txt[i], this->_text.getCharacterSize(), false).advance;
		}
		return size;
	}

	std::unique_ptr<PreparedShrunkRect> Screen::prepareShrunkRect(sf::Sprite &sprite)
	{
		auto result = std::make_unique<PreparedShrunkRect>();
		auto texture = sprite.getTexture();

		assert_exp(texture);
		assert_exp(sprite.getPosition().x == 0);
		assert_exp(sprite.getPosition().y == 0);

		auto size = texture->getSize();

		assert_exp(size.x % 2);
		assert_exp(size.y % 2);

		result->texSize = size;
		result->topLeft.create(result->texSize.x / 2 + 1, result->texSize.y / 2 + 1);
		result->topLeft.clear(sf::Color::Transparent);
		result->topRight.create(result->texSize.x / 2 + 1, result->texSize.y / 2 + 1);
		result->topRight.clear(sf::Color::Transparent);
		result->bottomLeft.create(result->texSize.x / 2 + 1, result->texSize.y / 2 + 1);
		result->bottomLeft.clear(sf::Color::Transparent);
		result->bottomRight.create(result->texSize.x / 2 + 1, result->texSize.y / 2 + 1);
		result->bottomRight.clear(sf::Color::Transparent);

		sprite.setTextureRect({
			0, 0,
			static_cast<int>(result->texSize.x / 2 + 1), static_cast<int>(result->texSize.y / 2 + 1)
		});
		result->topLeft.draw(sprite);
		result->topLeft.display();

		sprite.setTextureRect({
			static_cast<int>(result->texSize.x / 2), 0,
			static_cast<int>(result->texSize.x / 2 + 1), static_cast<int>(result->texSize.y / 2 + 1)
		});
		result->topRight.draw(sprite);
		result->topRight.display();

		sprite.setTextureRect({
			0, static_cast<int>(result->texSize.y / 2),
			static_cast<int>(result->texSize.x / 2 + 1), static_cast<int>(result->texSize.y / 2 + 1)
		});
		result->bottomLeft.draw(sprite);
		result->bottomLeft.display();

		sprite.setTextureRect({
			static_cast<int>(result->texSize.x / 2), static_cast<int>(result->texSize.y / 2),
			static_cast<int>(result->texSize.x / 2 + 1), static_cast<int>(result->texSize.y / 2 + 1)
		});
		result->bottomRight.draw(sprite);
		result->bottomRight.display();
		return result;
	}
}