//
// Created by PinkySmile on 18/09/2021
//

#ifdef _WIN32
#include <windows.h>
#endif
#include "ACharacter.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"
#ifndef max
#define max(x, y) (x > y ? x : y)
#endif
#ifndef min
#define min(x, y) (x < y ? x : y)
#endif

#define WALL_SLAM_THRESHOLD 15
#define GROUND_SLAM_THRESHOLD 20
#define HJ_BUFFER 5
#define DASH_BUFFER 7
#define QUARTER_CIRCLE_BUFFER 10
#define DP_BUFFER 15
#define HALF_CIRCLE_BUFFER 20
#define SPIRAL_BUFFER 30
#define NORMAL_BUFFER 4
#define CHARGE_PART_BUFFER 10
#define CHARGE_BUFFER 5
#define CHARGE_TIME 25

#ifdef _DEBUG
static const char *oFlags[] = {
	"grab",
	"airUnblockable",
	"unblockable",
	"voidElement",
	"spiritElement",
	"matterElement",
	"lowHit",
	"highHit",
	"autoHitPos",
	"canCounterHit",
	"hitSwitch",
	"cancelable",
	"jab",
	"resetHits",
	"resetSpeed",
	"restand",
	"super",
	"ultimate",
	"jumpCancelable",
	"transformCancelable",
	"unTransformCancelable",
	"dashCancelable",
	"backDashCancelable",
	"voidMana",
	"spiritMana",
	"matterMana"
};

static const char *dFlags[] = {
	"invulnerable",
	"invulnerableArmor",
	"superarmor",
	"grabInvulnerable",
	"voidBlock",
	"spiritBlock",
	"matterBlock",
	"neutralBlock",
	"airborne",
	"canBlock",
	"highBlock",
	"lowBlock",
	"charaCancel",
	"resetRotation",
	"counterHit",
	"flash",
	"crouch",
	"projectileInvul",
	"projectile",
	"landCancel",
	"dashCancel"
};
#endif
namespace Battle
{
	ACharacter::ACharacter(const std::string &frameData, const std::pair<std::vector<Color>, std::vector<Color>> &palette, std::shared_ptr<IInput> input) :
		_input(std::move(input))
	{
#ifdef _DEBUG
		this->_font.loadFromFile(getenv("SYSTEMROOT") + std::string("\\Fonts\\comic.ttf"));
		this->_text.setFont(this->_font);
		this->_text.setFillColor(sf::Color::White);
		this->_text.setOutlineColor(sf::Color::Black);
		this->_text.setOutlineThickness(2);
		this->_text.setCharacterSize(10);
		this->_text2.setFont(this->_font);
		this->_text2.setFillColor(sf::Color::White);
		this->_text2.setOutlineColor(sf::Color::Black);
		this->_text2.setOutlineThickness(2);
		this->_text2.setCharacterSize(10);
#endif
		this->_limit.fill(0);
		this->_moves = FrameData::loadFile(frameData, palette);
		this->_lastInputs.push_back({0, 0, 0});
	}

	void ACharacter::render() const
	{
		AObject::render();
#ifdef _DEBUG
		game.screen->draw(this->_text);
		game.screen->draw(this->_text2);
#endif
	}

	void ACharacter::update()
	{
		this->_tickMove();
		if (this->_isGrounded() && this->_action >= ACTION_UP_AIR_TECH && this->_action <= ACTION_BACKWARD_AIR_TECH)
			this->_forceStartMove(ACTION_AIR_TECH_LANDING_LAG);
		if (
			this->_isGrounded() &&
			this->_action != ACTION_NEUTRAL_JUMP &&
			this->_action != ACTION_FORWARD_JUMP &&
			this->_action != ACTION_BACKWARD_JUMP &&
			this->_action != ACTION_NEUTRAL_AIR_JUMP &&
			this->_action != ACTION_FORWARD_AIR_JUMP &&
			this->_action != ACTION_BACKWARD_AIR_JUMP &&
			this->_action != ACTION_NEUTRAL_HIGH_JUMP &&
			this->_action != ACTION_FORWARD_HIGH_JUMP &&
			this->_action != ACTION_BACKWARD_HIGH_JUMP
		)
			this->_jumpsUsed = 0;
		if (this->_action >= ACTION_AIR_DASH_1 && this->_action <= ACTION_AIR_DASH_9 && this->_isGrounded())
			this->_forceStartMove(ACTION_HARD_LAND);
		else if (this->_action == ACTION_AIR_HIT && this->_isGrounded()) {
			this->_blockStun = 0;
			this->_forceStartMove(ACTION_BEING_KNOCKED_DOWN);
		} else if ((
			this->_action == ACTION_AIR_NEUTRAL_BLOCK ||
			this->_action == ACTION_AIR_MATTER_BLOCK ||
			this->_action == ACTION_AIR_VOID_BLOCK ||
			this->_action == ACTION_AIR_SPIRIT_BLOCK ||
			(this->_action >= ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK &&
			this->_action <= ACTION_AIR_VOID_WRONG_BLOCK)
		) && this->_isGrounded()) {
			this->_blockStun = 0;
			this->_forceStartMove(ACTION_IDLE);
		}
		if (this->_blockStun) {
			this->_blockStun--;
			if (this->_blockStun == 0) {
				if (this->_isGrounded())
					this->_forceStartMove(this->getCurrentFrameData()->dFlag.crouch ? ACTION_CROUCH : ACTION_IDLE);
				else if (this->_action != ACTION_AIR_HIT || this->_restand)
					this->_forceStartMove(ACTION_FALLING);
			}
		}
		if (!this->_blockStun)
			this->_processInput(this->updateInputs());

		this->_applyMoveAttributes();
		if (this->_position.x < 0) {
			this->_position.x = 0;
			if (std::abs(this->_speed.x) >= WALL_SLAM_THRESHOLD && (
				this->_action == ACTION_AIR_HIT || this->_action == ACTION_GROUND_HIGH_HIT || this->_action == ACTION_GROUND_LOW_HIT)
			) {
				this->_forceStartMove(ACTION_WALL_SLAM);
				this->_speed.x *= -0.8;
				this->_speed.y = this->_blockStun / (this->_baseGravity.y * -2);
			} else
				this->_speed.x = 0;
		} else if (this->_position.x > 1000) {
			this->_position.x = 1000;
			if (std::abs(this->_speed.x) >= WALL_SLAM_THRESHOLD && (
				this->_action == ACTION_AIR_HIT || this->_action == ACTION_GROUND_HIGH_HIT || this->_action == ACTION_GROUND_LOW_HIT
			)) {
				this->_forceStartMove(ACTION_WALL_SLAM);
				this->_speed.x *= -0.8;
				this->_speed.y = this->_blockStun / (this->_baseGravity.y * -2);
			} else
				this->_speed.x = 0;
		}

		auto newPriority = (this->_position.x >= 1000) - (this->_position.x <= 0);

		this->_justGotCorner = newPriority && !this->_cornerPriority;
		if (newPriority && this->_justGotCorner) {
			if (newPriority == this->_opponent->_cornerPriority) {
				if (this->_team == 1) {
					if (this->_opponent->_justGotCorner && this->_opponent->_cornerPriority == 1) {
						this->_cornerPriority = newPriority;
						this->_opponent->_justGotCorner = false;
						this->_opponent->_cornerPriority = 0;
					} else
						this->_cornerPriority = 0;
				} else
					this->_cornerPriority = 0;
			} else {
				this->_justGotCorner = newPriority && !this->_cornerPriority;
				this->_cornerPriority = newPriority;
			}
		} else if (!newPriority) {
			this->_justGotCorner = false;
			this->_cornerPriority = 0;
		}
		if (this->_position.y < 0) {
			this->_position.y = 0;

			if (std::abs(this->_speed.y) >= GROUND_SLAM_THRESHOLD && (
				this->_action == ACTION_AIR_HIT || this->_action == ACTION_GROUND_HIGH_HIT || this->_action == ACTION_GROUND_LOW_HIT
			)) {
				this->_speed.x *= 0.8;
				this->_speed.y *= -0.8;
				this->_forceStartMove(ACTION_GROUND_SLAM);
			} else
				this->_speed.y = 0;
		} else if (this->_position.y > 1000) {
			this->_position.y = 1000;

			if (std::abs(this->_speed.y) >= GROUND_SLAM_THRESHOLD && (
				this->_action == ACTION_AIR_HIT || this->_action == ACTION_GROUND_HIGH_HIT || this->_action == ACTION_GROUND_LOW_HIT
			)) {
				this->_speed.x *= 0.8;
				this->_speed.y *= -0.8;
				this->_forceStartMove(ACTION_GROUND_SLAM);
			} else
				this->_speed.y = 0;
		}

		if (this->_action < ACTION_LANDING && this->_opponent) {
			if (this->_opponent->_position.x - this->_position.x != 0)
				this->_dir = std::copysign(1, this->_opponent->_position.x - this->_position.x);
			this->_direction = this->_dir == 1;
		}
	}

	void ACharacter::init(bool side, unsigned short maxHp, unsigned char maxJumps, Vector2f gravity)
	{
		this->_dir = side ? 1 : -1;
		this->_direction = side;
		this->_team = !side;
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
			(data->dFlag.airborne && this->_executeAirborneMoves(input)) ||
			(!data->dFlag.airborne && this->_executeGroundMoves(input))
		)
			return;
		if (this->_isGrounded())
			this->_startMove(this->_action == ACTION_CROUCH ? ACTION_STANDING_UP : ACTION_IDLE);
		else
			this->_startMove(ACTION_FALLING);
	}

	InputStruct ACharacter::updateInputs()
	{
		this->_input->update();

		InputStruct input = this->_input->getInputs();

		if (
			std::copysign(!!input.horizontalAxis, this->_dir * input.horizontalAxis) != this->_lastInputs.front().h ||
			std::copysign(!!input.verticalAxis,   this->_dir * input.verticalAxis)   != this->_lastInputs.front().v
		)
			this->_lastInputs.push_front({
				0,
				static_cast<int>(std::copysign(!!input.horizontalAxis, this->_dir * input.horizontalAxis)),
				static_cast<int>(std::copysign(!!input.verticalAxis,   input.verticalAxis))
			});
		this->_lastInputs.front().nbFrames++;
		if (this->_lastInputs.front().nbFrames > 45)
			this->_lastInputs.front().nbFrames = 45;
		this->_checkSpecialInputs();
		this->_hasJumped &= input.verticalAxis > 0;
		return input;
	}

	bool ACharacter::_executeAirborneMoves(const InputStruct &input)
	{
		return  //(input.n && input.n <= 4 && this->_startMove(ACTION_j5N));
			this->_executeAirTech(input) ||

		        (input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._624684  || this->_specialInputs._6314684)  && this->_startMove(ACTION_j6321469874N)) ||
		        (input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._6246974 || this->_specialInputs._63146974) && this->_startMove(ACTION_j6321469874N)) ||
		        (input.v && input.v <= NORMAL_BUFFER && (this->_specialInputs._624684  || this->_specialInputs._6314684)  && this->_startMove(ACTION_j6321469874V)) ||
		        (input.v && input.v <= NORMAL_BUFFER && (this->_specialInputs._6246974 || this->_specialInputs._63146974) && this->_startMove(ACTION_j6321469874V)) ||
		        (input.s && input.s <= NORMAL_BUFFER && (this->_specialInputs._624684  || this->_specialInputs._6314684)  && this->_startMove(ACTION_j6321469874S)) ||
		        (input.s && input.s <= NORMAL_BUFFER && (this->_specialInputs._6246974 || this->_specialInputs._63146974) && this->_startMove(ACTION_j6321469874S)) ||
		        (input.m && input.m <= NORMAL_BUFFER && (this->_specialInputs._624684  || this->_specialInputs._6314684)  && this->_startMove(ACTION_j6321469874M)) ||
		        (input.m && input.m <= NORMAL_BUFFER && (this->_specialInputs._6246974 || this->_specialInputs._63146974) && this->_startMove(ACTION_j6321469874M)) ||

		        (input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._624 || this->_specialInputs._6314) &&         this->_startMove(ACTION_j63214N)) ||
		        (input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._426 || this->_specialInputs._4136) &&         this->_startMove(ACTION_j41236N)) ||
		        (input.v && input.v <= NORMAL_BUFFER && (this->_specialInputs._624 || this->_specialInputs._6314) &&         this->_startMove(ACTION_j63214V)) ||
		        (input.v && input.v <= NORMAL_BUFFER && (this->_specialInputs._426 || this->_specialInputs._4136) &&         this->_startMove(ACTION_j41236V)) ||
		        (input.s && input.s <= NORMAL_BUFFER && (this->_specialInputs._624 || this->_specialInputs._6314) &&         this->_startMove(ACTION_j63214S)) ||
		        (input.s && input.s <= NORMAL_BUFFER && (this->_specialInputs._426 || this->_specialInputs._4136) &&         this->_startMove(ACTION_j41236S)) ||
		        (input.m && input.m <= NORMAL_BUFFER && (this->_specialInputs._624 || this->_specialInputs._6314) &&         this->_startMove(ACTION_j63214M)) ||
		        (input.m && input.m <= NORMAL_BUFFER && (this->_specialInputs._426 || this->_specialInputs._4136) &&         this->_startMove(ACTION_j41236M)) ||

		        (input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._623 &&                                         this->_startMove(ACTION_j623N)) ||
		        (input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._421 &&                                         this->_startMove(ACTION_j421N)) ||
		        (input.v && input.v <= NORMAL_BUFFER && this->_specialInputs._623 &&                                         this->_startMove(ACTION_j623V)) ||
		        (input.v && input.v <= NORMAL_BUFFER && this->_specialInputs._421 &&                                         this->_startMove(ACTION_j421V)) ||
		        (input.s && input.s <= NORMAL_BUFFER && this->_specialInputs._623 &&                                         this->_startMove(ACTION_j623S)) ||
		        (input.s && input.s <= NORMAL_BUFFER && this->_specialInputs._421 &&                                         this->_startMove(ACTION_j421S)) ||
		        (input.m && input.m <= NORMAL_BUFFER && this->_specialInputs._623 &&                                         this->_startMove(ACTION_j623M)) ||
		        (input.m && input.m <= NORMAL_BUFFER && this->_specialInputs._421 &&                                         this->_startMove(ACTION_j421M)) ||

		        (input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._236 &&                                         this->_startMove(ACTION_j236N)) ||
		        (input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._214 &&                                         this->_startMove(ACTION_j214N)) ||
		        (input.v && input.v <= NORMAL_BUFFER && this->_specialInputs._236 &&                                         this->_startMove(ACTION_j236V)) ||
		        (input.v && input.v <= NORMAL_BUFFER && this->_specialInputs._214 &&                                         this->_startMove(ACTION_j214V)) ||
		        (input.s && input.s <= NORMAL_BUFFER && this->_specialInputs._236 &&                                         this->_startMove(ACTION_j236S)) ||
		        (input.s && input.s <= NORMAL_BUFFER && this->_specialInputs._214 &&                                         this->_startMove(ACTION_j214S)) ||
		        (input.m && input.m <= NORMAL_BUFFER && this->_specialInputs._236 &&                                         this->_startMove(ACTION_j236M)) ||
		        (input.m && input.m <= NORMAL_BUFFER && this->_specialInputs._214 &&                                         this->_startMove(ACTION_j214M)) ||

		        this->_executeAirBlock(input) ||

		        (input.n && input.n <= NORMAL_BUFFER && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8N)) ||
		        (input.n && input.n <= NORMAL_BUFFER && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3N)) ||
		        (input.n && input.n <= NORMAL_BUFFER &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6N)) ||
		        (input.n && input.n <= NORMAL_BUFFER && input.verticalAxis < 0 &&                                            this->_startMove(ACTION_j2N)) ||
		        (input.n && input.n <= NORMAL_BUFFER &&                                                                      this->_startMove(ACTION_j5N)) ||
		        (input.v && input.v <= NORMAL_BUFFER && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8V)) ||
		        (input.v && input.v <= NORMAL_BUFFER && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3V)) ||
		        (input.v && input.v <= NORMAL_BUFFER &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6V)) ||
		        (input.v && input.v <= NORMAL_BUFFER && input.verticalAxis < 0 &&                                            this->_startMove(ACTION_j2V)) ||
		        (input.v && input.v <= NORMAL_BUFFER &&                                                                      this->_startMove(ACTION_j5V)) ||
		        (input.s && input.s <= NORMAL_BUFFER && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8S)) ||
		        (input.s && input.s <= NORMAL_BUFFER && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3S)) ||
		        (input.s && input.s <= NORMAL_BUFFER &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6S)) ||
		        (input.s && input.s <= NORMAL_BUFFER && input.verticalAxis < 0 &&                                            this->_startMove(ACTION_j2S)) ||
		        (input.s && input.s <= NORMAL_BUFFER &&                                                                      this->_startMove(ACTION_j5S)) ||
		        (input.m && input.m <= NORMAL_BUFFER && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8M)) ||
		        (input.m && input.m <= NORMAL_BUFFER && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3M)) ||
		        (input.m && input.m <= NORMAL_BUFFER &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6M)) ||
		        (input.m && input.m <= NORMAL_BUFFER && input.verticalAxis < 0 &&                                            this->_startMove(ACTION_j2M)) ||
		        (input.m && input.m <= NORMAL_BUFFER &&                                                                      this->_startMove(ACTION_j5M)) ||
		        this->_executeAirJump(input)   ||
		        this->_executeAirDashes(input);
	}

	bool ACharacter::_executeGroundMoves(const InputStruct &input)
	{
		return  //(input.n && input.n <= 4 && this->_startMove(ACTION_5N)) ||
			(input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._624684  || this->_specialInputs._6314684)  && this->_startMove(ACTION_6321469874N)) ||
			(input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._6246974 || this->_specialInputs._63146974) && this->_startMove(ACTION_6321469874N)) ||
			(input.v && input.v <= NORMAL_BUFFER && (this->_specialInputs._624684  || this->_specialInputs._6314684)  && this->_startMove(ACTION_6321469874V)) ||
			(input.v && input.v <= NORMAL_BUFFER && (this->_specialInputs._6246974 || this->_specialInputs._63146974) && this->_startMove(ACTION_6321469874V)) ||
			(input.s && input.s <= NORMAL_BUFFER && (this->_specialInputs._624684  || this->_specialInputs._6314684)  && this->_startMove(ACTION_6321469874S)) ||
			(input.s && input.s <= NORMAL_BUFFER && (this->_specialInputs._6246974 || this->_specialInputs._63146974) && this->_startMove(ACTION_6321469874S)) ||
			(input.m && input.m <= NORMAL_BUFFER && (this->_specialInputs._624684  || this->_specialInputs._6314684)  && this->_startMove(ACTION_6321469874M)) ||
			(input.m && input.m <= NORMAL_BUFFER && (this->_specialInputs._6246974 || this->_specialInputs._63146974) && this->_startMove(ACTION_6321469874M)) ||

			(input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._624 || this->_specialInputs._6314) &&         this->_startMove(ACTION_63214N)) ||
			(input.n && input.n <= NORMAL_BUFFER && (this->_specialInputs._426 || this->_specialInputs._4136) &&         this->_startMove(ACTION_41236N)) ||
			(input.v && input.v <= NORMAL_BUFFER && (this->_specialInputs._624 || this->_specialInputs._6314) &&         this->_startMove(ACTION_63214V)) ||
			(input.v && input.v <= NORMAL_BUFFER && (this->_specialInputs._426 || this->_specialInputs._4136) &&         this->_startMove(ACTION_41236V)) ||
			(input.s && input.s <= NORMAL_BUFFER && (this->_specialInputs._624 || this->_specialInputs._6314) &&         this->_startMove(ACTION_63214S)) ||
			(input.s && input.s <= NORMAL_BUFFER && (this->_specialInputs._426 || this->_specialInputs._4136) &&         this->_startMove(ACTION_41236S)) ||
			(input.m && input.m <= NORMAL_BUFFER && (this->_specialInputs._624 || this->_specialInputs._6314) &&         this->_startMove(ACTION_63214M)) ||
			(input.m && input.m <= NORMAL_BUFFER && (this->_specialInputs._426 || this->_specialInputs._4136) &&         this->_startMove(ACTION_41236M)) ||

			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._623 &&                                         this->_startMove(ACTION_623N)) ||
			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._421 &&                                         this->_startMove(ACTION_421N)) ||
			(input.v && input.v <= NORMAL_BUFFER && this->_specialInputs._623 &&                                         this->_startMove(ACTION_623V)) ||
			(input.v && input.v <= NORMAL_BUFFER && this->_specialInputs._421 &&                                         this->_startMove(ACTION_421V)) ||
			(input.s && input.s <= NORMAL_BUFFER && this->_specialInputs._623 &&                                         this->_startMove(ACTION_623S)) ||
			(input.s && input.s <= NORMAL_BUFFER && this->_specialInputs._421 &&                                         this->_startMove(ACTION_421S)) ||
			(input.m && input.m <= NORMAL_BUFFER && this->_specialInputs._623 &&                                         this->_startMove(ACTION_623M)) ||
			(input.m && input.m <= NORMAL_BUFFER && this->_specialInputs._421 &&                                         this->_startMove(ACTION_421M)) ||

			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._236 &&                                         this->_startMove(ACTION_236N)) ||
			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._214 &&                                         this->_startMove(ACTION_214N)) ||
			(input.v && input.v <= NORMAL_BUFFER && this->_specialInputs._236 &&                                         this->_startMove(ACTION_236V)) ||
			(input.v && input.v <= NORMAL_BUFFER && this->_specialInputs._214 &&                                         this->_startMove(ACTION_214V)) ||
			(input.s && input.s <= NORMAL_BUFFER && this->_specialInputs._236 &&                                         this->_startMove(ACTION_236S)) ||
			(input.s && input.s <= NORMAL_BUFFER && this->_specialInputs._214 &&                                         this->_startMove(ACTION_214S)) ||
			(input.m && input.m <= NORMAL_BUFFER && this->_specialInputs._236 &&                                         this->_startMove(ACTION_236M)) ||
			(input.m && input.m <= NORMAL_BUFFER && this->_specialInputs._214 &&                                         this->_startMove(ACTION_214M)) ||

			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._c64 &&                                         this->_startMove(ACTION_c64N)) ||
			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._c46 &&                                         this->_startMove(ACTION_c46N)) ||
			(input.n && input.n <= NORMAL_BUFFER && this->_specialInputs._c28 &&                                         this->_startMove(ACTION_c28N)) ||
			(input.v && input.v <= NORMAL_BUFFER && this->_specialInputs._c64 &&                                         this->_startMove(ACTION_c64V)) ||
			(input.v && input.v <= NORMAL_BUFFER && this->_specialInputs._c46 &&                                         this->_startMove(ACTION_c46V)) ||
			(input.v && input.v <= NORMAL_BUFFER && this->_specialInputs._c28 &&                                         this->_startMove(ACTION_c28V)) ||
			(input.s && input.s <= NORMAL_BUFFER && this->_specialInputs._c64 &&                                         this->_startMove(ACTION_c64S)) ||
			(input.s && input.s <= NORMAL_BUFFER && this->_specialInputs._c46 &&                                         this->_startMove(ACTION_c46S)) ||
			(input.s && input.s <= NORMAL_BUFFER && this->_specialInputs._c28 &&                                         this->_startMove(ACTION_c28S)) ||
			(input.m && input.m <= NORMAL_BUFFER && this->_specialInputs._c64 &&                                         this->_startMove(ACTION_c64M)) ||
			(input.m && input.m <= NORMAL_BUFFER && this->_specialInputs._c46 &&                                         this->_startMove(ACTION_c46M)) ||
			(input.m && input.m <= NORMAL_BUFFER && this->_specialInputs._c28 &&                                         this->_startMove(ACTION_c28M)) ||

			this->_executeGroundBlock(input) ||

			(input.n && input.n <= NORMAL_BUFFER && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8N)) ||
			(input.n && input.n <= NORMAL_BUFFER && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3N)) ||
			(input.n && input.n <= NORMAL_BUFFER &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6N)) ||
			(input.n && input.n <= NORMAL_BUFFER && input.verticalAxis < 0 &&                                            this->_startMove(ACTION_2N)) ||
			(input.n && input.n <= NORMAL_BUFFER &&                                                                      this->_startMove(ACTION_5N)) ||
			(input.v && input.v <= NORMAL_BUFFER && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8V)) ||
			(input.v && input.v <= NORMAL_BUFFER && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3V)) ||
			(input.v && input.v <= NORMAL_BUFFER &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6V)) ||
			(input.v && input.v <= NORMAL_BUFFER && input.verticalAxis < 0 &&                                            this->_startMove(ACTION_2V)) ||
			(input.v && input.v <= NORMAL_BUFFER &&                                                                      this->_startMove(ACTION_5V)) ||
			(input.s && input.s <= NORMAL_BUFFER && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8S)) ||
			(input.s && input.s <= NORMAL_BUFFER && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3S)) ||
			(input.s && input.s <= NORMAL_BUFFER &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6S)) ||
			(input.s && input.s <= NORMAL_BUFFER && input.verticalAxis < 0 &&                                            this->_startMove(ACTION_2S)) ||
			(input.s && input.s <= NORMAL_BUFFER &&                                                                      this->_startMove(ACTION_5S)) ||
			(input.m && input.m <= NORMAL_BUFFER && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8M)) ||
			(input.m && input.m <= NORMAL_BUFFER && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3M)) ||
			(input.m && input.m <= NORMAL_BUFFER &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6M)) ||
			(input.m && input.m <= NORMAL_BUFFER && input.verticalAxis < 0 &&                                            this->_startMove(ACTION_2M)) ||
			(input.m && input.m <= NORMAL_BUFFER &&                                                                      this->_startMove(ACTION_5M)) ||
			this->_executeGroundJump(input)   ||
			this->_executeGroundDashes(input) ||
			this->_executeCrouch(input)       ||
			this->_executeWalking(input);
	}

	bool ACharacter::_canStartMove(unsigned action, const FrameData &data)
	{
		if (action == ACTION_UP_AIR_TECH || action == ACTION_DOWN_AIR_TECH || action == ACTION_FORWARD_AIR_TECH || action == ACTION_BACKWARD_AIR_TECH) {
			for (auto limit : this->_limit)
				if (limit >= 100)
					return false;
			return this->_action == ACTION_AIR_HIT;
		}
		if (action == ACTION_IDLE && this->_action == ACTION_STANDING_UP)
			return false;
		if (action == ACTION_CROUCHING && this->_action == ACTION_CROUCH)
			return false;
		if (this->_canCancel(action))
			return true;
		if (action >= ACTION_AIR_DASH_1 && action <= ACTION_AIR_DASH_9)
			return this->_action < ACTION_NEUTRAL_HIGH_JUMP || this->_action == ACTION_FALLING || this->_action == ACTION_LANDING;
		if ((action >= ACTION_NEUTRAL_JUMP && action <= ACTION_BACKWARD_HIGH_JUMP) || (action >= ACTION_NEUTRAL_AIR_JUMP && action <= ACTION_BACKWARD_AIR_JUMP))
			return this->_jumpsUsed < this->_maxJumps && (this->_action <= ACTION_WALK_BACKWARD || this->_action == ACTION_FALLING || this->_action == ACTION_LANDING);
		if (this->_action == action)
			return false;
		if (action >= ACTION_GROUND_HIGH_NEUTRAL_BLOCK && action <= ACTION_AIR_HIT)
			return this->getCurrentFrameData()->dFlag.canBlock;
		if (this->_action >= ACTION_GROUND_HIGH_NEUTRAL_BLOCK && this->_action <= ACTION_AIR_HIT)
			return !this->_blockStun;
		if (
			this->_action >= ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK &&
			this->_action <= ACTION_AIR_VOID_WRONG_BLOCK
		)
			return !this->_blockStun;
		if (action <= ACTION_WALK_BACKWARD || action == ACTION_FALLING || action == ACTION_LANDING)
			return (this->_action <= ACTION_WALK_BACKWARD || this->_action == ACTION_FALLING || this->_action == ACTION_LANDING);
		if (this->_action <= ACTION_WALK_BACKWARD || this->_action == ACTION_FALLING || this->_action == ACTION_LANDING)
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
		if (this->_action == ACTION_AIR_TECH_LANDING_LAG)
			return this->_forceStartMove(ACTION_IDLE);
		if (this->_action == ACTION_FORWARD_DASH)
			return this->_forceStartMove(ACTION_IDLE);
		if (this->_action == ACTION_BACKWARD_DASH)
			return this->_forceStartMove(ACTION_IDLE);
		if (this->_action == ACTION_HARD_LAND)
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
		if (this->_action == ACTION_NEUTRAL_AIR_JUMP || this->_action == ACTION_FORWARD_AIR_JUMP || this->_action == ACTION_BACKWARD_AIR_JUMP)
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
		game.battleMgr->addHitStop(data->hitStop);
		if (myData->dFlag.invulnerableArmor)
			return;
		this->_restand = data->oFlag.restand;
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
				this->_hp -= data->damage * 1.5 * this->_prorate;
				this->_totalDamage += data->damage * 1.5 * this->_prorate;
				this->_comboCtr++;
				this->_blockStun = data->hitStun * 1.5;
				this->_speed.x -= data->counterHitSpeed.x * this->_dir;
				this->_speed.x = max(-data->counterHitSpeed.x * 1.5, min(data->counterHitSpeed.x * 1.5, this->_speed.x));
				this->_speed.y -= data->counterHitSpeed.y;
				this->_prorate *= data->prorate / 100;
				this->_limit[0] += data->neutralLimit;
				this->_limit[1] += data->voidLimit;
				this->_limit[2] += data->matterLimit;
				this->_limit[3] += data->spiritLimit;
				logger.debug("Counter hit !: " + std::to_string(this->_blockStun) + " hitstun frames");
			} else {
				this->_hp -= data->damage * this->_prorate;
				this->_totalDamage += data->damage * this->_prorate;
				this->_comboCtr++;
				this->_blockStun = data->hitStun;
				this->_speed.x -= data->hitSpeed.x * this->_dir;
				this->_speed.x = max(-data->hitSpeed.x * 1.5, min(data->hitSpeed.x * 1.5, this->_speed.x));
				this->_speed.y -= data->hitSpeed.y;
				this->_prorate *= data->prorate / 100;
				this->_limit[0] += data->neutralLimit;
				this->_limit[1] += data->voidLimit;
				this->_limit[2] += data->matterLimit;
				this->_limit[3] += data->spiritLimit;
				logger.debug(std::to_string(this->_blockStun) + " hitstun frames");
			}
		} else {
			unsigned wrongBlockLevel = 0;

			if (data->oFlag.lowHit && !myData->dFlag.lowBlock)
				wrongBlockLevel++;
			else if (data->oFlag.highHit && !myData->dFlag.highBlock)
				wrongBlockLevel++;
			if (data->oFlag.matterElement && data->oFlag.voidElement && data->oFlag.spiritElement && !myData->dFlag.neutralBlock) //TRUE NEUTRAL
				wrongBlockLevel++;
			else {
				if (data->oFlag.matterElement) {
					if (myData->dFlag.voidBlock);
					else if (myData->dFlag.matterBlock || myData->dFlag.neutralBlock)
						wrongBlockLevel += 1;
					else if (myData->dFlag.spiritBlock)
						wrongBlockLevel += 2;
				} else if (data->oFlag.voidElement) {
					if (myData->dFlag.spiritBlock);
					else if (myData->dFlag.voidBlock || myData->dFlag.neutralBlock)
						wrongBlockLevel += 1;
					else if (myData->dFlag.matterBlock)
						wrongBlockLevel += 2;
				} else if (data->oFlag.spiritElement) {
					if (myData->dFlag.matterBlock);
					else if (myData->dFlag.spiritBlock || myData->dFlag.neutralBlock)
						wrongBlockLevel += 1;
					else if (myData->dFlag.voidBlock)
						wrongBlockLevel += 2;
				}
			}
			if (wrongBlockLevel) {
				if (this->_isGrounded())
					this->_forceStartMove(myData->dFlag.crouch ? ACTION_GROUND_LOW_NEUTRAL_WRONG_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK);
				else
					this->_forceStartMove(ACTION_AIR_NEUTRAL_WRONG_BLOCK);
			} else {
				if (this->_isGrounded())
					this->_forceStartMove(myData->dFlag.crouch ? ACTION_GROUND_LOW_NEUTRAL_WRONG_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK);
				else
					this->_forceStartMove(ACTION_AIR_NEUTRAL_BLOCK);
				this->_blockStun *= 3;
				this->_blockStun /= 4;
			}
			this->_blockStun += data->blockStun * (3 + wrongBlockLevel) / 3;
			this->_speed.x += data->pushBlock * -this->_dir;
		}
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
		if (
			action == ACTION_NEUTRAL_JUMP ||
			action == ACTION_FORWARD_JUMP ||
			action == ACTION_BACKWARD_JUMP ||
			action == ACTION_NEUTRAL_AIR_JUMP ||
			action == ACTION_FORWARD_AIR_JUMP ||
			action == ACTION_BACKWARD_AIR_JUMP
		) {
			this->_jumpsUsed++;
			this->_hasJumped = true;
		} else if (action == ACTION_NEUTRAL_HIGH_JUMP || action == ACTION_FORWARD_HIGH_JUMP || action == ACTION_BACKWARD_HIGH_JUMP) {
			this->_jumpsUsed += 2;
			this->_hasJumped = true;
		} else if (action >= ACTION_AIR_DASH_1 && action <= ACTION_AIR_DASH_9) {
			if (
				this->_action == ACTION_NEUTRAL_JUMP ||
				this->_action == ACTION_FORWARD_JUMP ||
				this->_action == ACTION_BACKWARD_JUMP ||
				this->_action == ACTION_NEUTRAL_AIR_JUMP ||
				this->_action == ACTION_FORWARD_AIR_JUMP ||
				this->_action == ACTION_BACKWARD_AIR_JUMP
			) {
				this->_jumpsUsed--;
				this->_hasJumped = false;
			}
		} else if (action >= ACTION_5N)
			this->_hasJumped = true;
		if (action != ACTION_AIR_HIT && action != ACTION_GROUND_LOW_HIT && action != ACTION_GROUND_HIGH_HIT) {
			this->_comboCtr = 0;
			this->_prorate = 1;
			this->_totalDamage = 0;
			this->_limit.fill(0);
		}
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
		if (action < 100)
			return false;
		if (!currentData->oFlag.cancelable)
			return false;
		if (!this->_hasHit && !currentData->dFlag.charaCancel)
			return false;
		if (currentData->oFlag.jumpCancelable && ((action >= ACTION_NEUTRAL_JUMP && action <= ACTION_BACKWARD_HIGH_JUMP) || (action >= ACTION_NEUTRAL_AIR_JUMP && action <= ACTION_BACKWARD_AIR_JUMP)))
			return true;
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
		if (data->oFlag.ultimate)
			return 7;
		if (data->oFlag.super)
			return 6;
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
		case ACTION_c28N:
		case ACTION_c46N:
		case ACTION_c64N:
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
		this->_specialInputs._44 = this->_check44Input();
		this->_specialInputs._66 = this->_check66Input();
		this->_specialInputs._27 = this->_check27Input();
		this->_specialInputs._28 = this->_check28Input();
		this->_specialInputs._29 = this->_check29Input();
		this->_specialInputs._c28 = this->_checkc28Input();
		this->_specialInputs._c46 = this->_checkc46Input();
		this->_specialInputs._c64 = this->_checkc64Input();
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

	bool ACharacter::_check44Input()
	{
		unsigned total = 0;
		bool found4 = false;
		bool foundOther = false;

		for (auto &input : this->_lastInputs) {
			if (found4 && foundOther && input.h < 0)
				return true;
			found4 |= input.h < 0;
			foundOther |= found4 && input.h >= 0;
			total += input.nbFrames;
			if (total > DASH_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check66Input()
	{
		unsigned total = 0;
		bool found6 = false;
		bool foundOther = false;

		for (auto &input : this->_lastInputs) {
			if (found6 && foundOther && input.h > 0)
				return true;
			found6 |= input.h > 0;
			foundOther |= found6 && input.h <= 0;
			total += input.nbFrames;
			if (total > DASH_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check27Input()
	{
		unsigned total = 0;
		bool found7 = false;

		for (auto &input : this->_lastInputs) {
			found7 |= input.v > 0 && input.h < 0;
			if (input.v < 0 && found7)
				return true;
			total += input.nbFrames;
			if (total > HJ_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check28Input()
	{
		unsigned total = 0;
		bool found8 = false;

		for (auto &input : this->_lastInputs) {
			found8 |= input.v > 0;
			if (found8 && input.v < 0)
				return true;
			total += input.nbFrames;
			if (total > HJ_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_check29Input()
	{
		unsigned total = 0;
		bool found9 = false;

		for (auto &input : this->_lastInputs) {
			found9 |= input.v > 0 && input.h > 0;
			if (found9 && input.v < 0)
				return true;
			total += input.nbFrames;
			if (total > HJ_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_checkc28Input()
	{
		unsigned timer2 = 0;
		unsigned timer = 0;
		unsigned total = 0;
		bool found8 = false;

		for (auto &input : this->_lastInputs) {
			found8 |= input.v > 0;
			if (input.v < 0) {
				timer += input.nbFrames;
				timer2 = 0;
			} else {
				timer2 += input.nbFrames;
				if (timer2 >= CHARGE_PART_BUFFER)
					timer = 0;
			}
			if (timer >= CHARGE_TIME && found8)
				return true;
			total += input.nbFrames;
			if ((!found8 || timer == 0) && total > CHARGE_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_checkc46Input()
	{
		unsigned timer2 = 0;
		unsigned timer = 0;
		unsigned total = 0;
		bool found6 = false;

		for (auto &input : this->_lastInputs) {
			found6 |= input.h > 0;
			if (input.h < 0) {
				timer += input.nbFrames;
				timer2 = 0;
			} else {
				timer2 += input.nbFrames;
				if (timer2 >= CHARGE_PART_BUFFER)
					timer = 0;
			}
			if (timer >= CHARGE_TIME && found6)
				return true;
			total += input.nbFrames;
			if ((!found6 || timer == 0) && total > CHARGE_BUFFER)
				break;
		}
		return false;
	}

	bool ACharacter::_checkc64Input()
	{
		unsigned timer2 = 0;
		unsigned timer = 0;
		unsigned total = 0;
		bool found4 = false;

		for (auto &input : this->_lastInputs) {
			found4 |= input.h < 0;
			if (input.h > 0) {
				timer += input.nbFrames;
				timer2 = 0;
			} else {
				timer2 += input.nbFrames;
				if (timer2 >= CHARGE_PART_BUFFER)
					timer = 0;
			}
			if (timer >= CHARGE_TIME && found4)
				return true;
			total += input.nbFrames;
			if ((!found4 || timer == 0) && total > CHARGE_BUFFER)
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

	bool ACharacter::_executeAirDashes(const InputStruct &input)
	{
		if (this->_specialInputs._44 && this->_startMove(ACTION_AIR_DASH_4))
			return true;
		if (this->_specialInputs._66 && this->_startMove(ACTION_AIR_DASH_6))
			return true;
		if (!input.d || (!input.verticalAxis && !input.horizontalAxis))
			return false;
		return  (input.verticalAxis > 0 && this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_AIR_DASH_9)) ||
			(input.verticalAxis > 0 && this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_AIR_DASH_7)) ||
			(input.verticalAxis > 0 &&                                          this->_startMove(ACTION_AIR_DASH_8)) ||
			(input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_AIR_DASH_3)) ||
			(input.verticalAxis < 0 && this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_AIR_DASH_1)) ||
			(input.verticalAxis < 0 &&                                          this->_startMove(ACTION_AIR_DASH_2)) ||
			(                          this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_AIR_DASH_6)) ||
			(                                                                   this->_startMove(ACTION_AIR_DASH_4));
	}

	bool ACharacter::_executeAirBlock(const InputStruct &input)
	{
		if (input.n && input.horizontalAxis * this->_dir < 0) {
			if (this->_action == ACTION_AIR_NEUTRAL_BLOCK || this->_startMove(ACTION_AIR_NEUTRAL_BLOCK))
				return true;
		}
		if (input.m && input.horizontalAxis * this->_dir < 0) {
			if (
				this->_action == ACTION_AIR_MATTER_BLOCK || this->_startMove(ACTION_AIR_MATTER_BLOCK) ||
				this->_action == ACTION_AIR_NEUTRAL_BLOCK || this->_startMove(ACTION_AIR_NEUTRAL_BLOCK)
			)
				return true;
		}
		if (input.s && input.horizontalAxis * this->_dir < 0) {
			if (
				this->_action == ACTION_AIR_SPIRIT_BLOCK || this->_startMove(ACTION_AIR_SPIRIT_BLOCK) ||
				this->_action == ACTION_AIR_NEUTRAL_BLOCK || this->_startMove(ACTION_AIR_NEUTRAL_BLOCK)
			)
				return true;
		}
		if (input.v && input.horizontalAxis * this->_dir < 0) {
			if (
				this->_action == ACTION_AIR_VOID_BLOCK || this->_startMove(ACTION_AIR_VOID_BLOCK) ||
				this->_action == ACTION_AIR_NEUTRAL_BLOCK || this->_startMove(ACTION_AIR_NEUTRAL_BLOCK)
			)
				return true;
		}
		return false;
	}

	bool ACharacter::_executeAirJump(const InputStruct &input)
	{
		if (this->_hasJumped)
			return false;
		if (input.verticalAxis <= 0)
			return false;
		if (input.horizontalAxis * this->_dir > 0 && this->_startMove(ACTION_FORWARD_AIR_JUMP))
			return true;
		if (input.horizontalAxis * this->_dir < 0 && this->_startMove(ACTION_BACKWARD_AIR_JUMP))
			return true;
		return this->_startMove(ACTION_NEUTRAL_AIR_JUMP);
	}

	bool ACharacter::_executeGroundDashes(const InputStruct &input)
	{
		if (this->_specialInputs._44 && this->_startMove(ACTION_BACKWARD_DASH))
			return true;
		if (this->_specialInputs._66 && this->_startMove(ACTION_FORWARD_DASH))
			return true;
		if (!input.d || !input.horizontalAxis)
			return false;
		return this->_startMove(this->_dir * input.horizontalAxis > 0 ? ACTION_FORWARD_DASH : ACTION_BACKWARD_DASH);
	}

	bool ACharacter::_executeGroundBlock(const InputStruct &input)
	{
		if (input.n && input.horizontalAxis * this->_dir < 0) {
			auto move = input.verticalAxis < 0 ? ACTION_GROUND_LOW_NEUTRAL_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_BLOCK;

			if (this->_action == move || this->_startMove(move))
				return true;
		}
		if (input.m && input.horizontalAxis * this->_dir < 0) {
			auto move = input.verticalAxis < 0 ? ACTION_GROUND_LOW_MATTER_BLOCK : ACTION_GROUND_HIGH_MATTER_BLOCK;
			auto move2 = input.verticalAxis < 0 ? ACTION_GROUND_LOW_NEUTRAL_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_BLOCK;

			if (
				this->_action == move  || this->_startMove(move) ||
				this->_action == move2 || this->_startMove(move2)
			)
				return true;
		}
		if (input.s && input.horizontalAxis * this->_dir < 0) {
			auto move = input.verticalAxis < 0 ? ACTION_GROUND_LOW_SPIRIT_BLOCK : ACTION_GROUND_HIGH_SPIRIT_BLOCK;
			auto move2 = input.verticalAxis < 0 ? ACTION_GROUND_LOW_NEUTRAL_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_BLOCK;

			if (
				this->_action == move  || this->_startMove(move) ||
				this->_action == move2 || this->_startMove(move2)
			)
				return true;
		}
		if (input.v && input.horizontalAxis * this->_dir < 0) {
			auto move = input.verticalAxis < 0 ? ACTION_GROUND_LOW_VOID_BLOCK : ACTION_GROUND_HIGH_VOID_BLOCK;
			auto move2 = input.verticalAxis < 0 ? ACTION_GROUND_LOW_NEUTRAL_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_BLOCK;

			if (
				this->_action == move  || this->_startMove(move) ||
				this->_action == move2 || this->_startMove(move2)
			)
				return true;
		}
		return false;
	}

	bool ACharacter::_executeGroundJump(const InputStruct &input)
	{
		if (this->_specialInputs._29 && this->_startMove(ACTION_FORWARD_HIGH_JUMP))
			return true;
		if (this->_specialInputs._27 && this->_startMove(ACTION_BACKWARD_HIGH_JUMP))
			return true;
		if (this->_specialInputs._28 && this->_startMove(ACTION_NEUTRAL_HIGH_JUMP))
			return true;
		if (input.verticalAxis <= 0)
			return false;
		if (input.d) {
			if (input.horizontalAxis * this->_dir > 0 && this->_startMove(ACTION_FORWARD_HIGH_JUMP))
				return true;
			if (input.horizontalAxis * this->_dir < 0 && this->_startMove(ACTION_BACKWARD_HIGH_JUMP))
				return true;
			if (input.horizontalAxis == 0 && this->_startMove(ACTION_NEUTRAL_HIGH_JUMP))
				return true;
		}
		if (input.horizontalAxis * this->_dir > 0 && this->_startMove(ACTION_FORWARD_JUMP))
			return true;
		if (input.horizontalAxis * this->_dir < 0 && this->_startMove(ACTION_BACKWARD_JUMP))
			return true;
		return this->_startMove(ACTION_NEUTRAL_JUMP);
	}

	bool ACharacter::_executeCrouch(const InputStruct &input)
	{
		if (input.verticalAxis >= 0)
			return false;
		this->_startMove(ACTION_CROUCHING);
		return this->_action == ACTION_CROUCHING || this->_action == ACTION_CROUCH;
	}

	bool ACharacter::_executeWalking(const InputStruct &input)
	{
		if (!input.horizontalAxis)
			return false;
		this->_startMove(std::copysign(1, input.horizontalAxis) == std::copysign(1, this->_dir) ? ACTION_WALK_FORWARD : ACTION_WALK_BACKWARD);
		return true;
	}

	bool ACharacter::_executeAirTech(const InputStruct &input)
	{
		if (!input.d & !input.n & !input.v & !input.m & !input.a & !input.s)
			return false;
		return  (input.verticalAxis > 0 &&                this->_startMove(ACTION_UP_AIR_TECH)) ||
			(input.verticalAxis < 0 &&                this->_startMove(ACTION_DOWN_AIR_TECH)) ||
			(this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_FORWARD_AIR_TECH)) ||
			(this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_BACKWARD_AIR_TECH));
	}

	bool ACharacter::hits(IObject &other) const
	{
		for (auto limit : this->_limit)
			if (limit >= 100)
				return false;
		return AObject::hits(other);
	}

	void ACharacter::postUpdate()
	{
		if (this->_position.x < 0)
			this->_position.x = 0;
		else if (this->_position.x > 1000)
			this->_position.x = 1000;
		if (this->_position.y < 0)
			this->_position.y = 0;
		else if (this->_position.y > 1000)
			this->_position.y = 1000;


#ifdef _DEBUG
		auto data = this->getCurrentFrameData();
		char buffer[4096];

		sprintf(
			buffer,
			"PositionX: %f\n"
			"PositionY: %f\n"
			"SpeedX: %f\n"
			"SpeedY: %f\n"
			"GravityX: %f\n"
			"GravityY: %f\n"
			"BlockStun: %i\n"
			"JumpUsed: %i/%i\n"
			"AirDashUsed: %i/%i\n"
			"Jumped: %s\n"
			"Restand: %s\n"
			"Action: %i\n"
			"ActionBlock: %i/%llu\n"
			"Animation: %i/%llu\n"
			"AnimationCtr: %i/%i\n"
			"Hp: %i/%i\n"
			"Rotation: %f\n"
			"HasHit: %s\n"
			"Direction: %s\n"
			"Dir: %f\n"
			"cornerPriority: %i\n"
			"justGotCorner: %s\n"
			"comboCtr: %u\n"
			"prorate: %f\n"
			"totalDamage: %u\n"
			"neutralLimit: %u\n"
			"voidLimit: %u\n"
			"matterLimit: %u\n"
			"spiritLimit: %u\n"
			"BaseGravityX: %f\n"
			"BaseGravityY: %f",
			this->_position.x,
			this->_position.y,
			this->_speed.x,
			this->_speed.y,
			this->_gravity.x,
			this->_gravity.y,
			this->_blockStun,
			this->_jumpsUsed,
			this->_maxJumps,
			this->_airDashesUsed,
			this->_maxAirDashes,
			this->_hasJumped ? "true" : "false",
			this->_restand ? "true" : "false",
			this->_action,
			this->_actionBlock,
			this->_moves.at(this->_action).size(),
			this->_animation,
			this->_moves.at(this->_action)[this->_actionBlock].size(),
			this->_animationCtr,
			this->_moves.at(this->_action)[this->_actionBlock][this->_animation].duration,
			this->_hp,
			this->_baseHp,
			this->_rotation,
			this->_hasHit ? "true" : "false",
			this->_direction ? "right" : "left",
			this->_dir,
			this->_cornerPriority,
			this->_justGotCorner ? "true" : "false",
			this->_comboCtr,
			this->_prorate,
			this->_totalDamage,
			this->_limit[0],
			this->_limit[1],
			this->_limit[2],
			this->_limit[3],
			this->_baseGravity.x,
			this->_baseGravity.y
		);
		this->_text.setString(buffer);
		this->_text.setPosition({static_cast<float>(this->_team * 850), -450});


		sprintf(
			buffer,
			"specialMarker: %u\n"
			"blockStun: %u\n"
			"hitStun: %u\n"
			"prorate: %3.f\n"
			"neutralLimit: %u\n"
			"voidLimit: %u\n"
			"spiritLimit: %u\n"
			"matterLimit: %u\n"
			"pushBack: %u\n"
			"pushBlock: %u\n"
			"subObjectSpawn: %u\n"
			"manaGain: %u\n"
			"manaCost: %u\n"
			"hitStop: %u\n"
			"damage: %u\n",
			data->specialMarker,
			data->blockStun,
			data->hitStun,
			data->prorate,
			data->neutralLimit,
			data->voidLimit,
			data->spiritLimit,
			data->matterLimit,
			data->pushBack,
			data->pushBlock,
			data->subObjectSpawn,
			data->manaGain,
			data->manaCost,
			data->hitStop,
			data->damage
		);
		for (unsigned tmp = data->dFlag.flags, i = 0; tmp; tmp >>= 1, i++)
			if (tmp & 1) {
				strcat(buffer, dFlags[i]);
				strcat(buffer, "\n");
			}
		for (unsigned tmp = data->oFlag.flags, i = 0; tmp; tmp >>= 1, i++)
			if (tmp & 1) {
				strcat(buffer, oFlags[i]);
				strcat(buffer, "\n");
			}
		this->_text2.setString(buffer);
		this->_text2.setPosition({static_cast<float>(this->_team * 600 + 150), -450});
#endif
	}
}
