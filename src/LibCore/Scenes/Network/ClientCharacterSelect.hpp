//
// Created by PinkySmile on 26/08/2022.
//

#ifndef SOFGV_CLIENTCHARACTERSELECT_HPP
#define SOFGV_CLIENTCHARACTERSELECT_HPP


#include <memory>
#include "Scenes/CharacterSelect.hpp"
#include "Resources/Network/ClientConnection.hpp"
#include "Inputs/RollbackInput.hpp"

// I took examples on the bests
#define CHARACTER_SELECT_DELAY 6

namespace SpiralOfFate
{
	class ClientCharacterSelect : public CharacterSelect {
	private:
		std::shared_ptr<RollbackInput> _leftInput = std::make_shared<RollbackInput>();
		std::shared_ptr<RollbackInput> _rightInput = std::make_shared<RollbackInput>();
		std::shared_ptr<IInput> _localInput;
		std::list<PacketInput> _inputBuffer;
		unsigned _currentFrameToSend = 0;
		unsigned _frameToBeReceived = 0;

		std::list<InputStruct> _localDelayBuffer{CHARACTER_SELECT_DELAY, {0, 0, 0, 0, 0, 0, 0, 0, 0}};
		std::list<InputStruct> _remoteDelayBuffer{CHARACTER_SELECT_DELAY, {0, 0, 0, 0, 0, 0, 0, 0, 0}};

	protected:
		InGame *_launchGame() override;

	public:
		ClientCharacterSelect(std::shared_ptr<IInput> localInput);
		ClientCharacterSelect(
			std::shared_ptr<IInput> localInput,
			int leftPos, int rightPos,
			int leftPalette, int rightPalette,
			int stage, int platformCfg
		);

		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //SOFGV_CLIENTCHARACTERSELECT_HPP
