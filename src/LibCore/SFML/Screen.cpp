//
// Created by PinkySmile on 15/01/2019.
//

#include "Logger.hpp"
#include "Screen.hpp"
#include "Resources/Game.hpp"
#ifdef __linux__
#include <X11/Xlib.h>
#endif

namespace SpiralOfFate
{
	Screen::Screen(const std::string &title) :
	#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
		sf::RenderWindow(sf::VideoMode::getDesktopMode(), title),
	#else
		sf::RenderWindow([]{
			auto desktop = sf::VideoMode::getDesktopMode();

			desktop.size.y -= 40;
			return desktop;
		}(), title),
	#endif
		_title(title)
	{
		this->setPosition(sf::Vector2i(0, 0));
		game->logger.info("Opening game window \"" + title + "\"");
	}

	Screen::Screen(const Screen &other) :
		sf::RenderWindow(sf::VideoMode(other.getSize()), other.getTitle())
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

	void Screen::displayElement(IntRect rect, Color color)
	{
		this->_rect.setPosition(Vector2(rect.position).to<float>());
		this->_rect.setSize(Vector2(rect.size).to<float>());
		this->_rect.setFillColor(color);
		this->draw(this->_rect);
	}

	void Screen::borderColor(float thickness, const Color &color)
	{
		this->_rect.setOutlineColor(color);
		this->_text.setOutlineColor(color);
		this->_rect.setOutlineThickness(thickness);
		this->_text.setOutlineThickness(thickness);
	}

	void Screen::fillColor(const Color &color)
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

	void Screen::displayShrunkRect(const PreparedShrunkRect &sprite, sf::IntRect rect)
	{
		sf::Sprite temp{ sprite.topLeft.getTexture() };
		int w1 = std::floor(rect.size.x / 2);
		int w2 = rect.size.x - w1;
		int h1 = std::floor(rect.size.y / 2);
		int h2 = rect.size.y - h1;

		temp.setPosition(Vector2(rect.position).to<float>());
		temp.setTexture(sprite.topLeft.getTexture());
		temp.setTextureRect({{0, 0}, {w1, h1}});
		this->draw(temp);

		temp.setPosition(Vector2f(rect.position.x + w1, rect.position.y));
		temp.setTexture(sprite.topRight.getTexture());
		temp.setTextureRect({
			{static_cast<int>(sprite.texSize.x / 2 - w2), 0},
			{w2, h1}
		});
		this->draw(temp);

		temp.setPosition(Vector2f(rect.position.x, rect.position.y + h1));
		temp.setTexture(sprite.bottomLeft.getTexture());
		temp.setTextureRect({
			{0, static_cast<int>(sprite.texSize.y / 2 - h2)},
			{w1, h2}
		});
		this->draw(temp);

		temp.setPosition(Vector2f(rect.position.x + w1, rect.position.y + h1));
		temp.setTexture(sprite.bottomRight.getTexture());
		temp.setTextureRect({
			{static_cast<int>(sprite.texSize.x / 2 - w2), static_cast<int>(sprite.texSize.y / 2 - h2)},
			{w2, h2}
		});
		this->draw(temp);
	}

	float Screen::getTextSize(const std::string &txt) const
	{
		float size = 0;

		for (size_t i = 0; i < txt.size(); i++) {
			if (i != 0)
				size += this->_text.getFont().getKerning(txt[i - 1], txt[i], this->_text.getCharacterSize());
			size += this->_text.getFont().getGlyph(txt[i], this->_text.getCharacterSize(), false).advance;
		}
		return size;
	}

	std::unique_ptr<PreparedShrunkRect> Screen::prepareShrunkRect(sf::Sprite &sprite)
	{
		auto result = std::make_unique<PreparedShrunkRect>();
		auto texture = sprite.getTexture();

		assert_exp(sprite.getPosition().x == 0);
		assert_exp(sprite.getPosition().y == 0);

		auto size = texture.getSize();

		assert_exp(size.x % 2);
		assert_exp(size.y % 2);

		result->texSize = size;
		assert_exp(result->topLeft.resize({result->texSize.x / 2 + 1, result->texSize.y / 2 + 1}));
		result->topLeft.clear(Color::Transparent);
		assert_exp(result->topRight.resize({result->texSize.x / 2 + 1, result->texSize.y / 2 + 1}));
		result->topRight.clear(Color::Transparent);
		assert_exp(result->bottomLeft.resize({result->texSize.x / 2 + 1, result->texSize.y / 2 + 1}));
		result->bottomLeft.clear(Color::Transparent);
		assert_exp(result->bottomRight.resize({result->texSize.x / 2 + 1, result->texSize.y / 2 + 1}));
		result->bottomRight.clear(Color::Transparent);

		sprite.setTextureRect({
			{0, 0},
			{static_cast<int>(result->texSize.x / 2 + 1), static_cast<int>(result->texSize.y / 2 + 1)}
		});
		result->topLeft.draw(sprite);
		result->topLeft.display();

		sprite.setTextureRect({
			{static_cast<int>(result->texSize.x / 2), 0},
			{static_cast<int>(result->texSize.x / 2 + 1), static_cast<int>(result->texSize.y / 2 + 1)}
		});
		result->topRight.draw(sprite);
		result->topRight.display();

		sprite.setTextureRect({
			{0, static_cast<int>(result->texSize.y / 2)},
			{static_cast<int>(result->texSize.x / 2 + 1), static_cast<int>(result->texSize.y / 2 + 1)}
		});
		result->bottomLeft.draw(sprite);
		result->bottomLeft.display();

		sprite.setTextureRect({
			{static_cast<int>(result->texSize.x / 2), static_cast<int>(result->texSize.y / 2)},
			{static_cast<int>(result->texSize.x / 2 + 1), static_cast<int>(result->texSize.y / 2 + 1)}
		});
		result->bottomRight.draw(sprite);
		result->bottomRight.display();
		return result;
	}

	IntRect::IntRect(SpiralOfFate::Vector2i pos, SpiralOfFate::Vector2i size) :
		sf::IntRect(pos, size)
	{
	}

	SpiralOfFate::IntRect::IntRect(int x, int y, int w, int h) :
		sf::IntRect({x, y}, {w, h})
	{
	}
}

void libraryInit()
{
#ifdef __linux__
	XInitThreads();
#endif
}
