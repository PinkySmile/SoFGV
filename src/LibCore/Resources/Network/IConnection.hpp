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
	protected:
		std::vector<class RemoteInput *> _registeredInputs;

	public:
		virtual ~IConnection() = default;
		virtual std::list<PacketInput> receive() = 0;

		void registerInput(class RemoteInput *input)
		{
			this->_registeredInputs.push_back(input);
		}

		void unregisterInput(class RemoteInput *input)
		{
			this->_registeredInputs.erase(
				std::remove(this->_registeredInputs.begin(), this->_registeredInputs.end(), input),
				this->_registeredInputs.end()
			);
		};
	};
}


#endif //SOFGV_ICONNECTION_HPP
