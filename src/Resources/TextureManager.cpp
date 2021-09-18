//
// Created by PinkySmile on 18/09/2021
//

#include <cstring>
#include <cassert>
#include "TextureManager.hpp"
#include "Game.hpp"
#include "../Logger.hpp"

namespace Battle
{
	unsigned TextureManager::load(const std::string &file, Vector2u *size)
	{
		if (this->_allocatedTextures[file].second != 0) {
			this->_allocatedTextures[file].second++;
			logger.debug("Returning already loaded file " + file);
			return this->_allocatedTextures[file].first;
		}

		unsigned index;

		if (this->_freedIndexes.empty()) {
			this->_lastIndex += this->_lastIndex == 0;
			index = this->_lastIndex;
			this->_lastIndex++;
		} else {
			index = this->_freedIndexes.back();
			this->_freedIndexes.pop_back();
		}

		logger.debug("Loading file " + file);
		if (!this->_textures[index].loadFromFile(file)) {
			this->_freedIndexes.push_back(index);
			return 0;
		}

		if (size)
			*size = this->_textures[index].getSize();

		this->_allocatedTextures[file].first = index;
		this->_allocatedTextures[file].second = 1;
		return index;
	}

	unsigned TextureManager::load(const Color *pixels, Vector2u size)
	{
		sf::Image image;

		image.create(size.x, size.y, reinterpret_cast<const sf::Uint8 *>(pixels));
		this->_textures[this->_lastIndex].loadFromImage(image);
		return this->_lastIndex++;
	}

	Color *TextureManager::loadPixels(const std::string &file, Vector2u &size)
	{
		sf::Image image;
		Color *buffer;

		if (!image.loadFromFile(file))
			return nullptr;
		size = image.getSize();
		buffer = new Color[size.x * size.y];
		memcpy(buffer, image.getPixelsPtr(), size.x * size.y * sizeof(*buffer));
		return buffer;
	}

	void TextureManager::remove(unsigned int id)
	{
		for (auto [loadedPath, attr] : this->_allocatedTextures)
			if (attr.first == id && attr.second) {
				attr.second--;
				if (attr.second)
					return;
				break;
			}

		auto it = this->_textures.find(id);

		assert(it != this->_textures.end());
		this->_textures.erase(it);
		this->_freedIndexes.push_back(id);
	}

	void TextureManager::render(Sprite &sprite)
	{
		sprite.setTexture(this->_textures.at(sprite.textureHandle));
		game.screen->displayElement(sprite);
	}
}
