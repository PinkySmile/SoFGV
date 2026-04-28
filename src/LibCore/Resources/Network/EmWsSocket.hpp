//
// Created by PinkySmile on 15/03/2026.
//

#ifndef SOFGV_EMWSSOCKET_HPP
#define SOFGV_EMWSSOCKET_HPP


#include <optional>
#include <emscripten/websocket.h>
#include <SFML/Network.hpp>

namespace SpiralOfFate
{
	class EmWsSocket {
	private:
		std::optional<EMSCRIPTEN_WEBSOCKET_T> _websocket;
		unsigned short _localPort = 0;
		bool _hadError = false;
		bool _isClosed = false;
		std::atomic<bool> _needPort = false;
		std::atomic<bool> _waitingForOpen = false;
		std::mutex _queueMutex;
		std::deque<std::tuple<sf::IpAddress, unsigned short, std::vector<unsigned char>>> _messageQueue;

		static bool onMessageCallback(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent __attribute__((nonnull)), void *userData);
		static bool onOpenCallback(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent __attribute__((nonnull)), void *userData);
		static bool onCloseCallback(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent __attribute__((nonnull)), void *userData);
		static bool onErrorCallback(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent __attribute__((nonnull)), void *userData);

	public:
		~EmWsSocket();
		void setBlocking(bool blocking);
		void unbind();
		unsigned short getLocalPort() const;
		sf::Socket::Status bind(unsigned short port, sf::IpAddress address = sf::IpAddress::Any);
		sf::Socket::Status send(const void *data, std::size_t size, sf::IpAddress remoteAddress, unsigned short remotePort);
		sf::Socket::Status receive(void *data, std::size_t size, std::size_t &received, std::optional<sf::IpAddress> &remoteAddress, unsigned short &remotePort);
	};
}


#endif //SOFGV_EMWSSOCKET_HPP