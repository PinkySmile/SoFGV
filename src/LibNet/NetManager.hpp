//
// Created by PinkySmile on 26/04/2022.
//

#ifndef SOFGV_NETMANAGER_HPP
#define SOFGV_NETMANAGER_HPP


#include <string>
#include <thread>
#include <vector>
#include "NetHandler.hpp"

#define MAX_ROLLBACK 8

namespace SpiralOfFateNet
{
	struct EventHandlers {
		void (*switchMenu)(unsigned menuId);
		bool (*saveState)(void **data, size_t *size, unsigned *checksum);
		void (*loadState)(void *data, size_t size);
		void (*freeState)(void *data, size_t size);
		void (*onConnect)(const std::string &ip, unsigned port);
		void (*onDisconnect)(const std::string &ip, unsigned port);
		void (*nextFrame)(void *inputs);
	};

	struct NetStats {
		unsigned lastPing = 0;
		unsigned peakPing = 0;
		unsigned averPing = 0;
		unsigned aPktLoss = 0;
		unsigned netDelay = 0;
		unsigned avgRollback = 0;
	};

	class NetManager {
	public:
		struct Params {
			unsigned inputSize;
			unsigned playerCount;
			EventHandlers handlers;
		};

	private:
		void *buffer;
		unsigned _inputSize;
		unsigned _playerCount;
		EventHandlers _evntHandlers;
		std::unique_ptr<NetHandler> _handler;

		NetManager(unsigned inputSize, unsigned playerCount, EventHandlers handlers);
	public:
		~NetManager();
		static NetManager *syncTest(Params params);
		static NetManager *host(Params params, unsigned short port);
		static NetManager *connect(Params params, const std::string &ip, unsigned short port);
		static NetManager *spectate(Params params, const std::string &ip, unsigned short port);
		void addInputs(void *data, unsigned playerID);
		void switchMenu(unsigned menuId, void *initFrame, size_t frameSize);
		NetStats getNetStats();
	};
}


#endif //SOFGV_NETMANAGER_HPP
