//
// Created by Gegel85 on 30/04/2022.
//

#ifndef SOFGV_NETHANDLER_HPP
#define SOFGV_NETHANDLER_HPP


namespace SpiralOfFateNet
{
	class NetHandler {
	private:
		class NetManager *_manager;

	public:
		virtual void addInputs(void *data) = 0;
		virtual void switchMenu(unsigned menuId) = 0;
		virtual NetStats getNetStats() = 0;
	};
}


#endif //SOFGV_NETHANDLER_HPP
