//
// Created by PinkySmile on 30/04/2022.
//

#ifndef SOFGV_NETHANDLER_HPP
#define SOFGV_NETHANDLER_HPP


#include <cstddef>

namespace SpiralOfFateNet
{
	class NetHandler {
	public:
		virtual ~NetHandler() = default;
		virtual void addInputs(void *data, unsigned playerId) = 0;
		virtual void switchMenu(unsigned menuId, void *initFrame, size_t frameSize) = 0;
		virtual struct NetStats getNetStats() = 0;
		virtual void setDelay(unsigned int delay) = 0;
		virtual void update() = 0;
	};
}


#endif //SOFGV_NETHANDLER_HPP
