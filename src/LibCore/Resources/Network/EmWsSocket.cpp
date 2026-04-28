//
// Created by PinkySmile on 15/03/2026.
//

#include <thread>
#include <emscripten/val.h>
#include "EmWsSocket.hpp"

#include <arpa/inet.h>

#include "Resources/Assert.hpp"

#define RECV_BUFFER_SIZE (1 * 1024 * 1024) // 1MB

namespace SpiralOfFate
{
	bool EmWsSocket::onMessageCallback(int, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData)
	{
		auto This = static_cast<EmWsSocket *>(userData);

		if (This->_needPort) {
			if (websocketEvent->numBytes < 2) {
				This->_isClosed = true;
				This->_hadError = true;
				emscripten_websocket_close(websocketEvent->socket, 1002, "");
			} else
				This->_localPort = websocketEvent->data[0] << 8 | websocketEvent->data[1];
			This->_needPort = false;
		} else if (websocketEvent->numBytes < 6) {
			This->_isClosed = true;
			This->_hadError = true;
			emscripten_websocket_close(websocketEvent->socket, 1002, "");
		} else {
			sf::IpAddress addr{
				websocketEvent->data[0],
				websocketEvent->data[1],
				websocketEvent->data[2],
				websocketEvent->data[3]
			};
			unsigned short port = websocketEvent->data[4] << 8 | websocketEvent->data[5];
			std::vector<unsigned char> data{&websocketEvent->data[6], &websocketEvent->data[websocketEvent->numBytes]};

			This->_queueMutex.lock();
			This->_messageQueue.emplace_back(addr, port, data);
			This->_queueMutex.unlock();
		}
		return true;
	}

	bool EmWsSocket::onOpenCallback(int, const EmscriptenWebSocketOpenEvent *, void *userData)
	{
		auto This = static_cast<EmWsSocket *>(userData);

		This->_waitingForOpen = false;
		return true;
	}

	bool EmWsSocket::onCloseCallback(int, const EmscriptenWebSocketCloseEvent *, void *userData)
	{
		auto This = static_cast<EmWsSocket *>(userData);

		This->_waitingForOpen = false;
		This->_isClosed = true;
		return true;
	}

	bool EmWsSocket::onErrorCallback(int, const EmscriptenWebSocketErrorEvent *, void *userData)
	{
		auto This = static_cast<EmWsSocket *>(userData);

		This->_waitingForOpen = false;
		This->_isClosed = true;
		This->_hadError = true;
		return true;
	}

	EmWsSocket::~EmWsSocket()
	{
		if (this->_websocket)
			emscripten_websocket_delete(*this->_websocket);
	}

	void EmWsSocket::setBlocking(bool)
	{
	}

	void EmWsSocket::unbind()
	{
		if (!this->_websocket)
			return;
		emscripten_websocket_close(*this->_websocket, 1000, "");
	}

	unsigned short EmWsSocket::getLocalPort() const
	{
		return this->_localPort;
	}

	sf::Socket::Status EmWsSocket::bind(unsigned short, sf::IpAddress)
	{
		assert_exp(!this->_websocket);

		EmscriptenWebSocketCreateAttributes attributes;
		emscripten::val location = emscripten::val::global("window")["location"];
		std::string protocol = location["protocol"].as<std::string>();
		std::string host = location["host"].as<std::string>();
		std::string url;
		size_t size = protocol.size() + host.size() + strlen(WEBSOCKET_PATH) + strlen("host");

		url.reserve(size);
		if (protocol == "https:")
			url += "wss://";
		else
			url += "ws://";
		url += host;
		url += WEBSOCKET_PATH;
		url += "host";
		assert_exp(size == url.size());

		emscripten_websocket_init_create_attributes(&attributes);
		attributes.url = url.c_str();
		attributes.protocols = "binary";
		attributes.createOnMainThread = false;

		auto ret = emscripten_websocket_new(&attributes);

		if (ret < 0)
			return sf::Socket::Status::Error;
		this->_needPort = true;
		this->_waitingForOpen = true;
		this->_websocket = ret;
		emscripten_websocket_set_onmessage_callback(*this->_websocket, this, &EmWsSocket::onMessageCallback);
		emscripten_websocket_set_onopen_callback(*this->_websocket, this, &EmWsSocket::onOpenCallback);
		emscripten_websocket_set_onclose_callback(*this->_websocket, this, &EmWsSocket::onCloseCallback);
		emscripten_websocket_set_onerror_callback(*this->_websocket, this, &EmWsSocket::onErrorCallback);

		while ((this->_waitingForOpen || this->_needPort) && !this->_isClosed)
			emscripten_sleep(100);
		if (this->_hadError)
			return sf::Socket::Status::Error;
		if (this->_isClosed)
			return sf::Socket::Status::Disconnected;
		return sf::Socket::Status::Done;
	}

	sf::Socket::Status EmWsSocket::send(const void *data, std::size_t size, sf::IpAddress remoteAddress, unsigned short remotePort)
	{
		if (this->_isClosed)
			return sf::Socket::Status::Disconnected;
		if (!this->_websocket) {
			if (remoteAddress.toInteger() != 0)
				return sf::Socket::Status::Error;

			EmscriptenWebSocketCreateAttributes attributes;
			emscripten::val location = emscripten::val::global("window")["location"];
			std::string protocol = location["protocol"].as<std::string>();
			std::string host = location["host"].as<std::string>();
			std::string port = std::to_string(remotePort);
			std::string url;
			size_t requested_capacity = protocol.size() + host.size() + strlen(WEBSOCKET_PATH) + strlen("join/") + port.size();

			url.reserve(requested_capacity);
			if (protocol == "https:")
				url += "wss://";
			else
				url += "ws://";
			url += host;
			url += WEBSOCKET_PATH;
			url += "join/";
			url += port;
			assert_exp(requested_capacity == url.size());

			emscripten_websocket_init_create_attributes(&attributes);
			attributes.url = url.c_str();
			attributes.protocols = "binary";
			attributes.createOnMainThread = false;

			auto ret = emscripten_websocket_new(&attributes);

			if (ret < 0)
				return sf::Socket::Status::Error;
			this->_needPort = false;
			this->_waitingForOpen = true;
			this->_websocket = ret;
			emscripten_websocket_set_onmessage_callback(*this->_websocket, this, &EmWsSocket::onMessageCallback);
			emscripten_websocket_set_onopen_callback(*this->_websocket, this, &EmWsSocket::onOpenCallback);
			emscripten_websocket_set_onclose_callback(*this->_websocket, this, &EmWsSocket::onCloseCallback);
			emscripten_websocket_set_onerror_callback(*this->_websocket, this, &EmWsSocket::onErrorCallback);

			while (this->_waitingForOpen && !this->_isClosed)
				emscripten_sleep(100);
			if (this->_hadError)
				return sf::Socket::Status::Error;
			if (this->_isClosed)
				return sf::Socket::Status::Disconnected;
		}

		char buffer[RECV_BUFFER_SIZE + 6];
		unsigned ipInt = htonl(remoteAddress.toInteger());
		unsigned short port = htons(remotePort);

		assert_exp(size < RECV_BUFFER_SIZE);
		memcpy(buffer, &ipInt, 4);
		memcpy(&buffer[4], &port, 2);
		memcpy(&buffer[6], data, size);
		emscripten_websocket_send_binary(*this->_websocket, buffer, size + 6);
		return sf::Socket::Status::Done;
	}

	sf::Socket::Status EmWsSocket::receive(void *data, std::size_t size, std::size_t &received, std::optional<sf::IpAddress> &remoteAddress, unsigned short &remotePort)
	{
		this->_queueMutex.lock();
		if (this->_messageQueue.empty()) {
			this->_queueMutex.unlock();
			return sf::Socket::Status::NotReady;
		}

		remoteAddress = std::get<0>(this->_messageQueue.front());
		remotePort = std::get<1>(this->_messageQueue.front());
		std::vector<unsigned char> d;

		d.swap(std::get<2>(this->_messageQueue.front()));
		this->_messageQueue.pop_front();
		this->_queueMutex.unlock();
		if (d.size() > size) {
			memcpy(data, d.data(), size);
			received = size;
			return sf::Socket::Status::Partial;
		}
		received = d.size();
		memcpy(data, d.data(), d.size());
		return sf::Socket::Status::Done;
	}
}
