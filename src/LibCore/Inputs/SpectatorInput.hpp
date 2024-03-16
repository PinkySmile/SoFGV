//
// Created by PinkySmile on 25/12/23
//

#ifndef SOFGV_SPECTATORINPUT_HPP
#define SOFGV_SPECTATORINPUT_HPP


#include "RollbackInput.hpp"
#include "Resources/Network/Packet.hpp"

namespace SpiralOfFate
{
	class SpectatorInput : public RollbackInput {
	private:
		std::list<PacketInput> &_inputList;
		unsigned *_currentFrame;

	public:
		SpectatorInput(std::list<PacketInput> &list, unsigned *currentFrame = nullptr);
		void update() override;
		void skipInput();
	};
}


#endif //SOFGV_SPECTATORINPUT_HPP
