//
// Created by PinkySmile on 04/11/2023.
//

#include <SDL2/SDL_image.h>
#include "TextureManager.hpp"
#include "Resources/Game.hpp"
#include "Logger.hpp"

namespace SpiralOfFate
{
	unsigned TextureManager::load(std::string path, Vector2u *size, bool repeated)
	{
		for (auto pos = path.find('\\'); pos != std::string::npos; pos = path.find('\\'))
			path[pos] = '/';

		auto oit = this->_overrideList.find(path);
		auto file = oit == this->_overrideList.end() ? path : oit->second;

		if (this->_allocatedTextures[file].second != 0) {
			this->_allocatedTextures[file].second++;
			game->logger.verbose("Returning already loaded file " + path);
			if (size) {
				size->x = this->_textures[this->_allocatedTextures[file].first].surface->w;
				size->y = this->_textures[this->_allocatedTextures[file].first].surface->h;
			}
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

		game->logger.debug("Loading file " + file + " (" + path + ")");
		this->_textures[index].surface = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>(
			IMG_Load(file.c_str()),
			&SDL_FreeSurface
		);
		if (!this->_textures[index].surface) {
			game->logger.error("Cannot load image into surface: " + std::string(IMG_GetError()));
			this->_freedIndexes.push_back(index);
			return 0;
		}

		this->_textures[index].texture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(
			SDL_CreateTextureFromSurface(game->screen->getSDLRenderer(), &*this->_textures[index].surface),
			&SDL_DestroyTexture
		);
		if (!this->_textures[index].texture) {
			game->logger.error("Cannot create texture from surface: " + std::string(IMG_GetError()));
			this->_freedIndexes.push_back(index);
			return 0;
		}

		if (size) {
			size->x = this->_textures[index].surface->w;
			size->y = this->_textures[index].surface->h;
		}

		this->_textures[index].repeated = repeated;
		this->_allocatedTextures[file].first = index;
		this->_allocatedTextures[file].second = 1;
		return index;
	}

	static std::string colorToString(Color color)
	{
		char buffer[10];

		color.a = 0;
		sprintf(buffer, "#%06x", color.value);
		return buffer;
	}

	unsigned TextureManager::load(const std::string &path, std::pair<std::vector<Color>, std::vector<Color>> palette, Vector2u *size)
	{
		assert_eq(palette.first.size(), palette.second.size());
		if (palette.first.empty())
			return this->load(path, size);

		bool ok = false;
		Vector2u realSize;
		std::string allocName = path;
		auto oit = this->_overrideList.find(path);
		auto file = oit == this->_overrideList.end() ? path : oit->second;

		for (auto pos = allocName.find('\\'); pos != std::string::npos; pos = allocName.find('\\'))
			allocName[pos] = '/';
		allocName += ":";
		for (unsigned i = 0; i < palette.first.size(); i++) {
			palette.first[i].a = palette.second[i].a = 255;
			if (palette.first[i] != palette.second[i]) {
				allocName += "|" + colorToString(palette.first[i]) + "," + colorToString(palette.second[i]);
				ok = true;
			}
		}
		if (this->_allocatedTextures[allocName].second != 0) {
			this->_allocatedTextures[allocName].second++;
			game->logger.verbose("Returning already loaded paletted file " + allocName);
			if (size) {
				size->x = this->_textures[this->_allocatedTextures[allocName].first].surface->w;
				size->y = this->_textures[this->_allocatedTextures[allocName].first].surface->h;
			}
			return this->_allocatedTextures[allocName].first;
		}
		if (!ok)
			return this->load(file, size);

		auto pixels = TextureManager::loadPixels(file, realSize);

		if (!pixels) {
			game->logger.debug("Loading failed");
			return 0;
		}
		for (unsigned x = 0; x < realSize.x; x++)
			for (unsigned y = 0; y < realSize.y; y++) {
				auto &color = pixels[x + y * realSize.x];

				auto it = std::find_if(palette.first.begin(), palette.first.end(), [color](Color &a){
					return a.r == color.r && a.g == color.g && a.b == color.b;
				});

				if (it != palette.first.end()) {
					auto a = color.a;

					color = palette.second[it - palette.first.begin()];
					color.a = a;
				}
			}

		unsigned index = this->load(pixels, realSize);

		delete[] pixels;
		if (index) {
			if (size)
				*size = realSize;

			this->_allocatedTextures[allocName].first = index;
			this->_allocatedTextures[allocName].second = 1;
		}
		return index;
	}

	unsigned TextureManager::load(const Color *pixels, Vector2u size)
	{
		this->_textures[this->_lastIndex].surface = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>(
			SDL_CreateRGBSurfaceWithFormat(0, size.x, size.y, 32, SDL_PIXELFORMAT_ABGR8888),
			&SDL_FreeSurface
		);
		if (!this->_textures[this->_lastIndex].surface) {
			game->logger.error("Cannot load image into surface: " + std::string(IMG_GetError()));
			return 0;
		}
		memcpy(this->_textures[this->_lastIndex].surface->pixels, pixels, size.x * size.y * 4);

		this->_textures[this->_lastIndex].texture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(
			SDL_CreateTextureFromSurface(game->screen->getSDLRenderer(), &*this->_textures[this->_lastIndex].surface),
			&SDL_DestroyTexture
		);
		if (!this->_textures[this->_lastIndex].texture) {
			game->logger.error("Cannot create texture from surface: " + std::string(IMG_GetError()));
			return 0;
		}
		return this->_lastIndex++;
	}

	Color *TextureManager::loadPixels(const std::string &file, Vector2u &size)
	{
		game->logger.debug("Loading pixels from " + file);

		// TODO: Cache this too
		SDL_Surface *surface = IMG_Load(file.c_str());

		if (!surface) {
			game->logger.error("Failed to load image to surface: " + std::string(IMG_GetError()));
			return nullptr;
		}
		size.x = surface->w;
		size.y = surface->h;

		Color * buffer = new Color[size.x * size.y];
		auto ptr = (char *)surface->pixels;
		auto format = surface->format;
		std::pair<unsigned, unsigned> r = {format->Rmask, format->Rshift};
		std::pair<unsigned, unsigned> g = {format->Gmask, format->Gshift};
		std::pair<unsigned, unsigned> b = {format->Bmask, format->Bshift};
		std::pair<unsigned, unsigned> a = {format->Amask, format->Ashift};

		for (unsigned y = 0; y < size.y; y++) {
			auto ptr2 = ptr;

			for (unsigned x = 0; x < size.x; x++) {
				buffer[x + y * size.x].r = (*reinterpret_cast<unsigned *>(ptr2) & r.first) >> r.second;
				buffer[x + y * size.x].g = (*reinterpret_cast<unsigned *>(ptr2) & g.first) >> g.second;
				buffer[x + y * size.x].b = (*reinterpret_cast<unsigned *>(ptr2) & b.first) >> b.second;
				buffer[x + y * size.x].a = (*reinterpret_cast<unsigned *>(ptr2) & a.first) >> a.second;
				ptr2 += format->BytesPerPixel;
			}
			ptr += surface->pitch;
		}
		SDL_FreeSurface(surface);
		return buffer;
	}

	void TextureManager::remove(unsigned int id)
	{
		if (!id)
			return;

		for (auto &[loadedPath, attr] : this->_allocatedTextures)
			if (attr.first == id && attr.second) {
				attr.second--;
				if (attr.second) {
					game->logger.verbose("Remove ref to " + loadedPath);
					return;
				}
				game->logger.debug("Destroying texture " + loadedPath);
				break;
			}

		auto it = this->_textures.find(id);

		assert_exp(it != this->_textures.end());
		this->_textures.erase(it);
		this->_freedIndexes.push_back(id);
	}

	void TextureManager::setTexture(Sprite &sprite)
	{
		assert_exp(sprite.textureHandle);
		sprite.texture = &*this->_textures.at(sprite.textureHandle).texture;
		sprite.surface = &*this->_textures.at(sprite.textureHandle).surface;
	}

	void TextureManager::render(Sprite &sprite) const
	{
		if (!sprite.textureHandle)
			return;
		sprite.texture = &*this->_textures.at(sprite.textureHandle).texture;
		sprite.surface = &*this->_textures.at(sprite.textureHandle).surface;
		game->screen->displayElement(sprite);
	}

	void TextureManager::addRef(unsigned int id)
	{
		if (id == 0)
			return;
		for (auto &[loadedPath, attr] : this->_allocatedTextures)
			if (attr.first == id && attr.second) {
				attr.second++;
				assert_exp(attr.second > 1);
				game->logger.verbose("Adding ref to " + loadedPath);
				return;
			}
	}

	Vector2u TextureManager::getTextureSize(unsigned int id) const
	{
		if (id == 0)
			return {0, 0};

		auto &s = this->_textures.at(id).surface;

		return { static_cast<unsigned int>(s->w), static_cast<unsigned int>(s->h) };
	}

	void TextureManager::reloadEverything()
	{
		for (auto &[loadedPath, attr] : this->_allocatedTextures)
			if (attr.second) {
				game->logger.debug("Reloading " + loadedPath);
				this->_reload(loadedPath, attr.first);
			}
	}

	void TextureManager::_reload(const std::string &path, unsigned id)
	{
		Vector2u realSize;
		std::string p = path;
		size_t pos = p.find(':');
		auto elem = p.substr(0, pos);
		auto oit = this->_overrideList.find(elem);
		auto real = oit == this->_overrideList.end() ? elem : oit->second;
		Color *pixels = TextureManager::loadPixels(real, realSize);
		std::vector<Color> pal1;
		std::vector<Color> pal2;

		if (!pixels)
			return game->logger.debug("Loading failed");
		p = path.substr(pos + 1);
		pos = path.find('|');
		while (pos != std::string::npos) {
			size_t pos2 = p.find(',');
			Color col{};

			col.value = std::stoul(p.substr(pos + 1, 6), nullptr, 16);
			col.a = 255;
			pal1.push_back(col);
			col.value = std::stoul(p.substr(pos2 + 1, 6), nullptr, 16);
			col.a = 255;
			pal2.push_back(col);
			p = path.substr(pos + 1);
			pos = p.find('|');
		}
		if (!pal1.empty())
			for (unsigned x = 0; x < realSize.x; x++)
				for (unsigned y = 0; y < realSize.y; y++) {
					auto &color = pixels[x + y * realSize.x];
					auto oldA = color.a;
					auto it = std::find_if(pal1.begin(), pal1.end(), [color](Color &a){
						return a.r == color.r && a.g == color.g && a.b == color.b;
					});

					if (it != pal1.end())
						color = pal2[it - pal1.begin()];
					color.a = oldA;
				}

		game->logger.debug("Reloading resulting image (" + std::to_string(pal1.size()) + " paletted colors)");
		this->_textures[this->_lastIndex].surface = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>(
			SDL_CreateRGBSurfaceWithFormat(0, realSize.x, realSize.y, 32, SDL_PIXELFORMAT_ABGR8888),
			&SDL_FreeSurface
		);
		if (!this->_textures[this->_lastIndex].surface) {
			game->logger.error("Cannot load image into surface: " + std::string(IMG_GetError()));
			delete[] pixels;
			return;
		}
		memcpy(this->_textures[this->_lastIndex].surface->pixels, pixels, realSize.x * realSize.y * 4);

		this->_textures[this->_lastIndex].texture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(
			SDL_CreateTextureFromSurface(game->screen->getSDLRenderer(), &*this->_textures[this->_lastIndex].surface),
			&SDL_DestroyTexture
		);
		if (!this->_textures[this->_lastIndex].texture)
			game->logger.error("Cannot create texture from surface: " + std::string(IMG_GetError()));
		delete[] pixels;
	}

	void TextureManager::addOverride(const std::string &base, const std::string &newVal)
	{
		this->_overrideList[base] = newVal;
	}

	void TextureManager::removeOverride(const std::string &base)
	{
		auto it = this->_overrideList.find(base);

		if (it == this->_overrideList.end())
			return;
		this->_overrideList.erase(it);
	}
}
