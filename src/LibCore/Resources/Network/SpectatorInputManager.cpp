//
// Created by PinkySmile on 25/12/23
//

#include <zlib.h>
#include "SpectatorInputManager.hpp"
#include "Resources/Game.hpp"
#include "Utils.hpp"

namespace SpiralOfFate
{
	bool SpectatorInputManager::parseReplayPacket(PacketReplay &packet)
	{
		std::vector<unsigned char> out;
		int res = Utils::Z::decompress(packet.compressedData, packet.compressedSize, out);

		if (res != Z_OK) {
			game->logger.error("Decompression failed with result '" + Utils::Z::error(res) + "'");
			return false;
		}

		auto buffer = reinterpret_cast<PacketInput *>(out.data());

		for (unsigned i = 0; i < packet.nbInputs; i++) {
			if (packet.frameId + i > this->_lastReceivedFrame)
				break;
			if (packet.frameId + i != this->_lastReceivedFrame)
				continue;
			this->_leftList.push_back(buffer[i]);
			this->_rightList.push_back(buffer[i + packet.nbInputs]);
			this->_lastReceivedFrame++;
		}
		this->_end = packet.lastFrameId;
		return true;
	}

	std::pair<std::shared_ptr<SpectatorInput>, std::shared_ptr<SpectatorInput>> SpectatorInputManager::getInputs() const
	{
		return { this->_leftInput, this->_rightInput };
	}

	bool SpectatorInputManager::hasInputs() const
	{
		my_assert(this->_leftList.size() == this->_rightList.size());
		return !this->_leftList.empty();
	}

	unsigned SpectatorInputManager::getBufferSize() const
	{
		my_assert(this->_leftList.size() == this->_rightList.size());
		return this->_leftList.size();
	}

	unsigned SpectatorInputManager::getEnd() const
	{
		return this->_end;
	}

	unsigned SpectatorInputManager::getCurrentFrame() const
	{
		return this->_currentFrame;
	}

	unsigned SpectatorInputManager::getLastReceivedFrame() const
	{
		return this->_lastReceivedFrame;
	}

	void SpectatorInputManager::update()
	{
		while (this->_leftList.size() > this->_rightList.size())
			this->_leftList.pop_front();
		while (this->_leftList.size() < this->_rightList.size())
			this->_rightList.pop_front();
	}
}
