//
// Created by Gegel85 on 24/09/2021.
//

#ifndef SOFGV_IINPUT_HPP
#define SOFGV_IINPUT_HPP


#include <SFML/Window/Event.hpp>
#include "InputEnum.hpp"
#include <vector>

namespace SpiralOfFate
{
	class IInput {
	public:
		virtual ~IInput() = default;
		virtual bool isPressed(InputEnum input) const = 0;
		virtual InputStruct getInputs() const = 0;
		virtual void update() = 0;
		virtual void consumeEvent(const sf::Event &event) = 0;
		virtual std::string getName() const = 0;
		virtual std::vector<std::string> getKeyNames() const = 0;
	};
}


#endif //SOFGV_IINPUT_HPP
