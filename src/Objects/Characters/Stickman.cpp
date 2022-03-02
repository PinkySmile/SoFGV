//
// Created by Gegel85 on 01/03/2022.
//

#include "Stickman.hpp"
#include "../../Resources/Game.hpp"

#define MIN_RANDOM_DFLAGS 3
#define MIN_RANDOM_OFLAGS 3
#define NEW_FLAG_STEP 5
#define MAX_CHARGE 40

namespace Battle
{
	Stickman::Stickman(
		const std::string &frameData,
		const std::string &subobjFrameData,
		const std::pair<std::vector<Battle::Color>,
		std::vector<Battle::Color>> &palette,
		std::shared_ptr<IInput> input
	) :
		Character(frameData, subobjFrameData, palette, input)
	{
		game.logger.debug("Stickman class created");
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
		auto data = Object::getCurrentFrameData();

		if (!this->_flagsGenerated)
			return data;
		this->_fakeFrameData = *data;
		this->_fakeFrameData.dFlag.flags |= this->_addedDFlags.flags;
		if (this->_action >= ACTION_5A && this->_action <= ACTION_c64A)
			this->_fakeFrameData.oFlag.flags |= this->_addedOFlags.flags;
		else
			// We don't want non grabbing moves to keep the grab property
			this->_fakeFrameData.oFlag.flags |= this->_addedOFlags.flags & ~1;
		return &this->_fakeFrameData;
	}

	bool Stickman::_canStartMove(unsigned int action, const FrameData &data)
	{
		if (this->_flagsGenerated && !this->_moveLength)
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
		Object::_applyNewAnimFlags();
		if (this->_action >= ACTION_5A && this->_action <= ACTION_c64A) {
			auto data = this->getCurrentFrameData();
			int flag;

			if (data->specialMarker && !this->_flagsGenerated) {
				for (int i = 0; i < MIN_RANDOM_DFLAGS; i++) {
					for (flag = this->_dist(game.random); this->_addedDFlags.flags & 1 << flag || flag == 8; flag = this->_dist(game.random));
					this->_addedDFlags.flags |= 1 << flag;
				}
				for (int i = 0; i < MIN_RANDOM_OFLAGS; i++) {
					for (flag = this->_dist(game.random); this->_addedOFlags.flags & 1 << flag; flag = this->_dist(game.random));
					this->_addedOFlags.flags |= 1 << flag;
				}
				this->_flagsGenerated = true;
			}
			if (data->specialMarker > 1) {
				this->_moveLength = data->specialMarker;
				this->_actionBlock++;
				this->_animation = 0;
				this->_chargeTime = 0;
				this->_applyNewAnimFlags();
				return;
			}
		}
	}

	void Stickman::_applyMoveAttributes()
	{
		Character::_applyMoveAttributes();
		if (this->_action >= ACTION_5A && this->_action <= ACTION_c64A) {
			auto data = this->getCurrentFrameData();
			int flag;

			if (data->specialMarker && !this->_flagsGenerated) {
				for (int i = 0; i < MIN_RANDOM_DFLAGS; i++) {
					for (flag = this->_dist(game.random); this->_addedDFlags.flags & 1 << flag || flag == 8; flag = this->_dist(game.random));
					this->_addedDFlags.flags |= 1 << flag;
				}
				for (int i = 0; i < MIN_RANDOM_OFLAGS; i++) {
					for (flag = this->_dist(game.random); this->_addedOFlags.flags & 1 << flag; flag = this->_dist(game.random));
					this->_addedOFlags.flags |= 1 << flag;
				}
				this->_flagsGenerated = true;
			}
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
						for (flag = this->_dist(game.random); this->_addedDFlags.flags & 1 << flag || flag == 8; flag = this->_dist(game.random));
						this->_addedDFlags.flags |= 1 << flag;
						for (flag = this->_dist(game.random); this->_addedOFlags.flags & 1 << flag; flag = this->_dist(game.random));
						this->_addedOFlags.flags |= 1 << flag;
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
}
