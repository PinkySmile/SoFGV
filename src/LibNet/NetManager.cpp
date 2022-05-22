//
// Created by PinkySmile on 26/04/2022.
//

#include <memory>
#include "NetManager.hpp"
#include "SyncTestHandler.hpp"

namespace SpiralOfFateNet
{
	NetHandler *NetManager::syncTest(Params params)
	{
		return new SyncTestHandler(params);
	}

	NetHandler *NetManager::host(Params params, unsigned short port)
	{
		return nullptr;
	}

	NetHandler *NetManager::connect(Params params, const std::string &ip, unsigned short port)
	{
		return nullptr;
	}

	NetHandler *NetManager::spectate(Params params, const std::string &ip, unsigned short port)
	{
		return nullptr;
	}
}