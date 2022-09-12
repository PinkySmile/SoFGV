//
// Created by PinkySmile on 26/08/2022.
//

#include "ClientCharacterSelect.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	ClientCharacterSelect::ClientCharacterSelect(std::shared_ptr<IInput> localInput) :
		CharacterSelect(nullptr, nullptr, nullptr),
		_localInput(std::move(localInput))
	{
		this->_leftInput = this->_leftRollbackInput;
		this->_rightInput = this->_rightRollbackInput;
	}

	ClientCharacterSelect::ClientCharacterSelect(
		std::shared_ptr<IInput> localInput,
		int leftPos, int rightPos,
		int leftPalette, int rightPalette,
		int stage, int platformCfg
	) :
		CharacterSelect(nullptr, nullptr, leftPos, rightPos, leftPalette, rightPalette, stage, platformCfg, nullptr),
		_localInput(std::move(localInput))
	{
		this->_leftInput = this->_leftRollbackInput;
		this->_rightInput = this->_rightRollbackInput;
	}

	IScene *ClientCharacterSelect::update()
	{
		IScene *result = nullptr;

		if (!this->_remoteDelayBuffer.empty()) {
			this->_leftRollbackInput->_keyStates[INPUT_LEFT] = this->_remoteDelayBuffer.front().horizontalAxis < 0;
			this->_leftRollbackInput->_keyStates[INPUT_RIGHT] = this->_remoteDelayBuffer.front().horizontalAxis > 0;
			this->_leftRollbackInput->_keyStates[INPUT_UP] = this->_remoteDelayBuffer.front().verticalAxis > 0;
			this->_leftRollbackInput->_keyStates[INPUT_DOWN] = this->_remoteDelayBuffer.front().verticalAxis < 0;
			this->_leftRollbackInput->_keyStates[INPUT_N] = this->_remoteDelayBuffer.front().n != 0;
			this->_leftRollbackInput->_keyStates[INPUT_M] = this->_remoteDelayBuffer.front().m != 0;
			this->_leftRollbackInput->_keyStates[INPUT_S] = this->_remoteDelayBuffer.front().s != 0;
			this->_leftRollbackInput->_keyStates[INPUT_V] = this->_remoteDelayBuffer.front().v != 0;
			this->_leftRollbackInput->_keyStates[INPUT_A] = this->_remoteDelayBuffer.front().a != 0;
			this->_leftRollbackInput->_keyStates[INPUT_D] = this->_remoteDelayBuffer.front().d != 0;

			this->_rightRollbackInput->_keyStates[INPUT_LEFT] = this->_localDelayBuffer.front().horizontalAxis < 0;
			this->_rightRollbackInput->_keyStates[INPUT_RIGHT] = this->_localDelayBuffer.front().horizontalAxis > 0;
			this->_rightRollbackInput->_keyStates[INPUT_UP] = this->_localDelayBuffer.front().verticalAxis > 0;
			this->_rightRollbackInput->_keyStates[INPUT_DOWN] = this->_localDelayBuffer.front().verticalAxis < 0;
			this->_rightRollbackInput->_keyStates[INPUT_N] = this->_localDelayBuffer.front().n != 0;
			this->_rightRollbackInput->_keyStates[INPUT_M] = this->_localDelayBuffer.front().m != 0;
			this->_rightRollbackInput->_keyStates[INPUT_S] = this->_localDelayBuffer.front().s != 0;
			this->_rightRollbackInput->_keyStates[INPUT_V] = this->_localDelayBuffer.front().v != 0;
			this->_rightRollbackInput->_keyStates[INPUT_A] = this->_localDelayBuffer.front().a != 0;
			this->_rightRollbackInput->_keyStates[INPUT_D] = this->_localDelayBuffer.front().d != 0;

			this->_remoteDelayBuffer.pop_front();
			this->_localDelayBuffer.pop_front();
			result = CharacterSelect::update();
		}

		while (this->_localDelayBuffer.size() != CHARACTER_SELECT_DELAY) {
			this->_localInput->update();
			this->_localDelayBuffer.push_back(this->_localInput->getInputs());
			game->connection->send(this->_localDelayBuffer.back());
		}
		while (this->_remoteDelayBuffer.size() != CHARACTER_SELECT_DELAY) {
			for (auto &input : game->connection->receive())
				this->_inputBuffer.push_back(input);
			if (this->_inputBuffer.empty())
				break;
			this->_remoteDelayBuffer.push_back({
				this->_inputBuffer.front()._h,
				this->_inputBuffer.front()._v,
				this->_inputBuffer.front().n,
				this->_inputBuffer.front().m,
				this->_inputBuffer.front().s,
				this->_inputBuffer.front().v,
				this->_inputBuffer.front().a,
				this->_inputBuffer.front().d,
				false
			});
			this->_inputBuffer.pop_front();
		}
		return result;
	}

	void ClientCharacterSelect::consumeEvent(const sf::Event &event)
	{
		this->_localInput->consumeEvent(event);
		CharacterSelect::consumeEvent(event);
	}

	InGame *ClientCharacterSelect::_launchGame()
	{
		game->soundMgr.play(BASICSOUND_GAME_LAUNCH);
		return nullptr;
	}
}