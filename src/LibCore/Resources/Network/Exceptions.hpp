//
// Created by PinkySmile on 21/08/2022.
//

#ifndef SOFGV_EXCEPTIONS_HPP
#define SOFGV_EXCEPTIONS_HPP


#include <stdexcept>
#include "Packet.hpp"

namespace SpiralOfFate
{
	class NetworkException : public std::exception {
	private:
		std::string _message;

	public:
		NetworkException(const std::string &&message) : _message(message) {}
		const char *what() const noexcept override { return _message.c_str(); }
	};

	class ErrorPacketException : public NetworkException {
	private:
		PacketError _packet;

	public:
		ErrorPacketException(const PacketError &err) : NetworkException(err.toString()), _packet(err) {}
		const PacketError &getPacket() const { return _packet; }
	};

	class TimeoutExceededException : public NetworkException {
	public:
		TimeoutExceededException() : NetworkException("Timeout exceeded") {}
	};
}


#endif //SOFGV_EXCEPTIONS_HPP
