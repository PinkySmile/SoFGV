//
// Created by PinkySmile on 01/03/2022.
//

#include "Stickman.hpp"
#include "Resources/Game.hpp"
#include "Resources/Battle/PracticeBattleManager.hpp"

#define INSTALL_COST 200
#define INSTALL_DURATION 30

namespace SpiralOfFate
{
	Stickman::Stickman(
		unsigned index,
		const std::string &folder,
		const std::pair<std::vector<SpiralOfFate::Color>,
		std::vector<SpiralOfFate::Color>> &palette,
		std::shared_ptr<IInput> input
	) :
		Character(index, folder, palette, std::move(input))
	{
		game->logger.debug("Stickman class created");
	}

	unsigned int Stickman::getClassId() const
	{
		return 3;
	}

	void Stickman::_onMoveEnd(const FrameData &lastData)
	{
		if (this->_action == ACTION_WIN_MATCH2 && this->_actionBlock == 1) {
			this->_actionBlock++;
			assert(this->_moves.at(this->_action).size() != this->_actionBlock);
			return Character::_onMoveEnd(lastData);
		}
		Character::_onMoveEnd(lastData);
	}

	void Stickman::update()
	{
		Character::update();
		this->_buffTimer -= !!this->_buffTimer;
		if (
			this->_inputBuffer.a &&
			this->_inputBuffer.verticalAxis < 0 &&
			this->_action >= ACTION_5N &&
			!isOverdriveAction(this->_action) &&
			!this->_voidInstallTimer &&
			!this->_spiritInstallTimer &&
			!this->_matterInstallTimer &&
			!this->_buffTimer
		) {
			this->_voidInstallTimer = 0;
			this->_spiritInstallTimer = 0;
			this->_matterInstallTimer = 0;
			this->_buffTimer = INSTALL_DURATION;
			this->_mana -= INSTALL_COST;
			this->_specialInputs._am = -30;
			this->_specialInputs._as = -30;
			this->_specialInputs._av = -30;
			this->_inputBuffer.a = 0;
			game->soundMgr.play(BASICSOUND_INSTALL_START);
		}
	}

	unsigned int Stickman::getBufferSize() const
	{
		return Character::getBufferSize() + sizeof(Data);
	}

	void Stickman::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());

		Character::copyToBuffer(data);
		game->logger.verbose("Saving Stickman (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
		dat->_time = this->_time;
		dat->_oldAction = this->_oldAction;
		dat->_buffTimer = this->_buffTimer;
	}

	void Stickman::restoreFromBuffer(void *data)
	{
		Character::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());

		this->_time = dat->_time;
		this->_oldAction = dat->_oldAction;
		this->_buffTimer = dat->_buffTimer;
		game->logger.verbose("Restored Stickman @" + std::to_string((uintptr_t)dat));
	}

	void Stickman::onMatchEnd()
	{
		Character::onMatchEnd();
		this->_oldAction = this->_action;
		if (this->_oldAction <= ACTION_WIN_MATCH2)
			game->soundMgr.play(BASICSOUND_DASH);
	}

	bool Stickman::matchEndUpdate()
	{
		if (this->_oldAction < ACTION_WIN_MATCH1)
			return false;
		if (this->_action < ACTION_WIN_MATCH1)
			this->_forceStartMove(this->_oldAction);
		if (!this->_isGrounded())
			this->_speed.x = 0;
		if (std::abs(this->_position.x - this->_opponent->getPosition().x) < 30 && this->_actionBlock == 0) {
			this->_actionBlock++;
			this->_animation = 0;
			this->_animationCtr = 0;
			this->_speed.x = 0;
		}
		this->_time += this->_actionBlock;
		return this->_time < 30;
	}

	size_t Stickman::printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const
	{
		auto length = Character::printDifference(msgStart, data1, data2, startOffset);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		game->logger.info("Stickman @" + std::to_string(startOffset + length));
		if (dat1->_buffTimer != dat2->_buffTimer)
			game->logger.fatal(std::string(msgStart) + "Stickman::_buffTimer: " + std::to_string(dat1->_buffTimer) + " vs " + std::to_string(dat2->_buffTimer));
		return length + sizeof(Data);
	}

	void Stickman::_renderExtraEffects(const Vector2f &pos) const
	{
		Character::_renderExtraEffects(pos);
		if (this->_buffTimer)
			this->_renderInstallEffect(this->_neutralEffect);
	}

	void Stickman::_computeFrameDataCache()
	{
		Character::_computeFrameDataCache();
		if (!this->_buffTimer)
			return;
		this->_fdCache.oFlag.voidElement = true;
		this->_fdCache.oFlag.matterElement = true;
		this->_fdCache.oFlag.spiritElement = true;
	}

	size_t Stickman::printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const
	{
		auto length = Character::printContent(msgStart, data, startOffset, dataSize);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data + length);

		game->logger.info("Stickman @" + std::to_string(startOffset + length));
		if (startOffset + length + sizeof(Data) >= dataSize)
			game->logger.warn("Object is " + std::to_string(startOffset + length + sizeof(Data) - dataSize) + " bytes bigger than input");
		game->logger.info(std::string(msgStart) + "Stickman::_buffTimer: " + std::to_string(dat1->_buffTimer));
		game->logger.info(std::string(msgStart) + "Stickman::_time: " + std::to_string(dat1->_time));
		game->logger.info(std::string(msgStart) + "Stickman::_oldAction: " + std::to_string(dat1->_oldAction));
		if (startOffset + length + sizeof(Data) >= dataSize) {
			game->logger.fatal("Invalid input frame");
			return 0;
		}
		return length + sizeof(Data);
	}
}
