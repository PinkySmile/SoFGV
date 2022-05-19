//
// Created by PinkySmile on 11/05/2022.
//

#include "SyncTestHandler.hpp"
#include "NetManager.hpp"

namespace SpiralOfFateNet
{
	SyncTestHandler::SyncTestHandler(NetManager *manager) :
		NetHandler(manager)
	{

	}

	void SyncTestHandler::addInputs(void *data, unsigned playerId)
	{

	}

	void SyncTestHandler::switchMenu(unsigned int menuId, void *initFrame, size_t frameSize)
	{
		this->_manager->getParams().handlers.switchMenu(menuId, initFrame, frameSize);
	}

	NetStats SyncTestHandler::getNetStats()
	{
		return {};
	}

	void SyncTestHandler::update()
	{

	}
}