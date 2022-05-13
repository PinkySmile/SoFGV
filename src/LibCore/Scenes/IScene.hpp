//
// Created by PinkySmile on 24/09/2021.
//

#ifndef SOFGV_ISCENE_HPP
#define SOFGV_ISCENE_HPP


#include <SFML/Window/Event.hpp>

namespace SpiralOfFate
{
	class IScene {
	public:
		virtual ~IScene() = default;
		virtual void render() const = 0;
		virtual IScene *update() = 0;
		virtual void consumeEvent(const sf::Event &event) = 0;
	};
}


#endif //SOFGV_ISCENE_HPP
