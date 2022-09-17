//
// Created by PinkySmile on 26/08/2022.
//

#ifndef SOFGV_NETWORKCHARACTERSELECT_HPP
#define SOFGV_NETWORKCHARACTERSELECT_HPP


#include <memory>
#include "Scenes/CharacterSelect.hpp"
#include "Resources/Network/ClientConnection.hpp"
#include "Inputs/DelayInput.hpp"
#include "Inputs/RemoteInput.hpp"
#include "Inputs/NetworkInput.hpp"

// I took examples on the bests
#define CHARACTER_SELECT_DELAY 6

namespace SpiralOfFate
{
	class NetworkCharacterSelect : public CharacterSelect {
	protected:
		std::shared_ptr<NetworkInput> _localRealInput;
		std::shared_ptr<RemoteInput> _remoteRealInput;
		DelayInput *_localInput = nullptr;
		DelayInput *_remoteInput = nullptr;

	public:
		NetworkCharacterSelect(std::shared_ptr<IInput> localInput);
		NetworkCharacterSelect(
			std::shared_ptr<IInput> localInput,
			int leftPos, int rightPos,
			int leftPalette, int rightPalette,
			int stage, int platformCfg
		);

		IScene *update() override;
	};
}


#endif //SOFGV_NETWORKCHARACTERSELECT_HPP
