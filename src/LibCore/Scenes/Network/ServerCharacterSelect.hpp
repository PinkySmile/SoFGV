//
// Created by PinkySmile on 26/08/2022.
//

#ifndef SOFGV_SERVERCHARACTERSELECT_HPP
#define SOFGV_SERVERCHARACTERSELECT_HPP


#include <memory>
#include "NetworkCharacterSelect.hpp"

// I took examples on the bests
#define CHARACTER_SELECT_DELAY 6

namespace SpiralOfFate
{
	class ServerCharacterSelect : public NetworkCharacterSelect {
	protected:
		InGame *_launchGame() override;

	public:
		ServerCharacterSelect(std::shared_ptr<IInput> localInput);
		ServerCharacterSelect(
			std::shared_ptr<IInput> localInput,
			int leftPos, int rightPos,
			int leftPalette, int rightPalette,
			int stage, int platformCfg
		);
	};
}


#endif //SOFGV_SERVERCHARACTERSELECT_HPP
