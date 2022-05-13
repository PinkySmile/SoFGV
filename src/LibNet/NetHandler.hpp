//
// Created by PinkySmile on 30/04/2022.
//

#ifndef SOFGV_NETHANDLER_HPP
#define SOFGV_NETHANDLER_HPP


namespace SpiralOfFateNet
{
	class NetHandler {
	protected:
		class NetManager *_manager;

		NetHandler(class NetManager *manager) : _manager(manager) {}
	public:
		virtual ~NetHandler() = default;
		virtual void addInputs(void *data) = 0;
		virtual void switchMenu(unsigned menuId) = 0;
		virtual struct NetStats getNetStats() = 0;
		virtual void update() = 0;
	};
}


#endif //SOFGV_NETHANDLER_HPP
