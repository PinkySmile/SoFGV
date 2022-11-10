//
// Created by PinkySmile on 15/09/2022.
//

#ifndef SOFGV_DELAYINPUT_HPP
#define SOFGV_DELAYINPUT_HPP


#include <list>
#include <memory>
#include "RollbackInput.hpp"

namespace SpiralOfFate
{
	class DelayInput : public RollbackInput {
	private:
		std::shared_ptr<IInput> _input;
		unsigned int _delay;
		std::list<InputStruct> _delayBuffer;

	public:
		DelayInput(std::shared_ptr<IInput> input);
		void update() override;
		void setDelay(unsigned delay);
		bool hasInputs() override;
		void flush(unsigned delay);
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //SOFGV_DELAYINPUT_HPP
