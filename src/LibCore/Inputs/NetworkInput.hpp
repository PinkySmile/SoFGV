//
// Created by PinkySmile on 15/09/2022.
//

#ifndef SOFGV_NETWORKINPUT_HPP
#define SOFGV_NETWORKINPUT_HPP


#include "IInput.hpp"
#include "Resources/Network/Connection.hpp"

namespace SpiralOfFate
{
	class NetworkInput : public IInput {
	private:
		IInput &_input;
		Connection &_connection;

	public:
		NetworkInput(IInput &input, Connection &connection);
		bool isPressed(InputEnum input) const override;
		InputStruct getInputs() const override;
		void update() override;
		void consumeEvent(const sf::Event &event) override;
		std::string getName() const override;
		std::vector<std::string> getKeyNames() const override;
	};
}


#endif //SOFGV_NETWORKINPUT_HPP
