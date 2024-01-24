//
// Created by PinkySmile on 01/03/2022.
//

#include "Stickman.hpp"
#include "Resources/Game.hpp"
#include "Resources/Battle/PracticeBattleManager.hpp"
#include "Objects/Characters/CharacterParams.hpp"

#define SPECIAL_INSTALL_COST 300

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
		if (this->_hasSpiritInstall || this->_hasMatterInstall || this->_hasVoidInstall)
			this->_hasBuff = false;
		if (
			this->_specialInputs._da > 0 &&
			!isOverdriveAction(this->_action) &&
			!this->_installMoveStarted &&
			!this->_hasBuff
		) {
			for (auto &obj : this->_typeSwitchEffects)
				if (obj.second)
					obj.second->kill();
			this->_spawnSystemParticles(SYS_PARTICLE_GENERATOR_NEUTRAL_TYPE_SWITCH);
			this->_hasVoidInstall = false;
			this->_hasSpiritInstall = false;
			this->_hasMatterInstall = false;
			this->_hasBuff = true;
			this->_mana -= SPECIAL_INSTALL_COST;
			this->_specialInputs._dm = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._ds = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._dv = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._da = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_clearBasicBuffer();
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
		dat->_hasBuff = this->_hasBuff;
	}

	void Stickman::restoreFromBuffer(void *data)
	{
		Character::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());

		this->_time = dat->_time;
		this->_oldAction = dat->_oldAction;
		this->_hasBuff = dat->_hasBuff;
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
		if (dat1->_time != dat2->_time)
			game->logger.fatal(std::string(msgStart) + "Stickman::_time: " + std::to_string(dat1->_time) + " vs " + std::to_string(dat2->_time));
		if (dat1->_oldAction != dat2->_oldAction)
			game->logger.fatal(std::string(msgStart) + "Stickman::_oldAction: " + std::to_string(dat1->_oldAction) + " vs " + std::to_string(dat2->_oldAction));
		if (dat1->_hasBuff != dat2->_hasBuff)
			game->logger.fatal(std::string(msgStart) + "Stickman::_hasBuff: " + std::to_string(dat1->_hasBuff) + " vs " + std::to_string(dat2->_hasBuff));
		return length + sizeof(Data);
	}

	void Stickman::_renderExtraEffects(const Vector2f &pos) const
	{
		Character::_renderExtraEffects(pos);
		if (this->_hasBuff && this->_installMoveStarted)
			this->_renderInstallEffect(this->_neutralEffect);
	}

	void Stickman::_computeFrameDataCache()
	{
		Character::_computeFrameDataCache();
		if (!this->_hasBuff)
			return;
		this->_fdCache.oFlag.voidElement = true;
		this->_fdCache.oFlag.matterElement = true;
		this->_fdCache.oFlag.spiritElement = true;
		this->_fdCache.neutralLimit += this->_fdCache.voidLimit;
		this->_fdCache.neutralLimit += this->_fdCache.matterLimit;
		this->_fdCache.neutralLimit += this->_fdCache.spiritLimit;
		this->_fdCache.voidLimit = 0;
		this->_fdCache.matterLimit = 0;
		this->_fdCache.spiritLimit = 0;
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
		game->logger.info(std::string(msgStart) + "Stickman::_hasBuff: " + std::to_string(dat1->_hasBuff));
		game->logger.info(std::string(msgStart) + "Stickman::_time: " + std::to_string(dat1->_time));
		game->logger.info(std::string(msgStart) + "Stickman::_oldAction: " + std::to_string(dat1->_oldAction));
		if (startOffset + length + sizeof(Data) >= dataSize) {
			game->logger.fatal("Invalid input frame");
			return 0;
		}
		return length + sizeof(Data);
	}

	void Stickman::_forceStartMove(unsigned int action)
	{
		if (this->_installMoveStarted)
			this->_hasBuff = false;
		Character::_forceStartMove(action);
		if (action >= ACTION_5N) {
			this->_clearBasicBuffer();
			if (this->_hasBuff)
				this->_installMoveStarted = true;
		}
	}
}
