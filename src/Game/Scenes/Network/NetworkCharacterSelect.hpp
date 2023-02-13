//
// Created by PinkySmile on 26/08/2022.
//

#ifndef SOFGV_NETWORKCHARACTERSELECT_HPP
#define SOFGV_NETWORKCHARACTERSELECT_HPP


#include <memory>
#include "../CharacterSelect.hpp"
#include "Resources/Network/ClientConnection.hpp"
#include "Inputs/DelayInput.hpp"
#include "Inputs/RemoteInput.hpp"
#include "Inputs/NetworkInput.hpp"

// I took examples on the bests
#define CHARACTER_SELECT_DELAY 0

namespace SpiralOfFate
{
	class NetworkCharacterSelect : public CharacterSelect {
	protected:
		std::shared_ptr<NetworkInput> _localRealInput;
		std::shared_ptr<RemoteInput> _remoteRealInput;
		std::shared_ptr<DelayInput> _localInput;
		std::shared_ptr<DelayInput> _remoteInput;

	public:
		NetworkCharacterSelect();
		NetworkCharacterSelect(
			int leftPos, int rightPos,
			int leftPalette, int rightPalette,
			int stage, int platformCfg
		);

		void flushInputs(unsigned delay);
		std::shared_ptr<RemoteInput> getRemoteRealInput();
		void update() override;
		InGame::InitParams createParams(SceneArguments *args, Connection::GameStartParams params);
		void render() const override;
	};
}


#endif //SOFGV_NETWORKCHARACTERSELECT_HPP
