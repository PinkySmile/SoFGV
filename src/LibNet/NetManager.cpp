//
// Created by PinkySmile on 26/04/2022.
//

#include <memory>
#include "NetManager.hpp"
#include "SyncTestHandler.hpp"

namespace SpiralOfFateNet
{
	NetManager::NetManager(unsigned inputSize, unsigned playerCount, EventHandlers handlers) :
		_inputSize(inputSize),
		_playerCount(playerCount),
		_evntHandlers(handlers)
	{
	}

	NetManager::~NetManager()
	{
	}

	NetManager *NetManager::syncTest(Params params)
	{
		auto result = new NetManager(params.inputSize, params.playerCount, params.handlers);

		result->_handler = std::make_unique<SyncTestHandler>(result);
		return result;
	}

	NetManager *NetManager::host(Params params, unsigned short port)
	{
		return nullptr;
	}

	NetManager *NetManager::connect(Params params, const std::string &ip, unsigned short port)
	{
		return nullptr;
	}

	NetManager *NetManager::spectate(Params params, const std::string &ip, unsigned short port)
	{
		return nullptr;
	}

	void NetManager::addInputs(void *data, unsigned playerID)
	{
		net_assert(this->_handler);
		this->_handler->addInputs(data, playerID);
	}

	void NetManager::switchMenu(unsigned int menuId, void *initFrame, size_t frameSize)
	{
		net_assert(this->_handler);
	}

	NetStats NetManager::getNetStats()
	{
		net_assert(this->_handler);
		return this->_handler->getNetStats();
	}

	NetManager::Params NetManager::getParams()
	{
		return {
			this->_inputSize,
			this->_playerCount,
			this->_evntHandlers
		};
	}
}