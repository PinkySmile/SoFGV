//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_CHARACTER_HPP
#define SOFGV_CHARACTER_HPP


#include <list>
#include "Object.hpp"
#include "Inputs/IInput.hpp"
#include "Data/Color.hpp"

#ifdef _MSC_VER
#ifdef MYDLL_EXPORTS
#define MYDLL_API __declspec(dllexport)
#else
#define MYDLL_API __declspec(dllimport)
#endif
#else
#define MYDLL_API
#endif

namespace SpiralOfFate
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
		/* 107 */ ACTION_NEUTRAL_OVERDRIVE,
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
		/* 119 */ ACTION_NEUTRAL_AIR_OVERDRIVE,
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
		/* 131 */ ACTION_ROMAN_CANCEL,
		/* 132 */ ACTION_AIR_ROMAN_CANCEL,
		/* 133 */ ACTION_421N,
		/* 134 */ ACTION_j421N,

		/* 150 */ ACTION_5M = 150,
		/* 151 */ ACTION_6M,
		/* 152 */ ACTION_8M,
		/* 153 */ ACTION_3M,
		/* 154 */ ACTION_2M,
		/* 155 */ ACTION_214M,
		/* 156 */ ACTION_236M,
		/* 157 */ ACTION_MATTER_OVERDRIVE,
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
		/* 169 */ ACTION_MATTER_AIR_OVERDRIVE,
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
		/* 181 */ ACTION_181,
		/* 182 */ ACTION_182,
		/* 183 */ ACTION_421M,
		/* 184 */ ACTION_j421M,

		/* 200 */ ACTION_5S = 200,
		/* 201 */ ACTION_6S,
		/* 202 */ ACTION_8S,
		/* 203 */ ACTION_3S,
		/* 204 */ ACTION_2S,
		/* 205 */ ACTION_214S,
		/* 206 */ ACTION_236S,
		/* 207 */ ACTION_SPIRIT_OVERDRIVE,
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
		/* 219 */ ACTION_SPIRIT_AIR_OVERDRIVE,
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
		/* 231 */ ACTION_231,
		/* 232 */ ACTION_232,
		/* 233 */ ACTION_421S,
		/* 234 */ ACTION_j421S,

		/* 250 */ ACTION_5V = 250,
		/* 251 */ ACTION_6V,
		/* 252 */ ACTION_8V,
		/* 253 */ ACTION_3V,
		/* 254 */ ACTION_2V,
		/* 255 */ ACTION_214V,
		/* 256 */ ACTION_236V,
		/* 257 */ ACTION_VOID_OVERDRIVE,
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
		/* 269 */ ACTION_VOID_AIR_OVERDRIVE,
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
		/* 281 */ ACTION_281,
		/* 282 */ ACTION_282,
		/* 283 */ ACTION_421V,
		/* 284 */ ACTION_j421V,

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
		/* 404 */ ACTION_GAME_START1,
		/* 405 */ ACTION_GAME_START2,
		/* 406 */ ACTION_GAME_START3,
		/* 407 */ ACTION_GAME_START4,

		/* 450  */ ACTION_GROUND_HIGH_REVERSAL = 450,
		/* 451  */ ACTION_GROUND_LOW_REVERSAL,
		/* 452  */ ACTION_AIR_REVERSAL,
	};

	extern MYDLL_API const std::map<CharacterActions, std::string> actionNames;

	class Character : public Object {
	public:
		struct ReplayData {
			bool n : 1;
			bool m : 1;
			bool v : 1;
			bool s : 1;
			bool a : 1;
			bool d : 1;
			char _h : 2;
			char _v : 2;
			unsigned char time : 6;
		};
		static_assert(sizeof(ReplayData) == 2);

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
		enum SubObjectAnchor {
			ANCHOR_OWNER,
			ANCHOR_OPPONENT,
			ANCHOR_BORDER_FRONT,
			ANCHOR_BORDER_BACK,
			ANCHOR_STAGE_MIN,
			ANCHOR_STAGE_MAX,
			ANCHOR_STAGE_CENTER,
		};
		enum SubObjectDirection {
			DIRECTION_FRONT,
			DIRECTION_BACK,
			DIRECTION_LEFT,
			DIRECTION_RIGHT,
			DIRECTION_OP_FRONT,
			DIRECTION_OP_BACK,
		};
		struct SubObjectData {
			unsigned action;
			Vector2f offset;
			Vector2<SubObjectAnchor> anchor{ANCHOR_OWNER, ANCHOR_OWNER};
			SubObjectDirection dir;
			nlohmann::json json;
		};
#pragma pack(push, 1)
		struct LastInput {
			unsigned nbFrames;
			bool n;
			bool m;
			bool s;
			bool o;
			bool d;
			bool a;
			char h;
			char v;
		};
		struct Data {
			InputStruct _inputBuffer;
			unsigned _limit[4];
			unsigned _subObjects[128];
			int _timeSinceIdle;
			unsigned _grabInvul;
			unsigned _supersUsed;
			unsigned _skillsUsed;
			unsigned _odCooldown;
			unsigned _blockStun;
			unsigned _jumpsUsed;
			unsigned _airDashesUsed;
			unsigned _comboCtr;
			unsigned _totalDamage;
			unsigned _guardCooldown;
			unsigned _guardBar;
			unsigned _guardRegenCd;
			unsigned _nbLastInputs;
			unsigned _nbReplayInputs;
			unsigned _nbUsedMoves;
			unsigned _neutralEffectTimer;
			unsigned _matterEffectTimer;
			unsigned _spiritEffectTimer;
			unsigned _voidEffectTimer;
			unsigned _matterInstallTimer;
			unsigned _spiritInstallTimer;
			unsigned _voidInstallTimer;
			unsigned char _hitStop;
			float _regen;
			float _voidMana;
			float _spiritMana;
			float _matterMana;
			float _prorate;
			bool _ultimateUsed;
			bool _counter;
			bool _jumpCanceled;
			bool _hadUltimate;
			bool _wrongMana;
			bool _atkDisabled;
			bool _inputDisabled;
			bool _hasJumped;
			bool _restand;
			bool _justGotCorner;
			char _normalTreeFlag;
			bool _armorUsed;
			bool _hardKD;
			unsigned char _specialInputs[52];
		};
		union SpecialInputs {
			unsigned char _value[52] = {0};
			struct {
				unsigned char _22: 4;
				unsigned char _44: 4;
				unsigned char _66: 4;
				unsigned char _27: 4;
				unsigned char _28: 4;
				unsigned char _29: 4;
				char _an: 6;
				char _am: 6;
				char _as: 6;
				char _av: 6;
				unsigned char _c28n: 4;
				unsigned char _c28m: 4;
				unsigned char _c28s: 4;
				unsigned char _c28v: 4;
				unsigned char _c28d: 4;
				unsigned char _c28a: 4;
				unsigned char _c46n: 4;
				unsigned char _c46m: 4;
				unsigned char _c46s: 4;
				unsigned char _c46v: 4;
				unsigned char _c46d: 4;
				unsigned char _c46a: 4;
				unsigned char _c64n: 4;
				unsigned char _c64m: 4;
				unsigned char _c64s: 4;
				unsigned char _c64v: 4;
				unsigned char _c64d: 4;
				unsigned char _c64a: 4;
				unsigned char _236n: 4;
				unsigned char _236m: 4;
				unsigned char _236s: 4;
				unsigned char _236v: 4;
				unsigned char _236d: 4;
				unsigned char _236a: 4;
				unsigned char _214n: 4;
				unsigned char _214m: 4;
				unsigned char _214s: 4;
				unsigned char _214v: 4;
				unsigned char _214d: 4;
				unsigned char _214a: 4;
				unsigned char _623n: 4;
				unsigned char _623m: 4;
				unsigned char _623s: 4;
				unsigned char _623v: 4;
				unsigned char _623d: 4;
				unsigned char _623a: 4;
				unsigned char _421n: 4;
				unsigned char _421m: 4;
				unsigned char _421s: 4;
				unsigned char _421v: 4;
				unsigned char _421d: 4;
				unsigned char _421a: 4;
				unsigned char _624n: 4;
				unsigned char _624m: 4;
				unsigned char _624s: 4;
				unsigned char _624v: 4;
				unsigned char _624d: 4;
				unsigned char _624a: 4;
				unsigned char _426n: 4;
				unsigned char _426m: 4;
				unsigned char _426s: 4;
				unsigned char _426v: 4;
				unsigned char _426d: 4;
				unsigned char _426a: 4;
				unsigned char _6314n: 4;
				unsigned char _6314m: 4;
				unsigned char _6314s: 4;
				unsigned char _6314v: 4;
				unsigned char _6314d: 4;
				unsigned char _6314a: 4;
				unsigned char _4136n: 4;
				unsigned char _4136m: 4;
				unsigned char _4136s: 4;
				unsigned char _4136v: 4;
				unsigned char _4136d: 4;
				unsigned char _4136a: 4;
				unsigned char _624684n: 4;
				unsigned char _624684m: 4;
				unsigned char _624684s: 4;
				unsigned char _624684v: 4;
				unsigned char _624684d: 4;
				unsigned char _624684a: 4;
				unsigned char _6314684n: 4;
				unsigned char _6314684m: 4;
				unsigned char _6314684s: 4;
				unsigned char _6314684v: 4;
				unsigned char _6314684d: 4;
				unsigned char _6314684a: 4;
				unsigned char _6246974n: 4;
				unsigned char _6246974m: 4;
				unsigned char _6246974s: 4;
				unsigned char _6246974v: 4;
				unsigned char _6246974d: 4;
				unsigned char _6246974a: 4;
				unsigned char _63146974n: 4;
				unsigned char _63146974m: 4;
				unsigned char _63146974s: 4;
				unsigned char _63146974v: 4;
				unsigned char _63146974d: 4;
				unsigned char _63146974a: 4;
			};
		};
#pragma pack(pop)
		static_assert(sizeof(SpecialInputs::_value) == sizeof(Data::_specialInputs));
		static_assert(sizeof(SpecialInputs::_value) == sizeof(SpecialInputs));

		static std::function<bool (const LastInput &)> getInputN;
		static std::function<bool (const LastInput &)> getInputM;
		static std::function<bool (const LastInput &)> getInputS;
		static std::function<bool (const LastInput &)> getInputV;
		static std::function<bool (const LastInput &)> getInputD;
		static std::function<bool (const LastInput &)> getInputA;

		// Game State
		std::vector<ReplayData> _replayData;
		std::list<LastInput> _lastInputs;
		std::map<unsigned, unsigned> _usedMoves;
		std::array<std::pair<unsigned, std::shared_ptr<IObject>>, 128> _subobjects;
		std::array<unsigned, 4> _limit{0, 0, 0, 0};
		InputStruct _inputBuffer = {0, 0, 0, 0, 0, 0, 0, 0, 0};
		int _timeSinceIdle = 0;
		unsigned _supersUsed = 0;
		unsigned _skillsUsed = 0;
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
		unsigned _grabInvul = 0;
		unsigned _neutralEffectTimer = 0;
		unsigned _matterEffectTimer = 0;
		unsigned _spiritEffectTimer = 0;
		unsigned _voidEffectTimer = 0;
		unsigned _matterInstallTimer = 0;
		unsigned _spiritInstallTimer = 0;
		unsigned _voidInstallTimer = 0;
		float _prorate = 1;
		float _regen = 0;
		float _voidMana = 0;
		float _spiritMana = 0;
		float _matterMana = 0;
		SpecialInputs _specialInputs;
		bool _counter = false;
		bool _wrongMana = false;
		bool _ultimateUsed = false;
		bool _jumpCanceled = false;
		bool _atkDisabled = false;
		bool _inputDisabled = false;
		bool _hasJumped = false;
		bool _restand = false;
		bool _justGotCorner = false;
		bool _hadUltimate = false;
		bool _armorUsed = false;
		bool _forceCH = false;
		bool _hardKD = false;
		char _normalTreeFlag = 0;

		// Non Game State
		sf::Text _text;
		sf::Text _text2;
		Character *_opponent = nullptr;
		std::map<unsigned, SubObjectData> _projectileData;
		std::map<unsigned, std::vector<std::vector<FrameData>>> _subObjectsData;
		std::shared_ptr<IInput> _input;
		unsigned _maxOdCooldown = 0;
		unsigned _maxJumps = 0;
		unsigned _maxAirDashes = 0;
		unsigned _voidManaMax = 0;
		unsigned _spiritManaMax = 0;
		unsigned _matterManaMax = 0;
		unsigned _maxGuardCooldown = 0;
		unsigned _maxGuardBar = 0;
		bool _gotHitStopReset = false;
		bool _hasBeenHitDuringFrame = false;
		bool _hasHitDuringFrame = false;
		mutable unsigned _effectTimer = 0;
		mutable Sprite _neutralEffect;
		mutable Sprite _matterEffect;
		mutable Sprite _spiritEffect;
		mutable Sprite _voidEffect;

		//Practice
		GroundTech _dummyGroundTech = GROUNDTECH_NONE;
		AirTech _dummyAirTech = AIRTECH_NONE;
		DummyState _dummyState = DUMMYSTATE_STANDING;
		unsigned char _forceBlock = 0;
		unsigned char _counterHit = 0;

		virtual void _mutateHitFramedata(FrameData &framedata) const;
		virtual bool _executeNeutralAttack(unsigned base);
		virtual bool _executeDownAttack(unsigned base);
		virtual unsigned _getReversalAction();
		virtual void _parryEffect(Object *other);
		virtual void _parryVoidEffect(Object *other, bool isStrongest);
		virtual void _parryMatterEffect(Object *other, bool isStrongest);
		virtual void _parrySpiritEffect(Object *other, bool isStrongest);
		virtual void _processGuardLoss(unsigned loss);
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
		virtual void _manaCrush();
		virtual bool _consumeVoidMana(float cost);
		virtual bool _consumeMatterMana(float cost);
		virtual bool _consumeSpiritMana(float cost);
		virtual std::pair<unsigned int, std::shared_ptr<IObject>>
		_spawnSubObject(BattleManager &manager, unsigned int id, bool needRegister);

		static bool isBlockingAction(unsigned action);
		static bool isParryAction(unsigned action);
		static bool isOverdriveAction(unsigned action);
		static bool isRomanCancelAction(unsigned action);
		static bool isHitAction(unsigned int action);
		static SubObjectAnchor anchorFromString(const std::string &str);
		static SubObjectDirection directionFromString(const std::string &str);

		bool _getProjectileDirection(const SubObjectData &data);
		float _getAnchoredPos(const Character::SubObjectData &data, bool y);
		Vector2f _calcProjectilePosition(const SubObjectData &data, float dir);
		void _loadProjectileData(const std::string &path);
		void _tickMove() override;
		void _applyNewAnimFlags() override;
		void _applyMoveAttributes() override;
		void _forceStartMove(unsigned action) override;
		void _onMoveEnd(const FrameData &lastData) override;
		bool _canStartMove(unsigned action, const FrameData &data) override;
		void _checkPlatforms(Vector2f oldPos) override;
		bool _isOnPlatform() const override;
		InputStruct _updateInputs(bool tickBuffer = true);
		void _computeFrameDataCache() override;

		void _renderInstallEffect(Sprite &sprite) const;
		void _renderEffect(const Vector2f &result, Sprite &sprite) const;
		void _removeSubobjects();
		void _checkSpecialInputs(bool tickBuffer);
		void _clearLastInputs();
		bool _check22Input();
		bool _check44Input();
		bool _check66Input();
		bool _check27Input();
		bool _check28Input();
		bool _check29Input();
		void _checkAllAXMacro(bool tickBuffer);
		void _checkAllHJInput(bool tickBuffer);
		void _checkAllDashInput(bool tickBuffer);
		void _checkAllc28Input(bool tickBuffer);
		void _checkAllc46Input(bool tickBuffer);
		void _checkAllc64Input(bool tickBuffer);
		void _checkAll236Input(bool tickBuffer);
		void _checkAll214Input(bool tickBuffer);
		void _checkAll623Input(bool tickBuffer);
		void _checkAll421Input(bool tickBuffer);
		void _checkAll624Input(bool tickBuffer);
		void _checkAll426Input(bool tickBuffer);
		void _checkAll6314Input(bool tickBuffer);
		void _checkAll4136Input(bool tickBuffer);
		void _checkAll624684Input(bool tickBuffer);
		void _checkAll6314684Input(bool tickBuffer);
		void _checkAll6246974Input(bool tickBuffer);
		void _checkAll63146974Input(bool tickBuffer);
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
		struct InitData {
			bool side;
			unsigned short maxHp;
			unsigned char maxJumps;
			unsigned char maxAirDash;
			unsigned maxMMana;
			unsigned maxVMana;
			unsigned maxSMana;
			float manaRegen;
			unsigned maxGuardBar;
			unsigned maxGuardCooldown;
			unsigned odCd;
			float groundDrag;
			Vector2f airDrag;
			Vector2f gravity;
		};

		unsigned index;
		std::wstring name;
		bool showAttributes = false;
		bool startedAttack = false;

		Character();
		Character(unsigned index, const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette, std::shared_ptr<IInput> input);
		~Character() override;
		void setOpponent(Character *opponent);
		bool hits(const IObject &other) const override;
		void hit(IObject &other, const FrameData *data) override;
		void getHit(IObject &other, const FrameData *data) override;
		void render() const override;
		void update() override;
		InputStruct updateInputs();
		void consumeEvent(const sf::Event &event);
		virtual void postUpdate();
		std::shared_ptr<IInput> &getInput();
		const std::shared_ptr<IInput> &getInput() const;
		const std::map<unsigned, std::vector<std::vector<FrameData>>> &getFrameData();
		virtual int getAttackTier(unsigned int action) const;
		virtual void setAttacksDisabled(bool disabled);
		virtual void disableInputs(bool disabled);
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		unsigned int getClassId() const override;
		const std::vector<ReplayData> &getReplayData() const;
		bool isHit() const;
		size_t printDifference(const char *msgStart, void *pVoid, void *pVoid1, unsigned startOffset) const override;
		int getLayer() const override;
		std::array<unsigned, 4> getLimit() const;

		static std::string actionToString(int action);

		virtual void drawSpecialHUD(sf::RenderTarget &texture);
		virtual void drawSpecialHUDNoReverse(sf::RenderTarget &texture);
		virtual void init(BattleManager &manager, const InitData &data);
		virtual void resolveSubObjects(const BattleManager &manager);
		virtual void onMatchEnd();
		virtual bool matchEndUpdate();

		friend class PracticeBattleManager;
		friend class PracticeInGame;
		friend class ReplayInGame;
		friend class BattleManager;
		friend class RollbackMachine;
	};
}


#endif //SOFGV_CHARACTER_HPP
