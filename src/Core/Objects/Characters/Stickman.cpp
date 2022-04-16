//
// Created by Gegel85 on 01/03/2022.
//

#include "Stickman.hpp"
#include "../../Resources/Game.hpp"

#define MIN_RANDOM_FLAGS 6
#define NEW_FLAG_STEP 5
#define MAX_CHARGE 40
#define HITSTUN_RATIO 2/3
#define BLOCKSTUN_RATIO 2/3

namespace SpiralOfFate
{
	const std::map<unsigned, std::vector<unsigned>> _probas{
		{ACTION_5A, {
			8, //"grab",
			8, //"airUnblockable",
			0, //"unblockable",
			8, //"voidElement",
			8, //"spiritElement",
			8, //"matterElement",
			8, //"lowHit",
			8, //"highHit",
			0, //"autoHitPos",
			5, //"canCounterHit",
			0, //"hitSwitch",
			0, //"cancelable",
			0, //"jab",
			0, //"resetHits",
			5, //"resetOPSpeed",
			5, //"restand",
			0, //"super",
			0, //"ultimate",
			5, //"jumpCancelable",
			0, //"transformCancelable",
			0, //"unTransformCancelable",
			5, //"dashCancelable",
			0, //"backDashCancelable",
			0, //"voidMana",
			0, //"spiritMana",
			0, //"matterMana",
			0, //"invulnerable",
			0, //"invulnerableArmor",
			0, //"superarmor",
			5, //"grabInvulnerable",
			3, //"voidBlock",
			3, //"spiritBlock",
			3, //"matterBlock",
			0, //"neutralBlock",
			0, //"airborne",
			0, //"canBlock",
			0, //"highBlock",
			0, //"lowBlock",
			0, //"karaCancel",
			0, //"resetRotation",
			0, //"counterHit",
			0, //"flash",
			0, //"crouch",
			5, //"projectileInvul",
			0, //"projectile",
			0, //"landCancel",
			0, //"dashCancel",
			0, //"resetSpeed",
			0, //"neutralInvul",
			0, //"matterInvul",
			0, //"spiritInvul",
			0, //"voidInvul"
		}},
		{ACTION_2A, {
			0, //"grab",
			0, //"airUnblockable",
			0, //"unblockable",
			3, //"voidElement",
			3, //"spiritElement",
			3, //"matterElement",
			0, //"lowHit",
			0, //"highHit",
			0, //"autoHitPos",
			0, //"canCounterHit",
			0, //"hitSwitch",
			0, //"cancelable",
			5, //"jab",
			0, //"resetHits",
			0, //"resetOPSpeed",
			0, //"restand",
			0, //"super",
			0, //"ultimate",
			5, //"jumpCancelable",
			0, //"transformCancelable",
			0, //"unTransformCancelable",
			0, //"dashCancelable",
			5, //"backDashCancelable",
			0, //"voidMana",
			0, //"spiritMana",
			0, //"matterMana",
			0, //"invulnerable",
			0, //"invulnerableArmor",
			0, //"superarmor",
			8, //"grabInvulnerable",
			8, //"voidBlock",
			8, //"spiritBlock",
			8, //"matterBlock",
			0, //"neutralBlock",
			0, //"airborne",
			0, //"canBlock",
			8, //"highBlock",
			8, //"lowBlock",
			0, //"karaCancel",
			0, //"resetRotation",
			0, //"counterHit",
			0, //"flash",
			0, //"crouch",
			8, //"projectileInvul",
			0, //"projectile",
			0, //"landCancel",
			0, //"dashCancel",
			5, //"resetSpeed",
			0, //"neutralInvul",
			5, //"matterInvul",
			5, //"spiritInvul",
			5, //"voidInvul"
		}},
		{ACTION_6A, {
			0, //"grab",
			60,//"airUnblockable",
			15,//"unblockable",
			30,//"voidElement",
			30,//"spiritElement",
			30,//"matterElement",
			50,//"lowHit",
			50,//"highHit",
			0, //"autoHitPos",
			0, //"canCounterHit",
			0, //"hitSwitch",
			0, //"cancelable",
			0, //"jab",
			0, //"resetHits",
			20,//"resetOPSpeed",
			0, //"restand",
			5, //"super",
			2, //"ultimate",
			0, //"jumpCancelable",
			0, //"transformCancelable",
			0, //"unTransformCancelable",
			50,//"dashCancelable",
			0, //"backDashCancelable",
			0, //"voidMana",
			0, //"spiritMana",
			0, //"matterMana",
			0, //"invulnerable",
			0, //"invulnerableArmor",
			60,//"superarmor",
			0, //"grabInvulnerable",
			0, //"voidBlock",
			0, //"spiritBlock",
			0, //"matterBlock",
			0, //"neutralBlock",
			0, //"airborne",
			0, //"canBlock",
			0, //"highBlock",
			0, //"lowBlock",
			0, //"karaCancel",
			0, //"resetRotation",
			0, //"counterHit",
			0, //"flash",
			0, //"crouch",
			0, //"projectileInvul",
			0, //"projectile",
			0, //"landCancel",
			0, //"dashCancel",
			0, //"resetSpeed",
			0, //"neutralInvul",
			0, //"matterInvul",
			0, //"spiritInvul",
			0, //"voidInvul"
		}},
		{ACTION_3A, {
			0, //"grab",
			0, //"airUnblockable",
			0, //"unblockable",
			0, //"voidElement",
			0, //"spiritElement",
			0, //"matterElement",
			0, //"lowHit",
			0, //"highHit",
			0, //"autoHitPos",
			0, //"canCounterHit",
			0, //"hitSwitch",
			0, //"cancelable",
			0, //"jab",
			0, //"resetHits",
			0, //"resetOPSpeed",
			0, //"restand",
			7, //"super",
			4, //"ultimate",
			0, //"jumpCancelable",
			0, //"transformCancelable",
			0, //"unTransformCancelable",
			0, //"dashCancelable",
			40,//"backDashCancelable",
			0, //"voidMana",
			0, //"spiritMana",
			0, //"matterMana",
			10,//"invulnerable",
			10,//"invulnerableArmor",
			0, //"superarmor",
			60,//"grabInvulnerable",
			0, //"voidBlock",
			0, //"spiritBlock",
			0, //"matterBlock",
			0, //"neutralBlock",
			0, //"airborne",
			20,//"canBlock",
			0, //"highBlock",
			0, //"lowBlock",
			0, //"karaCancel",
			0, //"resetRotation",
			0, //"counterHit",
			0, //"flash",
			0, //"crouch",
			60,//"projectileInvul",
			0, //"projectile",
			0, //"landCancel",
			0, //"dashCancel",
			0, //"resetSpeed",
			0, //"neutralInvul",
			50,//"matterInvul",
			50,//"spiritInvul",
			50,//"voidInvul"
		}},
	};

	Stickman::Stickman(
		unsigned index,
		const std::string &frameData,
		const std::string &subobjFrameData,
		const std::pair<std::vector<SpiralOfFate::Color>,
		std::vector<SpiralOfFate::Color>> &palette,
		std::shared_ptr<IInput> input
	) :
		Character(index, frameData, subobjFrameData, palette, input)
	{
		this->_fakeFrameData.setSlave();
		game->logger.debug("Stickman class created");
	}

	unsigned int Stickman::getClassId() const
	{
		return 3;
	}

	void Stickman::_tickMove()
	{
		if (this->_action < ACTION_5A || this->_action > ACTION_c64A || this->_actionBlock == 2)
			this->_decreaseMoveTime();
		Object::_tickMove();
	}

	const FrameData *Stickman::getCurrentFrameData() const
	{
		auto data = Character::getCurrentFrameData();

		if (!this->_flagsGenerated)
			return data;
		this->_fakeFrameData = *data;
		this->_fakeFrameData.dFlag.flags |= this->_addedDFlags.flags;
		if (this->_action >= ACTION_5A && this->_action <= ACTION_c64A && this->_actionBlock < 3) {
			this->_fakeFrameData.hitStun   += this->_chargeTime * HITSTUN_RATIO;
			this->_fakeFrameData.blockStun += this->_chargeTime * BLOCKSTUN_RATIO;
			this->_fakeFrameData.oFlag.flags |= this->_addedOFlags.flags;
		} else
			// We don't want non grabbing moves to keep the grab property, same for ultimates.
			this->_fakeFrameData.oFlag.flags |= this->_addedOFlags.flags & ~(1 << 0 | 1 << 17);
		return &this->_fakeFrameData;
	}

	void Stickman::_forceStartMove(unsigned int action)
	{
		if (action >= ACTION_5A && action <= ACTION_c64A) {
			this->_addedDFlags.flags = 0;
			this->_addedOFlags.flags = 0;
			this->_flagsGenerated = false;
		}
		Character::_forceStartMove(action);
	}

	bool Stickman::_canStartMove(unsigned int action, const FrameData &data)
	{
		if (this->_flagsGenerated && !this->_moveLength && this->_action >= ACTION_5A && this->_action <= ACTION_c64A)
			return false;
		return Character::_canStartMove(action, data);
	}

	void Stickman::_decreaseMoveTime()
	{
		if (this->_moveLength) {
			this->_moveLength--;
			if (!this->_moveLength) {
				this->_addedDFlags.flags = 0;
				this->_addedOFlags.flags = 0;
				this->_flagsGenerated = false;
			}
		}
	}

	void Stickman::_applyNewAnimFlags()
	{
		Character::_applyNewAnimFlags();
		if (this->_action == ACTION_5A || this->_action == ACTION_2A)
			this->_checkStartSystemARandom();
		else if (this->_action == ACTION_6A || this->_action == ACTION_3A)
			this->_checkStartSystemBRandom();
		if (this->_action >= ACTION_5A && this->_action <= ACTION_c64A && this->getCurrentFrameData()->specialMarker > 1) {
			this->_moveLength = this->getCurrentFrameData()->specialMarker;
			this->_actionBlock++;
			this->_animation = 0;
			this->_chargeTime = 0;
			this->_applyNewAnimFlags();
			return;
		}
	}

	void Stickman::_applyMoveAttributes()
	{
		Character::_applyMoveAttributes();
		if (this->_action >= ACTION_5A && this->_action <= ACTION_c64A) {
			auto data = this->getCurrentFrameData();

			if (this->_action == ACTION_5A || this->_action == ACTION_2A)
				this->_checkStartSystemARandom();
			else if (this->_action == ACTION_6A || this->_action == ACTION_3A)
				this->_checkStartSystemBRandom();
			switch (this->_actionBlock) {
			case 0:
				if (data->specialMarker > 1) {
					this->_moveLength = data->specialMarker;
					this->_actionBlock++;
					this->_animation = 0;
					this->_chargeTime = 0;
					this->_applyNewAnimFlags();
					return;
				}
				break;
			case 1:
				if (this->_input->isPressed(INPUT_ASCEND) && this->_chargeTime < MAX_CHARGE) {
					this->_chargeTime++;
					if (this->_chargeTime % NEW_FLAG_STEP == 0) {
						if (this->_action == ACTION_5A || this->_action == ACTION_2A)
							this->_systemARandomNewRound();
						else if (this->_action == ACTION_6A || this->_action == ACTION_3A)
							this->_systemBRandomNewRound();
					}
				} else {
					this->_actionBlock++;
					this->_animation = 0;
					this->_applyNewAnimFlags();
				}
				break;
			case 2:
				break;
			}
		}
	}

	void Stickman::_onMoveEnd(const FrameData &lastData)
	{
		if (this->_action >= ACTION_5A && this->_action <= ACTION_c64A && this->_actionBlock == 1) {
			this->_animation = 0;
			this->_applyNewAnimFlags();
			return;
		}
		Character::_onMoveEnd(lastData);
	}

	void Stickman::update()
	{
		Character::update();
		game->battleRandom();
	}

	unsigned int Stickman::getBufferSize() const
	{
		return Character::getBufferSize() + sizeof(Data);
	}

	void Stickman::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());

		Character::copyToBuffer(data);
#ifdef _DEBUG
		game->logger.debug("Saving Stickman (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
		game->logger.debug(std::to_string(this->_addedOFlags.flags) + " " + std::to_string(this->_addedDFlags.flags) + " " + (this->_flagsGenerated ? "true" : "false") + " " + std::to_string(this->_moveLength) + " " + std::to_string(this->_chargeTime) + " ");
#endif
		dat->_addedOFlags = this->_addedOFlags.flags;
		dat->_addedDFlags = this->_addedDFlags.flags;
		dat->_flagsGenerated = this->_flagsGenerated;
		dat->_moveLength = this->_moveLength;
		dat->_chargeTime = this->_chargeTime;
	}

	void Stickman::restoreFromBuffer(void *data)
	{
		Character::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());

		this->_addedOFlags.flags = dat->_addedOFlags;
		this->_addedDFlags.flags = dat->_addedDFlags;
		this->_flagsGenerated = dat->_flagsGenerated;
		this->_moveLength = dat->_moveLength;
		this->_chargeTime = dat->_chargeTime;
#ifdef _DEBUG
		game->logger.debug("Restored Stickman @" + std::to_string((uintptr_t)dat));
		game->logger.debug(std::to_string(this->_addedOFlags.flags) + " " + std::to_string(this->_addedDFlags.flags) + " " + (this->_flagsGenerated ? "true" : "false") + " " + std::to_string(this->_moveLength) + " " + std::to_string(this->_chargeTime) + " ");
#endif
	}

	std::pair<unsigned int, std::shared_ptr<IObject>> Stickman::_spawnSubobject(unsigned int id, bool needRegister)
	{
		auto data = this->getCurrentFrameData();
		auto pos = this->_position + Vector2i{
			0,
			data->offset.y
		} + data->size / 2;

		if (id == 128)
			return game->battleMgr->registerObject<Projectile>(
				needRegister,
				this->_subObjectsData.at(id),
				this->_team,
				this->_direction,
				pos,
				this->_team,
				id,
				4
			);
		return Character::_spawnSubobject(id, needRegister);
	}

	void Stickman::_checkStartSystemARandom()
	{
		auto data = this->getCurrentFrameData();

		if (data->specialMarker && !this->_flagsGenerated) {
			for (int i = 0; i < MIN_RANDOM_FLAGS; i++)
				this->_systemARandomNewRound();
			this->_flagsGenerated = true;
		}
	}

	void Stickman::_checkStartSystemBRandom()
	{
		if (this->getCurrentFrameData()->specialMarker && !this->_flagsGenerated) {
			int i = 0;

			while (i < MIN_RANDOM_FLAGS) {
				for (size_t j = 0; j < _probas.at(this->_action).size(); j++) {
					if (!_probas.at(this->_action)[j])
						continue;
					if (this->_dist(game->battleRandom) >= _probas.at(this->_action)[j])
						continue;
					if (j < 26)
						this->_addedOFlags.flags |= 1 << j;
					else
						this->_addedDFlags.flags |= 1 << (j - 26);
					i++;
				}
			}
			this->_flagsGenerated = true;
		}
	}

	void Stickman::_systemARandomNewRound()
	{
		bool done = false;
		unsigned bestDFlags = 0;
		unsigned bestOFlags = 0;

		for (size_t i = 0; i < _probas.at(this->_action).size(); i++) {
			if (i < 26)
				bestOFlags |= (_probas.at(this->_action)[i] != 0) << i;
			else
				bestDFlags |= (_probas.at(this->_action)[i] != 0) << (i - 26);
		}

		if (this->_addedDFlags.flags == bestDFlags && this->_addedOFlags.flags == bestOFlags)
			return;

		while (!done) {
			unsigned total = 0;
			auto random = this->_dist(game->battleRandom);

			done = true;
			for (size_t j = 0; j < _probas.at(this->_action).size(); j++) {
				total += _probas.at(this->_action)[j];
				if (random < total) {
					if (j < 26) {
						if (this->_addedOFlags.flags & 1 << j) {
							done = false;
							break;
						}
						this->_addedOFlags.flags |= 1 << j;
					} else {
						if (this->_addedDFlags.flags & 1 << (j - 26)) {
							done = false;
							break;
						}
						this->_addedDFlags.flags |= 1 << (j - 26);
					}
					break;
				}
			}
		}
	}

	void Stickman::_systemBRandomNewRound()
	{
		for (size_t j = 0; j < _probas.at(this->_action).size(); j++) {
			if (!_probas.at(this->_action)[j])
				continue;
			if (this->_dist(game->battleRandom) >= _probas.at(this->_action)[j])
				continue;
			if (j < 26)
				this->_addedOFlags.flags |= 1 << j;
			else
				this->_addedDFlags.flags |= 1 << (j - 26);
		}
	}
}
