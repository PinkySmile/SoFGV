//
// Created by PinkySmile on 19/12/23.
//

#ifndef SOFGV_REPLAYDATA_HPP
#define SOFGV_REPLAYDATA_HPP


namespace SpiralOfFate {
	struct ReplayData {
		bool n : 1;
		bool m : 1;
		bool v : 1;
		bool s : 1;
		bool a : 1;
		bool d : 1;
		char _h : 2;
		char _v : 2;
		unsigned char time : 6;
	};
	static_assert(sizeof(ReplayData) == 2);
}


#endif //SOFGV_REPLAYDATA_HPP
