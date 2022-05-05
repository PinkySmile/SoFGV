//
// Created by Gegel85 on 26/04/2022.
//

#ifndef SOFGV_NETMANAGER_HPP
#define SOFGV_NETMANAGER_HPP


#include <string>
#include <thread>
#include <vector>
#include "NetHandler.hpp"

namespace SpiralOfFateNet
{
	struct EventHandlers {
		void (*switchMenu)(unsigned menuId);
		bool (*saveState)(void **data, size_t *size, unsigned *checksum);
		void (*loadState)(void *data, size_t size);
		void (*freeState)(void *data, size_t size);
		void (*onConnect)(const std::string &ip, unsigned port);
		void (*onDisconnect)(const std::string &ip, unsigned port);
		void (*nextFrame)();
	};

	struct NetStat {
		unsigned lastPing = 0;
		unsigned peakPing = 0;
		unsigned averPing = 0;
		unsigned aPktLoss = 0;
		unsigned netDelay = 0;
	};

	struct NetStats {
		std::vector<NetStat> stats;
		unsigned avgRollback = 0;
	};

	class NetManager {
	private:
		unsigned _inputSize;
		unsigned _playerSize;
		EventHandlers _evntHandlers;
		std::unique_ptr<NetHandler> _handler;

	public:
		NetManager(unsigned inputSize, unsigned playerSize, EventHandlers handlers);
		~NetManager();
		void syncTest();
		void host(unsigned short port);
		void connect(const std::string &ip, unsigned short port);
		void spectate(const std::string &ip, unsigned short port);
		void addInputs(void *data);
		void switchMenu(unsigned menuId);
		NetStats getNetStats();
	};
}


#endif //SOFGV_NETMANAGER_HPP
