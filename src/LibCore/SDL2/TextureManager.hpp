//
// Created by PinkySmile on 04/11/2023.
//

#ifndef SOFGV_TEXTUREMANAGER_HPP
#define SOFGV_TEXTUREMANAGER_HPP


#include <map>
#include <string>
#include <memory>
#include <SDL2/SDL.h>
#include "Sprite.hpp"
#include "Data/Vector.hpp"
#include "Color.hpp"

namespace SpiralOfFate
{
	class TextureManager {
	private:
		struct TextureData {
			std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface{nullptr, &SDL_FreeSurface};
			std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture{nullptr, &SDL_DestroyTexture};
			bool repeated = false;
		};

		unsigned _lastIndex = 0;
		std::vector<unsigned> _freedIndexes;
		std::map<unsigned, TextureData> _textures;
		std::map<std::string, std::pair<unsigned, unsigned>> _allocatedTextures;
		std::map<std::string, std::string> _overrideList;

		void _reload(const std::string &path, unsigned id);
	public:
		static_assert(sizeof(Color) == 4);
		unsigned load(std::string file, Vector2u *size = nullptr, bool repeated = false);
		unsigned load(const std::string &file, std::pair<std::vector<Color>, std::vector<Color>> palette, Vector2u *size = nullptr);
		unsigned load(const Color *pixels, Vector2u size);
		Vector2u getTextureSize(unsigned id) const;
		void addRef(unsigned id);
		void remove(unsigned id);
		void setTexture(Sprite &sprite);
		void render(Sprite &sprite) const;
		void reloadEverything();
		void addOverride(const std::string &base, const std::string &newVal);
		void removeOverride(const std::string &base);

		static Color *loadPixels(const std::string &file, Vector2u &size);
	};
}


#endif //SOFGV_TEXTUREMANAGER_HPP
