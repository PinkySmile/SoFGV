//
// Created by Gegel85 on 24/09/2021.
//

#ifndef BATTLE_IINPUT_HPP
#define BATTLE_IINPUT_HPP


#include <SFML/Window/Event.hpp>
#include "InputEnum.hpp"

namespace Battle
{
	class IInput {
	public:
		virtual ~IInput() = default;
		virtual bool isPressed(InputEnum input) const = 0;
		virtual InputStruct getInputs() const = 0;
		virtual void update() = 0;
		virtual void consumeEvent(const sf::Event &event) = 0;
	};
}


#endif //BATTLE_IINPUT_HPP
