//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_SPRITE_HPP
#define BATTLE_SPRITE_HPP


#include <SFML/Graphics.hpp>

namespace Battle
{
	struct Sprite : public sf::Sprite {
		unsigned textureHandle = 0;
	};
}


#endif //BATTLE_SPRITE_HPP
