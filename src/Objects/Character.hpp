//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_CHARACTER_HPP
#define BATTLE_CHARACTER_HPP


#include <list>
#include "Object.hpp"
#include "../Inputs/IInput.hpp"
#include "../Data/Color.hpp"
#include "Projectile.hpp"

namespace Battle
{
	enum CharacterActions {
		/*  0  */ ACTION_IDLE,
		/*  1  */ ACTION_CROUCHING,
		/*  2  */ ACTION_CROUCH,
		/*  3  */ ACTION_STANDING_UP,
		/*  4  */ ACTION_WALK_FORWARD,
		/*  5  */ ACTION_WALK_BACKWARD,
		/*  6  */ ACTION_FORWARD_DASH,
		/*  7  */ ACTION_BACKWARD_DASH,
		/*  8  */ ACTION_NEUTRAL_JUMP,
		/*  9  */ ACTION_FORWARD_JUMP,
		/* 10  */ ACTION_BACKWARD_JUMP,
		/* 11  */ ACTION_NEUTRAL_HIGH_JUMP,
		/* 12  */ ACTION_FORWARD_HIGH_JUMP,
		/* 13  */ ACTION_BACKWARD_HIGH_JUMP,
		/* 14  */ ACTION_FALLING,
		/* 15  */ ACTION_LANDING,
		/* 16  */ ACTION_AIR_DASH_1,
		/* 17  */ ACTION_AIR_DASH_2,
		/* 18  */ ACTION_AIR_DASH_3,
		/* 19  */ ACTION_AIR_DASH_4,
		/* 20  */ ACTION_AIR_DASH_6,
		/* 21  */ ACTION_AIR_DASH_7,
		/* 22  */ ACTION_AIR_DASH_8,
		/* 23  */ ACTION_AIR_DASH_9,
		/* 24  */ ACTION_AIR_TRANSFORM,
		/* 25  */ ACTION_GROUND_HIGH_NEUTRAL_BLOCK,
		/* 26  */ ACTION_GROUND_HIGH_SPIRIT_PARRY,
		/* 27  */ ACTION_GROUND_HIGH_MATTER_PARRY,
		/* 28  */ ACTION_GROUND_HIGH_VOID_PARRY,
		/* 29  */ ACTION_GROUND_HIGH_HIT,
		/* 30  */ ACTION_GROUND_LOW_NEUTRAL_BLOCK,
		/* 31  */ ACTION_GROUND_LOW_SPIRIT_PARRY,
		/* 32  */ ACTION_GROUND_LOW_MATTER_PARRY,
		/* 33  */ ACTION_GROUND_LOW_VOID_PARRY,
		/* 34  */ ACTION_GROUND_LOW_HIT,
		/* 35  */ ACTION_AIR_NEUTRAL_BLOCK,
		/* 36  */ ACTION_AIR_SPIRIT_PARRY,
		/* 37  */ ACTION_AIR_MATTER_PARRY,
		/* 38  */ ACTION_AIR_VOID_PARRY,
		/* 39  */ ACTION_AIR_HIT,
		/* 40  */ ACTION_BEING_KNOCKED_DOWN,
		/* 41  */ ACTION_KNOCKED_DOWN,
		/* 42  */ ACTION_NEUTRAL_TECH,
		/* 43  */ ACTION_BACKWARD_TECH,
		/* 44  */ ACTION_FORWARD_TECH,
		/* 45  */ ACTION_FALLING_TECH,
		/* 46  */ ACTION_UP_AIR_TECH,
		/* 47  */ ACTION_DOWN_AIR_TECH,
		/* 48  */ ACTION_FORWARD_AIR_TECH,
		/* 49  */ ACTION_BACKWARD_AIR_TECH,
		/* 50  */ ACTION_AIR_TECH_LANDING_LAG,
		/* 51  */ ACTION_UNTRANSFORM,
		/* 52  */ ACTION_GROUND_SLAM,
		/* 53  */ ACTION_WALL_SLAM,
		/* 54  */ ACTION_HARD_LAND,
		/* 55  */ ACTION_NEUTRAL_AIR_JUMP,
		/* 56  */ ACTION_FORWARD_AIR_JUMP,
		/* 57  */ ACTION_BACKWARD_AIR_JUMP,
		/* 58  */ ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK,
		/* 59  */ ACTION_GROUND_LOW_NEUTRAL_WRONG_BLOCK,
		/* 60  */ ACTION_AIR_NEUTRAL_WRONG_BLOCK,
		/* 61  */ ACTION_GROUND_HIGH_NEUTRAL_PARRY,
		/* 62  */ ACTION_GROUND_LOW_NEUTRAL_PARRY,
		/* 63  */ ACTION_AIR_NEUTRAL_PARRY,

		/* 100 */ ACTION_5N = 100,
		/* 101 */ ACTION_6N,
		/* 102 */ ACTION_8N,
		/* 103 */ ACTION_3N,
		/* 104 */ ACTION_2N,
		/* 105 */ ACTION_214N,
		/* 106 */ ACTION_236N,
		/* 107 */ ACTION_421N,
		/* 107 */ ACTION_NEUTRAL_OVERDRIVE = ACTION_421N,
		/* 108 */ ACTION_623N,
		/* 109 */ ACTION_41236N,
		/* 110 */ ACTION_63214N,
		/* 111 */ ACTION_6321469874N,
		/* 112 */ ACTION_j5N,
		/* 113 */ ACTION_j6N,
		/* 114 */ ACTION_j8N,
		/* 115 */ ACTION_j3N,
		/* 116 */ ACTION_j2N,
		/* 117 */ ACTION_j214N,
		/* 118 */ ACTION_j236N,
		/* 119 */ ACTION_j421N,
		/* 119 */ ACTION_NEUTRAL_AIR_OVERDRIVE = ACTION_j421N,
		/* 120 */ ACTION_j623N,
		/* 121 */ ACTION_j41236N,
		/* 122 */ ACTION_j63214N,
		/* 123 */ ACTION_j6321469874N,
		/* 124 */ ACTION_t5N,
		/* 125 */ ACTION_t6N,
		/* 126 */ ACTION_t8N,
		/* 127 */ ACTION_t2N,
		/* 128 */ ACTION_c28N,
		/* 129 */ ACTION_c46N,
		/* 130 */ ACTION_c64N,
		/* 131 */ ACTION_NEUTRAL_ROMAN_CANCEL,
		/* 132 */ ACTION_NEUTRAL_AIR_ROMAN_CANCEL,

		/* 150 */ ACTION_5M = 150,
		/* 151 */ ACTION_6M,
		/* 152 */ ACTION_8M,
		/* 153 */ ACTION_3M,
		/* 154 */ ACTION_2M,
		/* 155 */ ACTION_214M,
		/* 156 */ ACTION_236M,
		/* 157 */ ACTION_421M,
		/* 157 */ ACTION_MATTER_OVERDRIVE = ACTION_421M,
		/* 158 */ ACTION_623M,
		/* 159 */ ACTION_41236M,
		/* 160 */ ACTION_63214M,
		/* 161 */ ACTION_6321469874M,
		/* 162 */ ACTION_j5M,
		/* 163 */ ACTION_j6M,
		/* 164 */ ACTION_j8M,
		/* 165 */ ACTION_j3M,
		/* 166 */ ACTION_j2M,
		/* 167 */ ACTION_j214M,
		/* 168 */ ACTION_j236M,
		/* 169 */ ACTION_j421M,
		/* 169 */ ACTION_MATTER_AIR_OVERDRIVE = ACTION_j421M,
		/* 170 */ ACTION_j623M,
		/* 171 */ ACTION_j41236M,
		/* 172 */ ACTION_j63214M,
		/* 173 */ ACTION_j6321469874M,
		/* 174 */ ACTION_t5M,
		/* 175 */ ACTION_t6M,
		/* 176 */ ACTION_t8M,
		/* 177 */ ACTION_t2M,
		/* 178 */ ACTION_c28M,
		/* 179 */ ACTION_c46M,
		/* 180 */ ACTION_c64M,
		/* 181 */ ACTION_MATTER_ROMAN_CANCEL,
		/* 182 */ ACTION_MATTER_AIR_ROMAN_CANCEL,

		/* 200 */ ACTION_5S = 200,
		/* 201 */ ACTION_6S,
		/* 202 */ ACTION_8S,
		/* 203 */ ACTION_3S,
		/* 204 */ ACTION_2S,
		/* 205 */ ACTION_214S,
		/* 206 */ ACTION_236S,
		/* 207 */ ACTION_421S,
		/* 207 */ ACTION_SPIRIT_OVERDRIVE = ACTION_421S,
		/* 208 */ ACTION_623S,
		/* 209 */ ACTION_41236S,
		/* 210 */ ACTION_63214S,
		/* 211 */ ACTION_6321469874S,
		/* 212 */ ACTION_j5S,
		/* 213 */ ACTION_j6S,
		/* 214 */ ACTION_j8S,
		/* 215 */ ACTION_j3S,
		/* 216 */ ACTION_j2S,
		/* 217 */ ACTION_j214S,
		/* 218 */ ACTION_j236S,
		/* 219 */ ACTION_j421S,
		/* 219 */ ACTION_SPIRIT_AIR_OVERDRIVE = ACTION_j421S,
		/* 220 */ ACTION_j623S,
		/* 221 */ ACTION_j41236S,
		/* 222 */ ACTION_j63214S,
		/* 223 */ ACTION_j6321469874S,
		/* 224 */ ACTION_t5S,
		/* 225 */ ACTION_t6S,
		/* 226 */ ACTION_t8S,
		/* 227 */ ACTION_t2S,
		/* 228 */ ACTION_c28S,
		/* 229 */ ACTION_c46S,
		/* 230 */ ACTION_c64S,
		/* 231 */ ACTION_SPIRIT_ROMAN_CANCEL,
		/* 232 */ ACTION_SPIRIT_AIR_ROMAN_CANCEL,

		/* 250 */ ACTION_5V = 250,
		/* 251 */ ACTION_6V,
		/* 252 */ ACTION_8V,
		/* 253 */ ACTION_3V,
		/* 254 */ ACTION_2V,
		/* 255 */ ACTION_214V,
		/* 256 */ ACTION_236V,
		/* 257 */ ACTION_421V,
		/* 257 */ ACTION_VOID_OVERDRIVE = ACTION_421V,
		/* 258 */ ACTION_623V,
		/* 259 */ ACTION_41236V,
		/* 260 */ ACTION_63214V,
		/* 261 */ ACTION_6321469874V,
		/* 262 */ ACTION_j5V,
		/* 263 */ ACTION_j6V,
		/* 264 */ ACTION_j8V,
		/* 265 */ ACTION_j3V,
		/* 266 */ ACTION_j2V,
		/* 267 */ ACTION_j214V,
		/* 268 */ ACTION_j236V,
		/* 269 */ ACTION_j421V,
		/* 269 */ ACTION_VOID_AIR_OVERDRIVE = ACTION_j421V,
		/* 270 */ ACTION_j623V,
		/* 271 */ ACTION_j41236V,
		/* 272 */ ACTION_j63214V,
		/* 273 */ ACTION_j6321469874V,
		/* 274 */ ACTION_t5V,
		/* 275 */ ACTION_t6V,
		/* 276 */ ACTION_t8V,
		/* 277 */ ACTION_t2V,
		/* 278 */ ACTION_c28V,
		/* 279 */ ACTION_c46V,
		/* 280 */ ACTION_c64V,
		/* 281 */ ACTION_VOID_ROMAN_CANCEL,
		/* 282 */ ACTION_VOID_AIR_ROMAN_CANCEL,

		/* 300 */ ACTION_5A = 300,
		/* 301 */ ACTION_6A,
		/* 302 */ ACTION_8A,
		/* 303 */ ACTION_3A,
		/* 304 */ ACTION_2A,
		/* 305 */ ACTION_214A,
		/* 306 */ ACTION_236A,
		/* 307 */ ACTION_421A,
		/* 308 */ ACTION_623A,
		/* 309 */ ACTION_41236A,
		/* 310 */ ACTION_63214A,
		/* 311 */ ACTION_6321469874A,
		/* 312 */ ACTION_j5A,
		/* 313 */ ACTION_j6A,
		/* 314 */ ACTION_j8A,
		/* 315 */ ACTION_j3A,
		/* 316 */ ACTION_j2A,
		/* 317 */ ACTION_j214A,
		/* 318 */ ACTION_j236A,
		/* 319 */ ACTION_j421A,
		/* 320 */ ACTION_j623A,
		/* 321 */ ACTION_j41236A,
		/* 322 */ ACTION_j63214A,
		/* 323 */ ACTION_j6321469874A,
		/* 324 */ ACTION_t5A,
		/* 325 */ ACTION_t6A,
		/* 326 */ ACTION_t8A,
		/* 327 */ ACTION_t2A,
		/* 328 */ ACTION_c28A,
		/* 329 */ ACTION_c46A,
		/* 330 */ ACTION_c64A,

		/* 350 */ ACTION_214D = 350,
		/* 351 */ ACTION_236D,
		/* 352 */ ACTION_421D,
		/* 353 */ ACTION_623D,
		/* 354 */ ACTION_41236D,
		/* 355 */ ACTION_63214D,
		/* 356 */ ACTION_6321469874D,
		/* 357 */ ACTION_j214D,
		/* 358 */ ACTION_j236D,
		/* 359 */ ACTION_j421D,
		/* 360 */ ACTION_j623D,
		/* 361 */ ACTION_j41236D,
		/* 362 */ ACTION_j63214D,
		/* 363 */ ACTION_j6321469874D,
		/* 364 */ ACTION_t5D,
		/* 365 */ ACTION_t6D,
		/* 366 */ ACTION_t8D,
		/* 367 */ ACTION_t2D,

		/* 400 */ ACTION_WIN_MATCH1 = 400,
		/* 401 */ ACTION_WIN_MATCH2,
		/* 402 */ ACTION_WIN_MATCH3,
		/* 403 */ ACTION_WIN_MATCH4,
	};

	extern const std::map<CharacterActions, std::string> actionNames;

	class Character : public Object {
	protected:
		enum GroundTech {
			GROUNDTECH_NONE,
			GROUNDTECH_FORWARD,
			GROUNDTECH_BACKWARD,
			GROUNDTECH_RANDOM
		};
		enum AirTech {
			AIRTECH_NONE,
			AIRTECH_FORWARD,
			AIRTECH_BACKWARD,
			AIRTECH_UP,
			AIRTECH_DOWN,
			AIRTECH_RANDOM
		};
		enum DummyState {
			DUMMYSTATE_STANDING,
			DUMMYSTATE_JUMP,
			DUMMYSTATE_HIGH_JUMP,
			DUMMYSTATE_CROUCH
		};
		enum ForceBlock {
			NO_BLOCK,
			ALL_RIGHT_BLOCK,
			ALL_WRONG_BLOCK,
			HIGH_BLOCK,
			LOW_BLOCK,
			RANDOM_HEIGHT_BLOCK,
			BLOCK_1ST_HIT,
			BLOCK_AFTER_HIT,
			RANDOM_BLOCK = 8
		};
		struct LastInput {
			unsigned nbFrames : 23;
			bool n : 1;
			bool m : 1;
			bool s : 1;
			bool o : 1;
			bool d : 1;
			char h : 2;
			char v : 2;
		};
#pragma pack(push, 1)
		struct Data {
			unsigned _limit[4];
			unsigned _subObjects[128];
			unsigned _odCooldown;
			unsigned _blockStun;
			unsigned _jumpsUsed;
			unsigned _airDashesUsed;
			unsigned _comboCtr;
			unsigned _totalDamage;
			unsigned _guardCooldown;
			unsigned _guardBar;
			unsigned _guardRegenCd;
			bool _counter;
			float _prorate;
			bool _atkDisabled;
			bool _inputDisabled;
			bool _hasJumped;
			bool _restand;
			bool _justGotCorner;
			float _regen;
			float _voidMana;
			float _spiritMana;
			float _matterMana;
			unsigned _specialInputs;
			unsigned _nbLastInputs;
		};
#pragma pack(pop)
		union SpecialInputs {
			unsigned short _value = 0;
			struct {
				bool _22: 1;
				bool _44: 1;
				bool _66: 1;
				bool _27: 1;
				bool _28: 1;
				bool _29: 1;
				bool _c28n: 1;
				bool _c28m: 1;
				bool _c28s: 1;
				bool _c28v: 1;
				bool _c28d: 1;
				bool _c46n: 1;
				bool _c46m: 1;
				bool _c46s: 1;
				bool _c46v: 1;
				bool _c46d: 1;
				bool _c64n: 1;
				bool _c64m: 1;
				bool _c64s: 1;
				bool _c64v: 1;
				bool _c64d: 1;
				bool _236n: 1;
				bool _236m: 1;
				bool _236s: 1;
				bool _236v: 1;
				bool _236d: 1;
				bool _214n: 1;
				bool _214m: 1;
				bool _214s: 1;
				bool _214v: 1;
				bool _214d: 1;
				bool _623n: 1;
				bool _623m: 1;
				bool _623s: 1;
				bool _623v: 1;
				bool _623d: 1;
				bool _421n: 1;
				bool _421m: 1;
				bool _421s: 1;
				bool _421v: 1;
				bool _421d: 1;
				bool _624n: 1;
				bool _624m: 1;
				bool _624s: 1;
				bool _624v: 1;
				bool _624d: 1;
				bool _426n: 1;
				bool _426m: 1;
				bool _426s: 1;
				bool _426v: 1;
				bool _426d: 1;
				bool _6314n: 1;
				bool _6314m: 1;
				bool _6314s: 1;
				bool _6314v: 1;
				bool _6314d: 1;
				bool _4136n: 1;
				bool _4136m: 1;
				bool _4136s: 1;
				bool _4136v: 1;
				bool _4136d: 1;
				bool _624684n: 1;
				bool _624684m: 1;
				bool _624684s: 1;
				bool _624684v: 1;
				bool _624684d: 1;
				bool _6314684n: 1;
				bool _6314684m: 1;
				bool _6314684s: 1;
				bool _6314684v: 1;
				bool _6314684d: 1;
				bool _6246974n: 1;
				bool _6246974m: 1;
				bool _6246974s: 1;
				bool _6246974v: 1;
				bool _6246974d: 1;
				bool _63146974n: 1;
				bool _63146974m: 1;
				bool _63146974s: 1;
				bool _63146974v: 1;
				bool _63146974d: 1;
			};
		};

		// Game State
		std::list<LastInput> _lastInputs;
		std::array<std::pair<unsigned, std::shared_ptr<IObject>>, 128> _subobjects;
		std::array<unsigned, 4> _limit;
		InputStruct _inputBuffer = {0, 0, 0, 0, 0, 0, 0, 0, 0};
		unsigned _barMaxOdCooldown = 0;
		unsigned _odCooldown = 0;
		unsigned _blockStun = 0;
		unsigned _jumpsUsed = 0;
		unsigned _airDashesUsed = 0;
		unsigned _comboCtr = 0;
		unsigned _totalDamage = 0;
		unsigned _guardCooldown = 0;
		unsigned _guardBar = 0;
		unsigned _guardRegenCd = 0;
		bool _counter = false;
		float _prorate = 1;
		bool _atkDisabled = false;
		bool _inputDisabled = false;
		bool _hasJumped = false;
		bool _restand = false;
		bool _justGotCorner = false;
		float _regen;
		float _voidMana;
		float _spiritMana;
		float _matterMana;
		SpecialInputs _specialInputs;

		// Non Game State
		sf::Text _text;
		sf::Text _text2;
		Character *_opponent;
		std::map<unsigned, std::vector<std::vector<FrameData>>> _subObjectsData;
		std::shared_ptr<IInput> _input;
		unsigned _maxOdCooldown;
		unsigned _maxJumps = 0;
		unsigned _maxAirDashes = 0;
		unsigned _voidManaMax;
		unsigned _spiritManaMax;
		unsigned _matterManaMax;
		unsigned _maxGuardCooldown = 0;
		unsigned _maxGuardBar = 0;

		//Practice
		GroundTech _dummyGroundTech = GROUNDTECH_NONE;
		AirTech _dummyAirTech = AIRTECH_NONE;
		DummyState _dummyState = DUMMYSTATE_STANDING;
		unsigned char _forceBlock = 0;
		unsigned char _counterHit = 0;

		virtual unsigned _getReversalAction();
		virtual void _parryEffect(Object *other);
		virtual void _processGuardLoss(bool wrongBlock);
		virtual bool _executeAirDashes(const InputStruct &input);
		virtual bool _executeAirParry(const InputStruct &input);
		virtual bool _executeAirJump(const InputStruct &input);
		virtual bool _executeGroundDashes(const InputStruct &input);
		virtual bool _executeGroundParry(const InputStruct &input);
		virtual bool _executeGroundJump(const InputStruct &input);
		virtual bool _executeCrouch(const InputStruct &input);
		virtual bool _executeWalking(const InputStruct &input);
		virtual bool _executeAirTech(const InputStruct &input);
		virtual void _processInput(InputStruct input);
		virtual bool _executeAirborneMoves(const InputStruct &input);
		virtual bool _executeGroundMoves(const InputStruct &input);
		virtual bool _isBlocking() const;
		virtual bool _canCancel(unsigned int action);
		virtual unsigned char _checkHitPos(const Object *other) const;
		virtual void _blockMove(Object *other, const FrameData &data);
		virtual void _getHitByMove(Object *other, const FrameData &data);
		virtual void _processWallSlams();
		virtual void _processGroundSlams();
		virtual void _calculateCornerPriority();
		virtual InputStruct _getInputs();
		virtual std::pair<unsigned, std::shared_ptr<IObject>> _spawnSubobject(unsigned id, bool needRegister = true);

		static bool isBlockingAction(unsigned action);
		static bool isParryAction(unsigned action);
		static bool isOverdriveAction(unsigned action);
		static bool isRomanCancelAction(unsigned action);

		void _applyMoveAttributes() override;
		void _forceStartMove(unsigned action) override;
		void _onMoveEnd(const FrameData &lastData) override;
		bool _canStartMove(unsigned action, const FrameData &data) override;
		void _checkPlatforms(Vector2f oldPos) override;
		bool _isOnPlatform() const override;

		void _removeSubobjects();
		void _checkSpecialInputs();
		void _clearLastInputs();
		bool _check22Input();
		bool _check44Input();
		bool _check66Input();
		bool _check27Input();
		bool _check28Input();
		bool _check29Input();
		bool _checkc28Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _checkc46Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _checkc64Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _check236Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _check214Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _check623Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _check421Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _check624Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _check426Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _check6314Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _check4136Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _check624684Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _check6314684Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _check6246974Input(const std::function<bool (const LastInput &)> &atkInput);
		bool _check63146974Input(const std::function<bool (const LastInput &)> &atkInput);

	public:
		std::string name;
		bool showAttributes = false;

		Character() = default;
		Character(const std::string &frameData, const std::string &suobjFrameData, const std::pair<std::vector<Color>, std::vector<Color>> &palette, std::shared_ptr<IInput> input);
		~Character() override = default;
		void setOpponent(Character *opponent);
		bool hits(const IObject &other) const override;
		void hit(IObject &other, const FrameData *data) override;
		void getHit(IObject &other, const FrameData *data) override;
		void render() const override;
		void update() override;
		InputStruct updateInputs();
		void init(
			bool side,
			unsigned short maxHp,
			unsigned char maxJumps,
			unsigned char maxAirDash,
			unsigned maxMMana,
			unsigned maxVMana,
			unsigned maxSMana,
			float manaRegen,
			unsigned maxGuardBar,
			unsigned maxGuardCooldown,
			unsigned odCd,
			float groundDrag,
			Vector2f airDrag,
			Vector2f gravity
		);
		void consumeEvent(const sf::Event &event);
		void postUpdate();
		std::shared_ptr<IInput> &getInput();
		const std::shared_ptr<IInput> &getInput() const;
		const std::map<unsigned, std::vector<std::vector<FrameData>>> &getFrameData();
		virtual int getAttackTier(unsigned int action) const;
		virtual void setAttacksDisabled(bool disabled);
		virtual void disableInputs(bool disabled);
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		void resolveSubObjects(const BattleManager &manager);
		unsigned int getClassId() const override;

		friend class PracticeBattleManager;
		friend class PracticeInGame;
		friend class BattleManager;
	};
}


#endif //BATTLE_CHARACTER_HPP
