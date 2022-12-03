//
// Created by PinkySmile on 26/08/2022.
//

#ifndef SOFGV_CLIENTCHARACTERSELECT_HPP
#define SOFGV_CLIENTCHARACTERSELECT_HPP


#include <memory>
#include "NetworkCharacterSelect.hpp"

// I took examples on the bests

namespace SpiralOfFate
{
	class ClientCharacterSelect : public NetworkCharacterSelect {
	protected:
		LoadingScene *_launchGame() override;

	public:
		ClientCharacterSelect();
		ClientCharacterSelect(
			int leftPos, int rightPos,
			int leftPalette, int rightPalette,
			int stage, int platformCfg
		);

		friend class ClientConnection;
	};
}


#endif //SOFGV_CLIENTCHARACTERSELECT_HPP
