//
// Created by PinkySmile on 15/09/2022.
//

#ifndef SOFGV_REMOTEINPUT_HPP
#define SOFGV_REMOTEINPUT_HPP


#include <array>
#include <bitset>
#include "RollbackInput.hpp"
#include <Resources/Network/IConnection.hpp>

namespace SpiralOfFate
{
	class RemoteInput : public RollbackInput {
	private:
		std::list<PacketInput> _inputBuffer;
		IConnection &_connection;

	public:
		RemoteInput(IConnection &connection);
		void update() override;
		std::string getName() const override;
		bool hasInputs() override;
		void refreshInputs();
	};
} // SpiralOfFate


#endif //SOFGV_REMOTEINPUT_HPP
