//
// Created by PinkySmile on 18/09/2021
//

#ifdef _WIN32
#include <windows.h>
#endif
#include "ACharacter.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"

#define QUARTER_CIRCLE_BUFFER 10
#define DP_BUFFER 15
#define HALF_CIRCLE_BUFFER 20
#define SPIRAL_BUFFER 30
#define NORMAL_BUFFER 4

namespace Battle
{
	ACharacter::ACharacter(const std::string &frameData, std::shared_ptr<IInput> input) :
		_input(std::move(input))
	{
		this->_moves = FrameData::loadFile(frameData);
		this->_lastInputs.push_back({0, 0, 0});
	}

	void ACharacter::render() const
	{
		AObject::render();
	}

	void ACharacter::update()
	{
		if (this->_action < ACTION_LANDING && this->_opponent) {
			if (this->_opponent->_position.x - this->_position.x != 0)
				this->_dir = std::copysign(1, this->_opponent->_position.x - this->_position.x);
			this->_direction = this->_dir == 1;
		}

		this->_input->update();
		AObject::update();
		if (
			this->_isGrounded() &&
			this->_action != ACTION_NEUTRAL_JUMP &&
			this->_action != ACTION_FORWARD_JUMP &&
			this->_action != ACTION_BACKWARD_JUMP &&
			this->_action != ACTION_NEUTRAL_HIGH_JUMP &&
			this->_action != ACTION_FORWARD_HIGH_JUMP &&
			this->_action != ACTION_BACKWARD_HIGH_JUMP
		)
			this->_jumpsUsed = 0;
		if (this->_action >= ACTION_AIR_DASH_1 && this->_action <= ACTION_AIR_DASH_9 && this->_isGrounded())
			return this->_forceStartMove(ACTION_HARD_LAND);
		if (this->_action == ACTION_AIR_HIT && this->_isGrounded()) {
			this->_blockStun = 0;
			this->_forceStartMove(ACTION_BEING_KNOCKED_DOWN);
			return;
		}
		if (this->_blockStun) {
			this->_blockStun--;
			if (this->_blockStun == 0) {
				if (this->_isGrounded())
					this->_forceStartMove(ACTION_IDLE);
				else if (this->_action != ACTION_AIR_HIT || this->_restand)
					this->_forceStartMove(ACTION_FALLING);
			}
		} else
			this->_processInput(this->_input->getInputs());

		if (this->_position.x < 0)
			this->_position.x = 0;
		if (this->_position.y < 0)
			this->_position.y = 0;
		if (this->_position.x > 1000)
			this->_position.x = 1000;
		if (this->_position.y > 1000)
			this->_position.y = 1000;
	}

	void ACharacter::init(bool side, unsigned short maxHp, unsigned char maxJumps, Vector2f gravity)
	{
		this->_dir = side ? 1 : -1;
		this->_direction = side;
		this->_team = side;
		this->_baseHp = this->_hp = maxHp;
		this->_maxJumps = maxJumps;
		this->_baseGravity = this->_gravity = gravity;
		if (side) {
			this->_position = {200, 0};
		} else {
			this->_position = {800, 0};
		}
	}

	void ACharacter::consumeEvent(const sf::Event &event)
	{
		this->_input->consumeEvent(event);
	}

	void ACharacter::_processInput(const InputStruct &input)
	{
		auto data = this->getCurrentFrameData();

		if (
			std::copysign(!!input.horizontalAxis, this->_dir * input.horizontalAxis) != this->_lastInputs.front().h ||
			std::copysign(!!input.verticalAxis,   this->_dir * input.verticalAxis)   != this->_lastInputs.front().v
		)
			this->_lastInputs.push_front({
				0,
				static_cast<int>(std::copysign(!!input.horizontalAxis, this->_dir * input.horizontalAxis)),
				static_cast<int>(std::copysign(!!input.verticalAxis,   this->_dir * input.verticalAxis))
			});
		this->_lastInputs.front().nbFrames++;
		if (this->_lastInputs.front().nbFrames > 45)
			this->_lastInputs.front().nbFrames = 45;
		this->_checkSpecialInputs();
		if (!this->_isGrounded() && (
			(input.d && input.verticalAxis > 0 && this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_AIR_DASH_9)) ||
			(input.d && input.verticalAxis > 0 && this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_AIR_DASH_7)) ||
			(input.d && input.verticalAxis > 0 &&                                          this->_startMove(ACTION_AIR_DASH_8)) ||
			(input.d && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_AIR_DASH_3)) ||
			(input.d && input.verticalAxis < 0 && this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_AIR_DASH_1)) ||
			(input.d && input.verticalAxis < 0 &&                                          this->_startMove(ACTION_AIR_DASH_2)) ||
			(input.d &&                           this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_AIR_DASH_6)) ||
			(input.d &&                           this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_AIR_DASH_4))
		))
			return;
		if (this->_isGrounded() && (
			(input.d && this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_FORWARD_DASH)) ||
			(input.d && this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_BACKWARD_DASH))
		))
			return;
		if (
			(data->dFlag.airborne && this->_executeAirborneMoves(input)) ||
			(!data->dFlag.airborne && this->_executeGroundMoves(input))
		)
			return;
		if (input.verticalAxis > 0) {
			if (!this->_hasJumped || this->_isGrounded()) {
				if (input.horizontalAxis) {
					if (std::copysign(1, input.horizontalAxis) == std::copysign(1, this->_dir)) {
						if (this->_startMove(ACTION_FORWARD_JUMP))
							return;
					} else if (this->_startMove(ACTION_BACKWARD_JUMP))
						return;
				} else if (this->_startMove(ACTION_NEUTRAL_JUMP))
					return;
			}
		} else if (input.verticalAxis < 0 && this->_isGrounded()) {
			this->_startMove(ACTION_CROUCHING);
			if (this->_action == ACTION_CROUCHING || this->_action == ACTION_CROUCH)
				return;
			this->_hasJumped = false;
		} else
			this->_hasJumped = false;
		if (this->_isGrounded()) {
			if (input.horizontalAxis) {
				if (std::copysign(1, input.horizontalAxis) == std::copysign(1, this->_dir)) {
					this->_startMove(ACTION_WALK_FORWARD);
					return;
				}
				this->_startMove(ACTION_WALK_BACKWARD);
				return;
			} else {
				this->_startMove(this->_action == ACTION_CROUCH ? ACTION_STANDING_UP : ACTION_IDLE);
				return;
			}
		} else {
			this->_startMove(ACTION_FALLING);
			return;
		}
	}

	bool ACharacter::_executeAirborneMoves(const InputStruct &input)
	{
		return  //(input.n && input.n <= 4 && this->_startMove(ACTION_j5N));
		        (input.d &&                           this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_FORWARD_AIR_TECH)) ||
		        (input.d &&                           this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_BACKWARD_AIR_TECH)) ||
		        (input.d && input.verticalAxis > 0 &&                                          this->_startMove(ACTION_UP_AIR_TECH)) ||
		        (input.d && input.verticalAxis < 0 &&                                          this->_startMove(ACTION_DOWN_AIR_TECH)) ||

			(input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._624684  || this->_specialInputs._6314684)  && this->_startMove(ACTION_j6321469874N)) ||
			(input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._6246974 || this->_specialInputs._63146974) && this->_startMove(ACTION_j6321469874N)) ||
			(input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._624 || this->_specialInputs._6314) &&         this->_startMove(ACTION_j63214N)) ||
			(input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._426 || this->_specialInputs._4136) &&         this->_startMove(ACTION_j41236N)) ||
			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._623 &&                                         this->_startMove(ACTION_j623N)) ||
			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._421 &&                                         this->_startMove(ACTION_j421N)) ||
			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._236 &&                                         this->_startMove(ACTION_j236N)) ||
			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._214 &&                                         this->_startMove(ACTION_j214N)) ||
		        (input.n && input.n <= NORMAL_BUFFER && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8N)) ||
		        (input.n && input.n <= NORMAL_BUFFER && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3N)) ||
		        (input.n && input.n <= NORMAL_BUFFER && this->_dir * input.horizontalAxis > 0 &&                             this->_startMove(ACTION_j6N)) ||
		        (input.n && input.n <= NORMAL_BUFFER && input.verticalAxis < 0 &&                                            this->_startMove(ACTION_j2N)) ||
		        (input.n && input.n <= NORMAL_BUFFER &&                                                                      this->_startMove(ACTION_j5N)) ||

		        (input.v && input.v <= NORMAL_BUFFER && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8V)) ||
		        (input.v && input.v <= NORMAL_BUFFER && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3V)) ||
		        (input.v && input.v <= NORMAL_BUFFER && this->_dir * input.horizontalAxis > 0 &&                             this->_startMove(ACTION_j6V)) ||
		        (input.v && input.v <= NORMAL_BUFFER && input.verticalAxis < 0 &&                                            this->_startMove(ACTION_j2V)) ||
		        (input.v && input.v <= NORMAL_BUFFER &&                                                                      this->_startMove(ACTION_j5V));
	}

	bool ACharacter::_executeGroundMoves(const InputStruct &input)
	{
		return  //(input.n && input.n <= 4 && this->_startMove(ACTION_5N)) ||
		        (input.d && input.verticalAxis > 0 && this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_FORWARD_HIGH_JUMP)) ||
		        (input.d && input.verticalAxis > 0 && this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_BACKWARD_HIGH_JUMP)) ||
		        (input.d && input.verticalAxis > 0 &&                                          this->_startMove(ACTION_NEUTRAL_HIGH_JUMP)) ||

			(input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._624684  || this->_specialInputs._6314684)  && this->_startMove(ACTION_6321469874N)) ||
			(input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._6246974 || this->_specialInputs._63146974) && this->_startMove(ACTION_6321469874N)) ||
			(input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._624 || this->_specialInputs._6314) &&         this->_startMove(ACTION_63214N)) ||
			(input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._426 || this->_specialInputs._4136) &&         this->_startMove(ACTION_41236N)) ||
			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._623 &&                                         this->_startMove(ACTION_623N)) ||
			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._421 &&                                         this->_startMove(ACTION_421N)) ||
			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._236 &&                                         this->_startMove(ACTION_236N)) ||
			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._214 &&                                         this->_startMove(ACTION_214N)) ||
		        (input.n && input.n <= NORMAL_BUFFER && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8N)) ||
		        (input.n && input.n <= NORMAL_BUFFER && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3N)) ||
		        (input.n && input.n <= NORMAL_BUFFER &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6N)) ||
		        (input.n && input.n <= NORMAL_BUFFER && input.verticalAxis < 0 &&                                            this->_startMove(ACTION_2N)) ||
		        (input.n && input.n <= NORMAL_BUFFER &&                                                                      this->_startMove(ACTION_5N)) ||

		        (input.v && input.v <= NORMAL_BUFFER && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8V)) ||
		        (input.v && input.v <= NORMAL_BUFFER && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3V)) ||
		        (input.v && input.v <= NORMAL_BUFFER &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6V)) ||
		        (input.v && input.v <= NORMAL_BUFFER && input.verticalAxis < 0 &&                                            this->_startMove(ACTION_2V)) ||
		        (input.v && input.v <= NORMAL_BUFFER &&                                                                      this->_startMove(ACTION_5V));
	}

	bool ACharacter::_canStartMove(unsigned action, const FrameData &data)
	{
		if (action == ACTION_IDLE && this->_action == ACTION_STANDING_UP)
			return false;
		if (action == ACTION_CROUCHING && this->_action == ACTION_CROUCH)
			return false;
		if (this->_canCancel(action))
			return true;
		if (action >= ACTION_AIR_DASH_1 && action <= ACTION_AIR_DASH_9)
			return this->_action > ACTION_BACKWARD_HIGH_JUMP || (this->_action < ACTION_NEUTRAL_HIGH_JUMP && (this->_action < ACTION_NEUTRAL_JUMP || !this->getCurrentFrameData()->dFlag.airborne));
		if (action >= ACTION_NEUTRAL_JUMP && action <= ACTION_BACKWARD_HIGH_JUMP)
			return this->_jumpsUsed < this->_maxJumps && (this->_action <= ACTION_WALK_BACKWARD || this->_action == ACTION_FALLING || this->_action == ACTION_LANDING);
		if (this->_action == action)
			return false;
		if (action <= ACTION_WALK_BACKWARD || action == ACTION_FALLING || action == ACTION_LANDING)
			return (this->_action <= ACTION_WALK_BACKWARD || this->_action == ACTION_FALLING || this->_action == ACTION_LANDING);
		if (this->_action == ACTION_BACKWARD_DASH)
			return false;
		if (this->_action == ACTION_FORWARD_DASH)
			return false;
		if (this->_action <= ACTION_LANDING)
			return true;
		return false;
	}

	void ACharacter::_onMoveEnd(FrameData &lastData)
	{
		logger.debug(std::to_string(this->_action) + " ended");
		if (this->_action == ACTION_BEING_KNOCKED_DOWN) {
			this->_blockStun = 0;
			return this->_forceStartMove(ACTION_KNOCKED_DOWN);
		}
		if (this->_blockStun && !this->_actionBlock) {
			this->_actionBlock++;
			if (this->_moves.at(this->_action).size() == 1)
				//TODO: make proper exceptions
				throw std::invalid_argument("Action " + std::to_string(this->_action) + " is missing block 1");
			AObject::_onMoveEnd(lastData);
			return;
		}
		if (this->_action == ACTION_KNOCKED_DOWN) {
			auto inputs = this->_input->getInputs();

			if (!inputs.a && !inputs.s && !inputs.d && !inputs.m && !inputs.n && !inputs.v)
				return this->_forceStartMove(ACTION_NEUTRAL_TECH);
			if (inputs.horizontalAxis && this->_startMove(inputs.horizontalAxis * this->_dir < 0 ? ACTION_BACKWARD_TECH : ACTION_FORWARD_TECH))
				return;
			return this->_forceStartMove(ACTION_NEUTRAL_TECH);
		}
		if (this->_action == ACTION_CROUCHING)
			return this->_forceStartMove(ACTION_CROUCH);
		if (this->_action == ACTION_BACKWARD_AIR_TECH || this->_action == ACTION_FORWARD_AIR_TECH || this->_action == ACTION_UP_AIR_TECH || this->_action == ACTION_DOWN_AIR_TECH)
			return this->_forceStartMove(this->_isGrounded() ? ACTION_IDLE : ACTION_FALLING);
		if (this->_action == ACTION_BACKWARD_TECH || this->_action == ACTION_FORWARD_TECH || this->_action == ACTION_NEUTRAL_TECH)
			return this->_forceStartMove(ACTION_IDLE);
		if (this->_action == ACTION_STANDING_UP)
			return this->_forceStartMove(ACTION_IDLE);
		if (this->_action == ACTION_FORWARD_DASH)
			return this->_forceStartMove(ACTION_IDLE);
		if (this->_action == ACTION_BACKWARD_DASH)
			return this->_forceStartMove(ACTION_IDLE);
		if (this->_action >= ACTION_AIR_DASH_1 && this->_action <= ACTION_AIR_DASH_9)
			return this->_forceStartMove(this->_isGrounded() ? ACTION_HARD_LAND : ACTION_FALLING);
		if (
			this->_action >= ACTION_5N ||
			this->_action == ACTION_LANDING
		)
			return this->_forceStartMove(this->_isGrounded() ? (lastData.dFlag.crouch ? ACTION_CROUCH : ACTION_IDLE) : ACTION_FALLING);
		if (this->_action == ACTION_NEUTRAL_JUMP || this->_action == ACTION_FORWARD_JUMP || this->_action == ACTION_BACKWARD_JUMP)
			return this->_forceStartMove(ACTION_FALLING);
		if (this->_action == ACTION_NEUTRAL_HIGH_JUMP || this->_action == ACTION_FORWARD_HIGH_JUMP || this->_action == ACTION_BACKWARD_HIGH_JUMP)
			return this->_forceStartMove(ACTION_FALLING);
		AObject::_onMoveEnd(lastData);
	}

	void ACharacter::hit(IObject &other, const FrameData *data)
	{
		this->_speed.x += data->pushBack * -this->_dir;
		AObject::hit(other, data);
	}

	void ACharacter::getHit(IObject &other, const FrameData *data)
	{
		auto myData = this->getCurrentFrameData();

		AObject::getHit(other, data);

		if (!data)
			return;
		this->_restand = data->oFlag.restand;
		if (myData->dFlag.invulnerableArmor) {
			game.battleMgr->addHitStop(data->hitStop);
			return;
		}
		if (
			!this->_isBlocking() ||
			(myData->dFlag.airborne && data->oFlag.airUnblockable) ||
			data->oFlag.unblockable ||
			data->oFlag.grab
		) {
			if (this->_isGrounded() && data->hitSpeed.y <= 0)
				this->_forceStartMove(myData->dFlag.crouch ? ACTION_GROUND_LOW_HIT : ACTION_GROUND_HIGH_HIT);
			else
				this->_forceStartMove(ACTION_AIR_HIT);
			if (myData->dFlag.counterHit && data->oFlag.canCounterHit) {
				this->_hp -= data->damage * 1.5;
				this->_blockStun = data->hitStun * 1.5;
				this->_speed += data->counterHitSpeed;
				this->_speed.x *= -this->_dir;
				logger.debug("Counter hit !: " + std::to_string(this->_blockStun) + " hitstun frames");
			} else {
				this->_hp -= data->damage;
				this->_blockStun = data->hitStun;
				this->_speed += data->hitSpeed;
				this->_speed.x *= -this->_dir;
				logger.debug(std::to_string(this->_blockStun) + " hitstun frames");
			}
		} else {
			if (this->_isGrounded())
				this->_forceStartMove(myData->dFlag.crouch ? ACTION_GROUND_LOW_NEUTRAL_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_BLOCK);
			else
				this->_forceStartMove(ACTION_AIR_NEUTRAL_BLOCK);
			this->_blockStun = data->blockStun;
			this->_speed.x += data->pushBlock * -this->_dir;
		}
		game.battleMgr->addHitStop(data->hitStop);
	}

	bool ACharacter::_isBlocking()
	{
		auto *data = this->getCurrentFrameData();

		if (this->_input->isPressed(this->_direction ? INPUT_LEFT : INPUT_RIGHT) && data->dFlag.canBlock)
			return true;
		return data->dFlag.neutralBlock || data->dFlag.spiritBlock || data->dFlag.matterBlock || data->dFlag.voidBlock;
	}

	bool ACharacter::_isGrounded() const
	{
		return this->_position.y <= 0;
	}

	void ACharacter::_forceStartMove(unsigned int action)
	{
		if (action == ACTION_NEUTRAL_JUMP || action == ACTION_FORWARD_JUMP || action == ACTION_BACKWARD_JUMP) {
			this->_jumpsUsed++;
			this->_hasJumped = true;
		} else if (action == ACTION_NEUTRAL_HIGH_JUMP || action == ACTION_FORWARD_HIGH_JUMP || action == ACTION_BACKWARD_HIGH_JUMP) {
			this->_jumpsUsed += 2;
			this->_hasJumped = true;
		} else if (action >= ACTION_AIR_DASH_1 && action <= ACTION_AIR_DASH_9) {
			if (this->_action == ACTION_NEUTRAL_JUMP || this->_action == ACTION_FORWARD_JUMP || this->_action == ACTION_BACKWARD_JUMP) {
				this->_jumpsUsed--;
				this->_hasJumped = false;
			}
		} else if (action >= ACTION_5N)
			this->_hasJumped = true;
		AObject::_forceStartMove(action);
	}

	void ACharacter::setOpponent(ACharacter *opponent)
	{
		this->_opponent = opponent;
	}

	bool ACharacter::_canCancel(unsigned int action)
	{
		auto currentData = this->getCurrentFrameData();

		if (action == ACTION_FORWARD_AIR_TECH || action == ACTION_BACKWARD_AIR_TECH || action == ACTION_UP_AIR_TECH || action == ACTION_DOWN_AIR_TECH)
			return this->_action == ACTION_AIR_HIT && this->_blockStun == 0;
		if (action == ACTION_BACKWARD_DASH && currentData->oFlag.backDashCancelable)
			return true;
		if (currentData->oFlag.jumpCancelable && action >= ACTION_NEUTRAL_JUMP && action <= ACTION_BACKWARD_HIGH_JUMP)
			return true;
		if (action < 100)
			return false;
		if (!currentData->oFlag.cancelable)
			return false;
		if (!this->_hasHit && !currentData->dFlag.charaCancel)
			return false;
		if (action == this->_action && currentData->oFlag.jab)
			return true;
		if (this->_getAttackTier(action) > this->_getAttackTier(this->_action))
			return true;
		if (currentData->oFlag.hitSwitch && this->_action != action && this->_getAttackTier(action) == this->_getAttackTier(this->_action))
			return true;
		return false;
	}

	int ACharacter::_getAttackTier(unsigned int action) const
	{
		const FrameData *data;
		bool isTyped = action >= ACTION_5M;

		if (action < 100)
			return -1;

		try {
			data = &this->_moves.at(action).at(0).at(0);
		} catch (...) {
			return -1;
		}
		if (data->oFlag.super)
			return 6;
		if (data->oFlag.ultimate)
			return 7;
		switch ((action % 50) + 100) {
		case ACTION_5N:
		case ACTION_2N:
		case ACTION_j5N:
			return 0 + isTyped;
		case ACTION_6N:
		case ACTION_8N:
		case ACTION_3N:
		case ACTION_j6N:
		case ACTION_j8N:
		case ACTION_j3N:
		case ACTION_j2N:
			return 2 + isTyped;
		case ACTION_214N:
		case ACTION_236N:
		case ACTION_421N:
		case ACTION_623N:
		case ACTION_41236N:
		case ACTION_63214N:
		case ACTION_6321469874N:
		case ACTION_j214N:
		case ACTION_j236N:
		case ACTION_j421N:
		case ACTION_j623N:
		case ACTION_j41236N:
		case ACTION_j63214N:
		case ACTION_j6321469874N:
			return 4 + isTyped;
		default:
			return -1;
		}
	}

	void ACharacter::_checkSpecialInputs()
	{
		this->_clearLastInputs();
		this->_specialInputs._value = 0;
		this->_specialInputs._236 = this->_check236Input();
		this->_specialInputs._214 = this->_check214Input();
		this->_specialInputs._623 = this->_check623Input();
		this->_specialInputs._421 = this->_check421Input();
		this->_specialInputs._624 = this->_check624Input();
		this->_specialInputs._426 = this->_check426Input();
		this->_specialInputs._6314 = this->_check6314Input();
		this->_specialInputs._4136 = this->_check4136Input();
		this->_specialInputs._624684 = this->_check624684Input();
		this->_specialInputs._6314684 = this->_check6314684Input();
		this->_specialInputs._6246974 = this->_check6246974Input();
		this->_specialInputs._63146974 = this->_check63146974Input();
	}

	bool ACharacter::_check236Input()
	{
		unsigned total = 0;
		bool found2 = false;
		bool found3 = false;
		bool found6 = false;

		for (auto &input : this->_lastInputs) {
			found6 |= !input.v && input.h > 0;
			found3 |= found6 && input.v < 0 && input.h > 0;
			found2 |= found3 && input.v < 0 && !input.h;
			if (found2 && found3 && found6)
				return true;
			total += input.nbFrames;
			if (total > QUARTER_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check214Input()
	{
		unsigned total = 0;
		bool found2 = false;
		bool found1 = false;
		bool found4 = false;

		for (auto &input : this->_lastInputs) {
			found4 |= !input.v && input.h < 0;
			found1 |= found4 && input.v < 0 && input.h < 0;
			found2 |= found1 && input.v < 0 && !input.h;
			if (found2 && found1 && found4)
				return true;
			total += input.nbFrames;
			if (total > QUARTER_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check623Input()
	{
		unsigned total = 0;
		bool found2 = false;
		bool found3 = false;
		bool found6 = false;

		for (auto &input : this->_lastInputs) {
			found3 |= input.v < 0 && input.h > 0;
			found2 |= found3 && input.v < 0 && !input.h;
			found6 |= found2 && !input.v && input.h > 0;
			if (found2 && found3 && found6)
				return true;
			total += input.nbFrames;
			if (total > DP_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check421Input()
	{
		unsigned total = 0;
		bool found2 = false;
		bool found1 = false;
		bool found4 = false;

		for (auto &input : this->_lastInputs) {
			found1 |= input.v < 0 && input.h < 0;
			found2 |= found1 && input.v < 0 && !input.h;
			found4 |= found2 && !input.v && input.h < 0;
			if (found2 && found1 && found4)
				return true;
			total += input.nbFrames;
			if (total > DP_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check624Input()
	{
		unsigned total = 0;
		bool found2 = false;
		bool found4 = false;
		bool found6 = false;

		for (auto &input : this->_lastInputs) {
			found4 |= !input.v && input.h < 0;
			found2 |= found4 && input.v < 0 && !input.h;
			found6 |= found2 && !input.v && input.h > 0;
			if (found2 && found4 && found6)
				return true;
			total += input.nbFrames;
			if (total > HALF_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check426Input()
	{
		unsigned total = 0;
		bool found2 = false;
		bool found4 = false;
		bool found6 = false;

		for (auto &input : this->_lastInputs) {
			found6 |= !input.v && input.h > 0;
			found2 |= found4 && input.v < 0 && !input.h;
			found4 |= found2 && !input.v && input.h < 0;
			if (found2 && found4 && found6)
				return true;
			total += input.nbFrames;
			if (total > HALF_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check6314Input()
	{
		unsigned total = 0;
		bool found1 = false;
		bool found3 = false;
		bool found4 = false;
		bool found6 = false;

		for (auto &input : this->_lastInputs) {
			found4 |= !input.v && input.h < 0;
			found1 |= found4 && input.v < 0 && input.h < 0;
			found3 |= found1 && input.v < 0 && input.h > 0;
			found6 |= found3 && !input.v && input.h > 0;
			if (found1 && found3 && found4 && found6)
				return true;
			total += input.nbFrames;
			if (total > HALF_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check4136Input()
	{
		unsigned total = 0;
		bool found1 = false;
		bool found3 = false;
		bool found4 = false;
		bool found6 = false;

		for (auto &input : this->_lastInputs) {
			found6 |= !input.v && input.h > 0;
			found3 |= found6 && input.v < 0 && input.h > 0;
			found1 |= found3 && input.v < 0 && input.h < 0;
			found4 |= found1 && !input.v && input.h < 0;
			if (found1 && found3 && found4 && found6)
				return true;
			total += input.nbFrames;
			if (total > HALF_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check624684Input()
	{
		unsigned total = 0;
		bool found6_1 = false;
		bool found2 = false;
		bool found4_1 = false;
		bool found6_2 = false;
		bool found8 = false;
		bool found4_2 = false;

		for (auto &input : this->_lastInputs) {
			found4_2 |= !input.v && input.h < 0;
			found8   |= found4_2 && input.v > 0 && !input.h;
			found6_2 |= found8   && !input.v && input.h > 0;
			found4_1 |= found6_2 && !input.v && input.h < 0;
			found2   |= found4_1 && input.v < 0 && !input.h;
			found6_1 |= found2   && !input.v && input.h > 0;
			if (found6_1 && found2 && found4_1 && found6_2 && found8 && found4_2)
				return true;
			total += input.nbFrames;
			if (total > SPIRAL_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check6314684Input()
	{
		unsigned total = 0;
		bool found6_1 = false;
		bool found3 = false;
		bool found1 = false;
		bool found4_1 = false;
		bool found6_2 = false;
		bool found8 = false;
		bool found4_2 = false;

		for (auto &input : this->_lastInputs) {
			found4_2 |= !input.v && input.h < 0;
			found8   |= found4_2 && input.v > 0 && !input.h;
			found6_2 |= found8   && !input.v && input.h > 0;
			found4_1 |= found6_2 && !input.v && input.h < 0;
			found1   |= found4_1 && input.v < 0 && input.h < 0;
			found3   |= found1   && input.v < 0 && input.h > 0;
			found6_1 |= found3   && !input.v && input.h > 0;
			if (found6_1 && found3 && found1 && found4_1 && found6_2 && found8 && found4_2)
				return true;
			total += input.nbFrames;
			if (total > SPIRAL_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check6246974Input()
	{
		unsigned total = 0;
		bool found6_1 = false;
		bool found2 = false;
		bool found4_1 = false;
		bool found6_2 = false;
		bool found9 = false;
		bool found7 = false;
		bool found4_2 = false;

		for (auto &input : this->_lastInputs) {
			found4_2 |= !input.v && input.h < 0;
			found7   |= found4_2 && input.v > 0 && input.h < 0;
			found9   |= found7   && input.v > 0 && input.h > 0;
			found6_2 |= found9   && !input.v && input.h > 0;
			found4_1 |= found6_2 && !input.v && input.h < 0;
			found2   |= found4_1 && input.v < 0 && !input.h;
			found6_1 |= found2   && !input.v && input.h > 0;
			if (found6_1 && found2 && found4_1 && found6_2 && found9 && found7 && found4_2)
				return true;
			total += input.nbFrames;
			if (total > SPIRAL_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check63146974Input()
	{
		unsigned total = 0;
		bool found6_1 = false;
		bool found3 = false;
		bool found1 = false;
		bool found4_1 = false;
		bool found6_2 = false;
		bool found9 = false;
		bool found7 = false;
		bool found4_2 = false;

		for (auto &input : this->_lastInputs) {
			found4_2 |= !input.v && input.h < 0;
			found7   |= found4_2 && input.v > 0 && input.h < 0;
			found9   |= found7   && input.v > 0 && input.h > 0;
			found6_2 |= found9   && !input.v && input.h > 0;
			found4_1 |= found6_2 && !input.v && input.h < 0;
			found1   |= found4_1 && input.v < 0 && input.h < 0;
			found3   |= found1   && input.v < 0 && input.h > 0;
			found6_1 |= found3   && !input.v && input.h > 0;
			if (found6_1 && found3 && found1 && found4_1 && found6_2 && found9 && found7 && found4_2)
				return true;
			total += input.nbFrames;
			if (total > SPIRAL_BUFFER)
				break;
		}
		return false;
	}

	void ACharacter::_clearLastInputs()
	{
		auto it = this->_lastInputs.begin();
		unsigned total = 0;

		while (it != this->_lastInputs.end() && total < 45) {
			total += it->nbFrames;
			it++;
		}
		this->_lastInputs.erase(it, this->_lastInputs.end());
	}
}
