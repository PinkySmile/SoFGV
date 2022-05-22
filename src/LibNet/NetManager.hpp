//
// Created by PinkySmile on 26/04/2022.
//

#ifndef SOFGV_NETMANAGER_HPP
#define SOFGV_NETMANAGER_HPP


#include <string>
#include <thread>
#include <vector>
#include <functional>
#include <Windows.h>
#include "NetHandler.hpp"

#define MAX_ROLLBACK 8

#ifdef __GNUC__
#define NET_FCT_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define NET_FCT_NAME __FUNCSIG__
#else
#define NET_FCT_NAME __func__
#endif

#ifdef _DEBUG

#define net_assert(_Expression)                                                                \
	do {                                                                                   \
		if (_Expression);                                                              \
		else {                                                                         \
                        char ____buffer[256];                                                  \
                        sprintf(                                                               \
				____buffer,                                                    \
                                "Net Assertion %s failed in %s at line %i in %s\n",            \
                                #_Expression,                                                  \
				__FILE__,                                                      \
				__LINE__,                                                      \
				NET_FCT_NAME                                                   \
                        );                                                                     \
                        fprintf(stderr, "%s\n", ____buffer);                                   \
                        MessageBoxA(nullptr, ____buffer, "NetAssertion failed", MB_ICONERROR); \
                        abort();                                                               \
                }                                                                              \
	} while (0)

#else

#define net_assert(_Expression)                                                                \
	do {                                                                                   \
		if (_Expression);                                                              \
		else {                                                                         \
                        char ____buffer[256];                                                  \
                        sprintf(                                                               \
				____buffer,                                                    \
                                "Net Assertion %s failed in %s at line %i in %s\n",            \
                                #_Expression,                                                  \
				__FILE__,                                                      \
				__LINE__,                                                      \
				NET_FCT_NAME                                                   \
                        );                                                                     \
                        fprintf(stderr, "%s\n", ____buffer);                                   \
                        MessageBoxA(nullptr, ____buffer, "NetAssertion failed", MB_ICONERROR); \
                }                                                                              \
	} while (0)

#endif

namespace SpiralOfFateNet
{
	struct EventHandlers {
		std::function<bool (unsigned menuId, void *initFrame, size_t frameSize)> switchMenu;
		std::function<bool (void **data, size_t *size, unsigned *checksum)> saveState;
		std::function<void (void *data, size_t size)> loadState;
		std::function<void (void *data, size_t size)> freeState;
		std::function<void (const std::string &ip, unsigned port)> onConnect;
		std::function<void (const std::string &ip, unsigned port)> onDisconnect;
		std::function<void (void **inputs)> nextFrame;
		std::function<void (const std::string &)> log;
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

		~NetManager();
		static NetHandler *syncTest(Params params);
		static NetHandler *host(Params params, unsigned short port);
		static NetHandler *connect(Params params, const std::string &ip, unsigned short port);
		static NetHandler *spectate(Params params, const std::string &ip, unsigned short port);
	};
}


#endif //SOFGV_NETMANAGER_HPP
