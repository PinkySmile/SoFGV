//
// Created by PinkySmile on 11/05/2022.
//

#ifndef SOFGV_SYNCTESTHANDLER_HPP
#define SOFGV_SYNCTESTHANDLER_HPP


#include <list>
#include <optional>
#include "NetHandler.hpp"
#include "NetManager.hpp"

namespace SpiralOfFateNet
{
	class SyncTestHandler : public NetHandler {
	private:
		struct RollbackBuffer {
			unsigned currentFrame = 0;
			unsigned checksum = 0;
			size_t bufferSize = 0;
			void *buffer = nullptr;
			void *leftInputs = nullptr;
			void *rightInputs = nullptr;
		};

		unsigned _currentFrame = 0;
		RollbackBuffer _rollbackBuffer;
		NetManager::Params _params;
		std::list<std::pair<void *, void *>> _inputs;

		RollbackBuffer _saveState();

	public:
		SyncTestHandler(NetManager::Params params);
		~SyncTestHandler() override;
		void addInputs(void *data, unsigned playerId) override;
		void switchMenu(unsigned int menuId, void *initFrame, size_t frameSize) override;
		struct NetStats getNetStats() override;
		void update() override;
		void setDelay(unsigned int delay) override;
	};
}


#endif //SOFGV_SYNCTESTHANDLER_HPP
