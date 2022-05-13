//
// Created by PinkySmile on 11/05/2022.
//

#ifndef SOFGV_SYNCTESTHANDLER_HPP
#define SOFGV_SYNCTESTHANDLER_HPP


#include "NetHandler.hpp"

namespace SpiralOfFateNet
{
	class SyncTestHandler : public NetHandler {
	public:
		SyncTestHandler(class NetManager *manager);
		~SyncTestHandler() override = default;
		void addInputs(void *data) override;
		void switchMenu(unsigned int menuId) override;
		struct NetStats getNetStats() override;
		void update() override;
	};
}


#endif //SOFGV_SYNCTESTHANDLER_HPP
