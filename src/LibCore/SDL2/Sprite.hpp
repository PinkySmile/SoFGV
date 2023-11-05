//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_SPRITE_HPP
#define SOFGV_SPRITE_HPP


#include <SFML/Graphics.hpp>
#include <SDL2/SDL.h>

namespace SpiralOfFate
{
	struct Sprite : public sf::Sprite {
		unsigned textureHandle = 0;
		SDL_Texture *texture = nullptr;
		SDL_Surface *surface = nullptr;
	};
}


#endif //SOFGV_SPRITE_HPP
