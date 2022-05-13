//
// Created by PinkySmile on 26/04/2022.
//

#ifndef SOFGV_EXCEPTION_HPP
#define SOFGV_EXCEPTION_HPP


#include <string>

namespace SpiralOfFateNet
{
	class NetException {
	private:
		std::string _msg;

	public:
		NetException(const std::string &&msg) : _msg(msg) {};
		const char *what() const noexcept { return this->_msg.c_str(); };
	};

	class ConnectException : public NetException {
	public:
		ConnectException(const std::string &ip, unsigned short port) : NetException("Cannot connect to " + ip + " on port " + std::to_string(port)) {};
	};

	class BindException : public NetException {
	public:
		BindException(const std::string &&msg) : NetException("Cannot bind port " + std::to_string(port)) {};
	};
}


#endif //SOFGV_EXCEPTION_HPP
