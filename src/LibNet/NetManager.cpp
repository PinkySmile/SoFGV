//
// Created by Gegel85 on 26/04/2022.
//

#include "NetManager.hpp"

namespace SpiralOfFateNet
{
	NetManager::NetManager(unsigned inputSize, unsigned playerSize, EventHandlers handlers) :
		_inputSize(inputSize),
		_playerSize(playerSize),
		_evntHandlers(handlers)
	{
	}

	NetManager::~NetManager()
	{

	}

	void NetManager::syncTest()
	{

	}

	void NetManager::host(unsigned short port)
	{

	}

	void NetManager::connect(const std::string &ip, unsigned short port)
	{

	}

	void NetManager::spectate(const std::string &ip, unsigned short port)
	{

	}

	void NetManager::addInputs(void *data)
	{

	}

	void NetManager::switchMenu(unsigned int menuId)
	{

	}

	NetStats NetManager::getNetStats()
	{
		return {};
	}
}