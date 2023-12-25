//
// Created by PinkySmile on 25/12/23
//

#ifndef SOFGV_SPECTATORINPUTMANAGER_HPP
#define SOFGV_SPECTATORINPUTMANAGER_HPP


#include "Inputs/RollbackInput.hpp"
#include "Packet.hpp"
#include "Inputs/SpectatorInput.hpp"

namespace SpiralOfFate
{
	class SpectatorInputManager {
	private:
		unsigned _end = 0;
		unsigned _currentFrame = 0;
		unsigned _lastReceivedFrame = 0;
		std::list<PacketInput> _leftList;
		std::list<PacketInput> _rightList;
		std::shared_ptr<SpectatorInput> _leftInput = std::make_shared<SpectatorInput>(this->_leftList, &this->_currentFrame);
		std::shared_ptr<SpectatorInput> _rightInput = std::make_shared<SpectatorInput>(this->_rightList);

	public:
		void update();
		bool parseReplayPacket(PacketReplay &packet);
		std::pair<std::shared_ptr<SpectatorInput>, std::shared_ptr<SpectatorInput>> getInputs() const;
		bool hasInputs() const;
		unsigned getBufferSize() const;
		unsigned getEnd() const;
		unsigned getCurrentFrame() const;
		unsigned getLastReceivedFrame() const;
	};
}


#endif //SOFGV_SPECTATORINPUTMANAGER_HPP
