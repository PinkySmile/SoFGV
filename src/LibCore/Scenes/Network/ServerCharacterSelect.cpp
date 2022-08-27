//
// Created by PinkySmile on 26/08/2022.
//

#include "ServerCharacterSelect.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	ServerCharacterSelect::ServerCharacterSelect(std::shared_ptr<IInput> localInput) :
		CharacterSelect(nullptr, nullptr, nullptr),
		_localInput(std::move(localInput))
	{
		this->_leftInput = this->_leftRollbackInput;
		this->_rightInput = this->_rightRollbackInput;
	}

	ServerCharacterSelect::ServerCharacterSelect(
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

	IScene *ServerCharacterSelect::update()
	{
		IScene *result = nullptr;

		if (!this->_localDelayBuffer.empty()) {
			this->_rightRollbackInput->_keyStates[INPUT_LEFT] = this->_remoteDelayBuffer.front().horizontalAxis < 0;
			this->_rightRollbackInput->_keyStates[INPUT_RIGHT] = this->_remoteDelayBuffer.front().horizontalAxis > 0;
			this->_rightRollbackInput->_keyStates[INPUT_UP] = this->_remoteDelayBuffer.front().verticalAxis > 0;
			this->_rightRollbackInput->_keyStates[INPUT_DOWN] = this->_remoteDelayBuffer.front().verticalAxis < 0;
			this->_rightRollbackInput->_keyStates[INPUT_N] = this->_remoteDelayBuffer.front().n != 0;
			this->_rightRollbackInput->_keyStates[INPUT_M] = this->_remoteDelayBuffer.front().m != 0;
			this->_rightRollbackInput->_keyStates[INPUT_S] = this->_remoteDelayBuffer.front().s != 0;
			this->_rightRollbackInput->_keyStates[INPUT_V] = this->_remoteDelayBuffer.front().v != 0;
			this->_rightRollbackInput->_keyStates[INPUT_A] = this->_remoteDelayBuffer.front().a != 0;
			this->_rightRollbackInput->_keyStates[INPUT_D] = this->_remoteDelayBuffer.front().d != 0;

			this->_leftRollbackInput->_keyStates[INPUT_LEFT] = this->_localDelayBuffer.front().horizontalAxis < 0;
			this->_leftRollbackInput->_keyStates[INPUT_RIGHT] = this->_localDelayBuffer.front().horizontalAxis > 0;
			this->_leftRollbackInput->_keyStates[INPUT_UP] = this->_localDelayBuffer.front().verticalAxis > 0;
			this->_leftRollbackInput->_keyStates[INPUT_DOWN] = this->_localDelayBuffer.front().verticalAxis < 0;
			this->_leftRollbackInput->_keyStates[INPUT_N] = this->_localDelayBuffer.front().n != 0;
			this->_leftRollbackInput->_keyStates[INPUT_M] = this->_localDelayBuffer.front().m != 0;
			this->_leftRollbackInput->_keyStates[INPUT_S] = this->_localDelayBuffer.front().s != 0;
			this->_leftRollbackInput->_keyStates[INPUT_V] = this->_localDelayBuffer.front().v != 0;
			this->_leftRollbackInput->_keyStates[INPUT_A] = this->_localDelayBuffer.front().a != 0;
			this->_leftRollbackInput->_keyStates[INPUT_D] = this->_localDelayBuffer.front().d != 0;

			this->_remoteDelayBuffer.pop_front();
			this->_localDelayBuffer.pop_front();
			result = CharacterSelect::update();
		}

		if (this->_inputBuffer.empty())
			this->_inputBuffer = game->connection->receive();

		if (this->_localDelayBuffer.size() != CHARACTER_SELECT_DELAY) {
			if (!this->_inputBuffer.empty()) {
				this->_localInput->update();
				this->_localDelayBuffer.push_back(this->_localInput->getInputs());
				game->connection->send(this->_localDelayBuffer.back());
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
		}
		return result;
	}

	void ServerCharacterSelect::consumeEvent(const sf::Event &event)
	{
		this->_localInput->consumeEvent(event);
		CharacterSelect::consumeEvent(event);
	}

	InGame *ServerCharacterSelect::_launchGame()
	{
		std::uniform_int_distribution<size_t> dist{0, this->_entries.size() - 1};
		std::uniform_int_distribution<size_t> dist2{0, this->_stages.size() - 1};

		if (this->_stage == -1) {
			this->_platform = -1;
			this->_stage = dist2(game->battleRandom);
		}

		std::uniform_int_distribution<size_t> dist3{0, this->_stages[this->_stage].platforms.size() - 1};

		if (this->_platform == -1)
			this->_platform = dist3(game->battleRandom);
		if (this->_leftPos < 0)
			this->_leftPalette = 0;
		if (this->_rightPos < 0)
			this->_rightPalette = 0;
		if (this->_leftPos < 0)
			this->_leftPos = dist(game->random);
		if (this->_rightPos < 0)
			this->_rightPos = dist(game->random);
		if (this->_leftPos == this->_rightPos && this->_entries[this->_leftPos].palettes.size() <= 1) {
			this->_leftPalette = 0;
			this->_rightPalette = 0;
		} else if (this->_leftPos == this->_rightPos && this->_entries[this->_leftPos].palettes.size() == 2 && this->_leftPalette == this->_rightPalette) {
			this->_leftPalette = 0;
			this->_rightPalette = 1;
		}
		if (this->_leftPos == this->_rightPos && this->_leftPalette == this->_rightPalette && this->_entries[this->_leftPos].palettes.size() > 1) {
			this->_rightPalette++;
			this->_rightPalette %= this->_entries[this->_leftPos].palettes.size();
		}
		game->soundMgr.play(BASICSOUND_GAME_LAUNCH);
		reinterpret_cast<ServerConnection *>(&*game->connection)->startGame(
			0 /* TODO: Generate a proper random seed */,
			this->_leftPos,
			this->_leftPalette,
			this->_rightPos,
			this->_rightPalette,
			this->_stage,
			this->_platform
		);
		return nullptr;
/*
		auto lchr = this->_createCharacter(this->_leftPos,  this->_leftPalette,  this->_leftInput);
		auto rchr = this->_createCharacter(this->_rightPos, this->_rightPalette, this->_rightInput);
		auto &lentry = this->_entries[this->_leftPos];
		auto &rentry = this->_entries[this->_rightPos];
		auto &licon = lentry.icon[this->_leftPalette];
		auto &ricon = rentry.icon[this->_rightPalette];

		return this->_sceneCreator(
			{static_cast<unsigned>(this->_stage), 0, static_cast<unsigned>(this->_platform)},
			stage.platforms[this->_platform],
			stage,
			lchr,
			rchr,
			licon.textureHandle,
			ricon.textureHandle,
			lentry.entry,
			rentry.entry
		);
		*/
	}
}