//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_TEXTUREMANAGER_HPP
#define SOFGV_TEXTUREMANAGER_HPP


#include <map>
#include <string>
#include <SFML/Graphics.hpp>
#include "Sprite.hpp"
#include "Data/Vector.hpp"
#include "Color.hpp"

namespace SpiralOfFate
{
	class TextureManager {
	private:
		unsigned _lastIndex = 0;
		std::vector<unsigned> _freedIndexes;
		std::map<unsigned, sf::Texture> _textures;
		std::map<std::string, std::pair<unsigned, unsigned>> _allocatedTextures;
		std::map<std::string, std::string> _overrideList;

		void _reload(const std::string &path, unsigned id);
	public:
		unsigned load(std::string file, Vector2u *size = nullptr, bool repeated = false);
		unsigned load(const std::string &file, std::pair<std::vector<Color>, std::vector<Color>> palette, Vector2u *size = nullptr);
		unsigned load(const Color *pixels, Vector2u size);
		Vector2u getTextureSize(unsigned id) const;
		void addRef(unsigned id);
		void remove(unsigned id);
		sf::Texture *setTexture(Sprite &sprite);
		void render(Sprite &sprite) const;
		void reloadEverything();
		void addOverride(const std::string &base, const std::string &newVal);
		void removeOverride(const std::string &base);

		static Color *loadPixels(const std::string &file, Vector2u &size);
	};
}


#endif //SOFGV_TEXTUREMANAGER_HPP
