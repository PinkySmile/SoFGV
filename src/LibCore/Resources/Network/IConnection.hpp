//
// Created by PinkySmile on 22/08/2022.
//

#ifndef SOFGV_ICONNECTION_HPP
#define SOFGV_ICONNECTION_HPP


#include <thread>
#include <list>
#include <SFML/Network.hpp>
#include "Packet.hpp"
#include "Inputs/InputEnum.hpp"

namespace SpiralOfFate
{
	class IConnection {
	public:
		virtual ~IConnection() = default;
		virtual std::list<PacketInput> receive() = 0;
	};
}


#endif //SOFGV_ICONNECTION_HPP
