//
// Created by PinkySmile on 18/09/2021.
//

#ifdef _WIN32
#include <windows.h>
#endif
#include "Character.hpp"
#include "Resources/Game.hpp"
#include "Logger.hpp"
#include "Objects/Characters/Projectile.hpp"
#ifndef maxi
#define maxi(x, y) (x > y ? x : y)
#endif
#ifndef mini
#define mini(x, y) (x < y ? x : y)
#endif

#define GRAB_INVUL_STACK 9

#define MAX_LIMIT_EFFECT_TIMER (15 << 4)
#define DEC_LIMIT_EFFECT_TIMER(x) ((x) -= (1 << 4))

#define TMP_GUARD_MAX (300)
#define REGEN_CD_BLOCK (240)
#define REGEN_CD_PARRY (240)

#define MINIMUM_STALLING_STACKING (-800)
#define STALLING_HIT_REMOVE (75)
#define STALLING_BLOCK_REMOVE (25)
#define STALLING_BEING_HIT_REMOVE (50)
#define STALLING_BLOCKING_REMOVE (50)
#define STALLING_BLOCK_WIPE_THRESHOLD (350)
#define MAXIMUM_STALLING_STACKING (2700)
#define BACKING_STALLING_FACTOR (1.6f)
#define FORWARD_STALLING_FACTOR (0.9f)
#define GROUND_STALLING_FACTOR (0.25f)
#define MAX_STALLING_FACTOR_HEIGHT (500.f)
#define PASSIVE_STALLING_FACTOR (0.5f)
// 0.1% per frame max
#define METER_PENALTY_EQUATION(val, maxval) (((val) - START_STALLING_THRESHOLD) * (maxval) / (float)(MAXIMUM_STALLING_STACKING - START_STALLING_THRESHOLD) / 1000)

#define IDLE_ANIM_FIRST 600
#define IDLE_ANIM_CHANCE 50
#define IDLE_ANIM_CD_MIN 300
#define IDLE_ANIM_CD_MAX 600

#define PARRY_COST 100
#define INSTALL_COST 200
#define INSTALL_DURATION 30
#define REFLECT_PERCENT 60
#define COMBINATION_LENIENCY 4
#define MAX_FRAME_IN_BUFFER 60

#define WALL_SLAM_HITSTUN_INCREASE 30
#define GROUND_SLAM_HITSTUN_INCREASE 30

#define SPECIAL_INPUT_BUFFER_PERSIST 10
#define DASH_BUFFER_PERSIST 6
#define HJ_BUFFER_PERSIST 6

#define NORMAL_BUFFER 4
#define HJ_BUFFER 10
#define DASH_BUFFER 15
#define QUARTER_CIRCLE_BUFFER 10
#define DP_BUFFER 15
#define HALF_CIRCLE_BUFFER 20
#define SPIRAL_BUFFER 30
#define CHARGE_PART_BUFFER 10
#define CHARGE_BUFFER 5
#define CHARGE_TIME 25

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
	"resetOPSpeed",
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
	"matterMana",
	"turnAround",
	"forceTurnAround",
	"nextBlockOnHit",
	"nextBlockOnBlock",
	"hardKnockDown",
	"groundSlam",
	"groundSlamCH",
	"wallSplat",
	"wallSplatCH"
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
	"karaCancel",
	"resetRotation",
	"counterHit",
	"flash",
	"crouch",
	"projectileInvul",
	"projectile",
	"landCancel",
	"dashCancel",
	"resetSpeed",
	"neutralInvul",
	"matterInvul",
	"spiritInvul",
	"voidInvul",
	"neutralArmor",
	"matterArmor",
	"spiritArmor",
	"voidArmor"
};

namespace SpiralOfFate
{
	std::function<bool (const Character::LastInput &)> Character::getInputN = [](const Character::LastInput &input) { return input.n; };
	std::function<bool (const Character::LastInput &)> Character::getInputM = [](const Character::LastInput &input) { return input.m; };
	std::function<bool (const Character::LastInput &)> Character::getInputS = [](const Character::LastInput &input) { return input.s; };
	std::function<bool (const Character::LastInput &)> Character::getInputV = [](const Character::LastInput &input) { return input.o; };
	std::function<bool (const Character::LastInput &)> Character::getInputD = [](const Character::LastInput &input) { return input.d; };
	std::function<bool (const Character::LastInput &)> Character::getInputA = [](const Character::LastInput &input) { return input.a; };

	const std::map<CharacterActions, std::string> actionNames{
		{ ACTION_IDLE,                           "Idle" },
		{ ACTION_CROUCHING,                      "Crouching" },
		{ ACTION_CROUCH,                         "Crouch" },
		{ ACTION_STANDING_UP,                    "Standing up" },
		{ ACTION_WALK_FORWARD,                   "Walk forward" },
		{ ACTION_WALK_BACKWARD,                  "Walk backward" },
		{ ACTION_FORWARD_DASH,                   "Forward dash" },
		{ ACTION_BACKWARD_DASH,                  "Backward dash" },
		{ ACTION_NEUTRAL_JUMP,                   "Neutral jump" },
		{ ACTION_FORWARD_JUMP,                   "Forward jump" },
		{ ACTION_BACKWARD_JUMP,                  "Backward jump" },
		{ ACTION_NEUTRAL_HIGH_JUMP,              "Neutral high jump" },
		{ ACTION_FORWARD_HIGH_JUMP,              "Forward high jump" },
		{ ACTION_BACKWARD_HIGH_JUMP,             "Backward high jump" },
		{ ACTION_FALLING,                        "Falling" },
		{ ACTION_LANDING,                        "Landing" },
		{ ACTION_AIR_DASH_1,                     "Up back air dash" },
		{ ACTION_AIR_DASH_2,                     "Up air dash" },
		{ ACTION_AIR_DASH_3,                     "Up forward air dash" },
		{ ACTION_AIR_DASH_4,                     "Back air dash" },
		{ ACTION_AIR_DASH_6,                     "Forward air dash" },
		{ ACTION_AIR_DASH_7,                     "Down back air dash" },
		{ ACTION_AIR_DASH_8,                     "Down air dash" },
		{ ACTION_AIR_DASH_9,                     "Down forward air dash" },
		{ ACTION_GROUND_HIGH_NEUTRAL_BLOCK,      "Ground high neutral block" },
		{ ACTION_GROUND_HIGH_NEUTRAL_PARRY,      "Ground high neutral parry" },
		{ ACTION_GROUND_HIGH_SPIRIT_PARRY,       "Ground high spirit parry" },
		{ ACTION_GROUND_HIGH_MATTER_PARRY,       "Ground high matter parry" },
		{ ACTION_GROUND_HIGH_VOID_PARRY,         "Ground high void parry" },
		{ ACTION_GROUND_HIGH_HIT,                "Ground high hit" },
		{ ACTION_GROUND_LOW_NEUTRAL_BLOCK,       "Ground low neutral block" },
		{ ACTION_GROUND_LOW_NEUTRAL_PARRY,       "Ground low neutral parry" },
		{ ACTION_GROUND_LOW_SPIRIT_PARRY,        "Ground low spirit parry" },
		{ ACTION_GROUND_LOW_MATTER_PARRY,        "Ground low matter parry" },
		{ ACTION_GROUND_LOW_VOID_PARRY,          "Ground low void parry" },
		{ ACTION_GROUND_LOW_HIT,                 "Ground low hit" },
		{ ACTION_AIR_NEUTRAL_BLOCK,              "Air neutral block" },
		{ ACTION_AIR_NEUTRAL_PARRY,              "Air neutral parry" },
		{ ACTION_AIR_SPIRIT_PARRY,               "Air spirit parry" },
		{ ACTION_AIR_MATTER_PARRY,               "Air matter parry" },
		{ ACTION_AIR_VOID_PARRY,                 "Air void parry" },
		{ ACTION_AIR_HIT,                        "Air hit" },
		{ ACTION_BEING_KNOCKED_DOWN,             "Being knocked down" },
		{ ACTION_KNOCKED_DOWN,                   "Knocked down" },
		{ ACTION_NEUTRAL_TECH,                   "Neutral tech" },
		{ ACTION_BACKWARD_TECH,                  "Backward tech" },
		{ ACTION_FORWARD_TECH,                   "Forward tech" },
		{ ACTION_FALLING_TECH,                   "Falling tech" },
		{ ACTION_UP_AIR_TECH,                    "Up air tech" },
		{ ACTION_DOWN_AIR_TECH,                  "Down air tech" },
		{ ACTION_FORWARD_AIR_TECH,               "Forward air tech" },
		{ ACTION_BACKWARD_AIR_TECH,              "Backward air tech" },
		{ ACTION_AIR_TECH_LANDING_LAG,           "Air tech landing lag" },
		{ ACTION_GROUND_SLAM,                    "Ground slam" },
		{ ACTION_WALL_SLAM,                      "Wall slam" },
		{ ACTION_HARD_LAND,                      "Hard land" },
		{ ACTION_NEUTRAL_AIR_JUMP,               "Neutral air jump" },
		{ ACTION_FORWARD_AIR_JUMP,               "Forward air jump" },
		{ ACTION_BACKWARD_AIR_JUMP,              "Backward air jump" },
		{ ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK,"Ground high neutral wrong block" },
		{ ACTION_GROUND_LOW_NEUTRAL_WRONG_BLOCK, "Ground low neutral wrong block" },
		{ ACTION_AIR_NEUTRAL_WRONG_BLOCK,        "Air neutral wrong block" },
		{ ACTION_5N,                             "5n" },
		{ ACTION_6N,                             "6n" },
		{ ACTION_8N,                             "8n" },
		{ ACTION_3N,                             "3n" },
		{ ACTION_2N,                             "2n" },
		{ ACTION_214N,                           "214n" },
		{ ACTION_236N,                           "236n" },
		{ ACTION_623N,                           "623n" },
		{ ACTION_41236N,                         "41236n" },
		{ ACTION_63214N,                         "63214n" },
		{ ACTION_6321469874N,                    "6321469874n" },
		{ ACTION_j5N,                            "j5n" },
		{ ACTION_j6N,                            "j6n" },
		{ ACTION_j8N,                            "j8n" },
		{ ACTION_j3N,                            "j3n" },
		{ ACTION_j2N,                            "j2n" },
		{ ACTION_j214N,                          "j214n" },
		{ ACTION_j236N,                          "j236n" },
		{ ACTION_j623N,                          "j623n" },
		{ ACTION_j41236N,                        "j41236n" },
		{ ACTION_j63214N,                        "j63214n" },
		{ ACTION_j6321469874N,                   "j6321469874n" },
		{ ACTION_t5N,                            "t5n" },
		{ ACTION_t6N,                            "t6n" },
		{ ACTION_t8N,                            "t8n" },
		{ ACTION_t2N,                            "t2n" },
		{ ACTION_c28N,                           "[2]8n" },
		{ ACTION_c46N,                           "[4]6n" },
		{ ACTION_c64N,                           "[6]4n" },
		{ ACTION_5M,                             "5m" },
		{ ACTION_6M,                             "6m" },
		{ ACTION_8M,                             "8m" },
		{ ACTION_3M,                             "3m" },
		{ ACTION_2M,                             "2m" },
		{ ACTION_214M,                           "214m" },
		{ ACTION_236M,                           "236m" },
		{ ACTION_623M,                           "623m" },
		{ ACTION_41236M,                         "41236m" },
		{ ACTION_63214M,                         "63214m" },
		{ ACTION_6321469874M,                    "6321469874m" },
		{ ACTION_j5M,                            "j5m" },
		{ ACTION_j6M,                            "j6m" },
		{ ACTION_j8M,                            "j8m" },
		{ ACTION_j3M,                            "j3m" },
		{ ACTION_j2M,                            "j2m" },
		{ ACTION_j214M,                          "j214m" },
		{ ACTION_j236M,                          "j236m" },
		{ ACTION_j623M,                          "j623m" },
		{ ACTION_j41236M,                        "j41236m" },
		{ ACTION_j63214M,                        "j63214m" },
		{ ACTION_j6321469874M,                   "j6321469874m" },
		{ ACTION_t5M,                            "t5m" },
		{ ACTION_t6M,                            "t6m" },
		{ ACTION_t8M,                            "t8m" },
		{ ACTION_t2M,                            "t2m" },
		{ ACTION_c28M,                           "[2]8m" },
		{ ACTION_c46M,                           "[4]6m" },
		{ ACTION_c64M,                           "[6]4m" },
		{ ACTION_5S,                             "5s" },
		{ ACTION_6S,                             "6s" },
		{ ACTION_8S,                             "8s" },
		{ ACTION_3S,                             "3s" },
		{ ACTION_2S,                             "2s" },
		{ ACTION_214S,                           "214s" },
		{ ACTION_236S,                           "236s" },
		{ ACTION_623S,                           "623s" },
		{ ACTION_41236S,                         "41236s" },
		{ ACTION_63214S,                         "63214s" },
		{ ACTION_6321469874S,                    "6321469874s" },
		{ ACTION_j5S,                            "j5s" },
		{ ACTION_j6S,                            "j6s" },
		{ ACTION_j8S,                            "j8s" },
		{ ACTION_j3S,                            "j3s" },
		{ ACTION_j2S,                            "j2s" },
		{ ACTION_j214S,                          "j214s" },
		{ ACTION_j236S,                          "j236s" },
		{ ACTION_j623S,                          "j623s" },
		{ ACTION_j41236S,                        "j41236s" },
		{ ACTION_j63214S,                        "j63214s" },
		{ ACTION_j6321469874S,                   "j6321469874s" },
		{ ACTION_t5S,                            "t5s" },
		{ ACTION_t6S,                            "t6s" },
		{ ACTION_t8S,                            "t8s" },
		{ ACTION_t2S,                            "t2s" },
		{ ACTION_c28S,                           "[2]8s" },
		{ ACTION_c46S,                           "[4]6s" },
		{ ACTION_c64S,                           "[6]4s" },
		{ ACTION_5V,                             "5v" },
		{ ACTION_6V,                             "6v" },
		{ ACTION_8V,                             "8v" },
		{ ACTION_3V,                             "3v" },
		{ ACTION_2V,                             "2v" },
		{ ACTION_214V,                           "214v" },
		{ ACTION_236V,                           "236v" },
		{ ACTION_623V,                           "623v" },
		{ ACTION_41236V,                         "41236v" },
		{ ACTION_63214V,                         "63214v" },
		{ ACTION_6321469874V,                    "6321469874v" },
		{ ACTION_j5V,                            "j5v" },
		{ ACTION_j6V,                            "j6v" },
		{ ACTION_j8V,                            "j8v" },
		{ ACTION_j3V,                            "j3v" },
		{ ACTION_j2V,                            "j2v" },
		{ ACTION_j214V,                          "j214v" },
		{ ACTION_j236V,                          "j236v" },
		{ ACTION_j623V,                          "j623v" },
		{ ACTION_j41236V,                        "j41236v" },
		{ ACTION_j63214V,                        "j63214v" },
		{ ACTION_j6321469874V,                   "j6321469874v" },
		{ ACTION_t5V,                            "t5v" },
		{ ACTION_t6V,                            "t6v" },
		{ ACTION_t8V,                            "t8v" },
		{ ACTION_t2V,                            "t2v" },
		{ ACTION_c28V,                           "[2]8v" },
		{ ACTION_c46V,                           "[4]6v" },
		{ ACTION_c64V,                           "[6]4v" },
		{ ACTION_5A,                             "5a" },
		{ ACTION_6A,                             "6a" },
		{ ACTION_8A,                             "8a" },
		{ ACTION_3A,                             "3a" },
		{ ACTION_2A,                             "2a" },
		{ ACTION_214A,                           "214a" },
		{ ACTION_236A,                           "236a" },
		{ ACTION_421A,                           "421a" },
		{ ACTION_623A,                           "623a" },
		{ ACTION_41236A,                         "41236a" },
		{ ACTION_63214A,                         "63214a" },
		{ ACTION_6321469874A,                    "6321469874a" },
		{ ACTION_j5A,                            "j5a" },
		{ ACTION_j6A,                            "j6a" },
		{ ACTION_j8A,                            "j8a" },
		{ ACTION_j3A,                            "j3a" },
		{ ACTION_j2A,                            "j2a" },
		{ ACTION_j214A,                          "j214a" },
		{ ACTION_j236A,                          "j236a" },
		{ ACTION_j421A,                          "j421a" },
		{ ACTION_j623A,                          "j623a" },
		{ ACTION_j41236A,                        "j41236a" },
		{ ACTION_j63214A,                        "j63214a" },
		{ ACTION_j6321469874A,                   "j6321469874a" },
		{ ACTION_t5A,                            "t5a" },
		{ ACTION_t6A,                            "t6a" },
		{ ACTION_t8A,                            "t8a" },
		{ ACTION_t2A,                            "t2a" },
		{ ACTION_c28A,                           "[2]8a" },
		{ ACTION_c46A,                           "[4]6a" },
		{ ACTION_c64A,                           "[6]4a" },
		{ ACTION_214D,                           "214d"},
		{ ACTION_236D,                           "236d" },
		{ ACTION_421D,                           "421d" },
		{ ACTION_623D,                           "623d" },
		{ ACTION_41236D,                         "41236d" },
		{ ACTION_63214D,                         "63214d" },
		{ ACTION_6321469874D,                    "6321469874d" },
		{ ACTION_j214D,                          "j214d" },
		{ ACTION_j236D,                          "j236d" },
		{ ACTION_j421D,                          "j421d" },
		{ ACTION_j623D,                          "j623d" },
		{ ACTION_j41236D,                        "j41236d" },
		{ ACTION_j63214D,                        "j63214d" },
		{ ACTION_j6321469874D,                   "j6321469874d" },
		{ ACTION_t5D,                            "t5d" },
		{ ACTION_t6D,                            "t6d" },
		{ ACTION_t8D,                            "t8d" },
		{ ACTION_t2D,                            "t2d" },

		{ ACTION_NEUTRAL_OVERDRIVE,     "Neutral overdrive" },
		{ ACTION_MATTER_OVERDRIVE,      "Matter overdrive" },
		{ ACTION_SPIRIT_OVERDRIVE,      "Spirit overdrive" },
		{ ACTION_VOID_OVERDRIVE,        "Void overdrive" },
		{ ACTION_NEUTRAL_AIR_OVERDRIVE, "Neutral air overdrive" },
		{ ACTION_MATTER_AIR_OVERDRIVE,  "Matter air overdrive" },
		{ ACTION_SPIRIT_AIR_OVERDRIVE,  "Spirit air overdrive" },
		{ ACTION_VOID_AIR_OVERDRIVE,    "Void air overdrive" },
		{ ACTION_ROMAN_CANCEL,          "Roman cancel" },
		{ ACTION_AIR_ROMAN_CANCEL,      "Air roman cancel" },
		{ ACTION_GROUND_HIGH_REVERSAL,  "Ground high reversal" },
		{ ACTION_GROUND_LOW_REVERSAL,   "Ground low reversal" },
		{ ACTION_AIR_REVERSAL,          "Air reversal" },

		{ ACTION_WIN_MATCH1,            "Win match1" },
		{ ACTION_WIN_MATCH2,            "Win match2" },
		{ ACTION_WIN_MATCH3,            "Win match3" },
		{ ACTION_WIN_MATCH4,            "Win match4" },
		{ ACTION_GAME_START1,           "Game start1" },
		{ ACTION_GAME_START2,           "Game start2" },
		{ ACTION_GAME_START3,           "Game start3" },
		{ ACTION_GAME_START4,           "Game start4" },
		//{ ACTION_WIN_ROUND1,          "Win round1" },
		//{ ACTION_WIN_ROUND2,          "Win round2" },
		//{ ACTION_WIN_ROUND3,          "Win round3" },
		//{ ACTION_WIN_ROUND4,          "Win round4" },
		//{ ACTION_LOOSE_MATCH1,        "Loose match1" },
		//{ ACTION_LOOSE_MATCH2,        "Loose match2" },
		//{ ACTION_LOOSE_MATCH3,        "Loose match3" },
		//{ ACTION_LOOSE_MATCH4,        "Loose match4" },
		//{ ACTION_LOOSE_ROUND1,        "Loose round1" },
		//{ ACTION_LOOSE_ROUND2,        "Loose round2" },
		//{ ACTION_LOOSE_ROUND3,        "Loose round3" },
		//{ ACTION_LOOSE_ROUND4,        "Loose round4" },
	};

	std::string Character::actionToString(int action)
	{
		auto name = actionNames.find(static_cast<SpiralOfFate::CharacterActions>(action));

		return name == SpiralOfFate::actionNames.end() ? "Action #" + std::to_string(action) : name->second;
	}

	Character::Character()
	{
		this->_neutralEffect.textureHandle = game->textureMgr.load("assets/effects/neutralHit.png");
		this->_matterEffect.textureHandle = game->textureMgr.load("assets/effects/matterHit.png");
		this->_spiritEffect.textureHandle = game->textureMgr.load("assets/effects/spiritHit.png");
		this->_voidEffect.textureHandle = game->textureMgr.load("assets/effects/voidHit.png");
	}

	Character::Character(unsigned index, const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette, std::shared_ptr<IInput> input) :
		_input(std::move(input)),
		index(index)
	{
		this->_neutralEffect.textureHandle = game->textureMgr.load("assets/effects/neutralHit.png");
		this->_matterEffect.textureHandle = game->textureMgr.load("assets/effects/matterHit.png");
		this->_spiritEffect.textureHandle = game->textureMgr.load("assets/effects/spiritHit.png");
		this->_voidEffect.textureHandle = game->textureMgr.load("assets/effects/voidHit.png");
		this->_text.setFont(game->font);
		this->_text.setFillColor(sf::Color::White);
		this->_text.setOutlineColor(sf::Color::Black);
		this->_text.setOutlineThickness(2);
		this->_text.setCharacterSize(10);
		this->_text2.setFont(game->font);
		this->_text2.setFillColor(sf::Color::White);
		this->_text2.setOutlineColor(sf::Color::Black);
		this->_text2.setOutlineThickness(2);
		this->_text2.setCharacterSize(10);
		this->_limit.fill(0);
		this->_moves = FrameData::loadFile(folder + "/framedata.json", folder, palette);
		this->_subObjectsData = FrameData::loadFile(folder + "/subobj_data.json", folder, palette);
		this->_loadProjectileData(folder + "/subobjects.json");
		this->_loadParticleData(folder + "/particles.json", folder);
		this->_lastInputs.push_back(LastInput{0, false, false, false, false, false, false, 0, 0});
	}

	Character::~Character()
	{
		game->textureMgr.remove(this->_neutralEffect.textureHandle);
		game->textureMgr.remove(this->_matterEffect.textureHandle);
		game->textureMgr.remove(this->_spiritEffect.textureHandle);
		game->textureMgr.remove(this->_voidEffect.textureHandle);
	}

	void Character::_renderEffect(const Vector2f &result, Sprite &sprite) const
	{
		auto size = game->textureMgr.getTextureSize(sprite.textureHandle);
		auto &data = *this->getCurrentFrameData();

		sprite.setScale({data.size.x / 40.f, static_cast<float>(data.size.y) / size.y});
		sprite.setOrigin({20, size.y / 2.f});
		sprite.setTextureRect({static_cast<int>(40 * (this->_effectTimer / 4) % size.x), 0, 40, 40});
		sprite.setPosition(result);
		game->textureMgr.render(sprite);
	}

	void Character::_renderInstallEffect(Sprite &sprite) const
	{
		auto size = game->textureMgr.getTextureSize(sprite.textureHandle);

		game->textureMgr.setTexture(sprite);
		for (auto &box : this->_getModifiedHitBoxes()) {
			sf::VertexArray arr{sf::Quads, 4};

			for (int i = 0; i < 4; i++) {
				arr[i].texCoords = {
					static_cast<int>(40 * (this->_effectTimer / 4) % size.x) + 40 * (i == 1 || i == 2) * 1.f,
					40 * (i >= 2) * 1.f,
				};
				arr[i].color = sf::Color::White;
				arr[i].position = (&box.pt1)[i];
			}
			game->screen->draw(arr, sprite.getTexture());
		}
	}

	void Character::render() const
	{
		if (this->_neutralEffectTimer)
			this->_sprite.setColor(sf::Color{0xA0, 0xA0, 0xA0});
		else if (this->_spiritEffectTimer)
			this->_sprite.setColor(sf::Color{51, 204, 204});
		else if (this->_matterEffectTimer)
			this->_sprite.setColor(sf::Color{187, 94, 0});
		else if (this->_voidEffectTimer)
			this->_sprite.setColor(sf::Color{0x80, 0x00, 0x80});
		else
			this->_sprite.setColor(sf::Color::White);

		auto &data = *this->getCurrentFrameData();
		auto result = Vector2f{data.offset.x * this->_dir, static_cast<float>(data.offset.y)} + this->_position;
		auto scale = Vector2f{
			this->_dir * static_cast<float>(data.size.x) / data.textureBounds.size.x,
			static_cast<float>(data.size.y) / data.textureBounds.size.y
		};

		result.y *= -1;
		result += Vector2f{
			!this->_direction * data.size.x + data.size.x / -2.f,
			-static_cast<float>(data.size.y)
		};
		result += Vector2f{
			data.textureBounds.size.x * scale.x / 2,
			data.textureBounds.size.y * scale.y / 2
		};

		Object::_render(result, scale);
		this->_effectTimer++;
		this->_renderExtraEffects(result);

		if (this->showBoxes) {
			if (isBlockingAction(this->_action))
				game->screen->displayElement({
					static_cast<int>(this->_position.x - this->_blockStun / 2),
					static_cast<int>(10 - this->_position.y),
					static_cast<int>(this->_blockStun),
					10
				}, sf::Color::White);
			else
				game->screen->displayElement(
					{
						static_cast<int>(this->_position.x - this->_blockStun / 2),
						static_cast<int>(10 - this->_position.y),
						static_cast<int>(this->_blockStun),
						10
					},
					this->_isGrounded() ? sf::Color::Red : sf::Color{0xFF, 0x80, 0x00}
				);
			game->screen->displayElement({
				static_cast<int>(this->_position.x - this->_hitStop),
				static_cast<int>(10 - this->_position.y + (this->_blockStun ? 15.f : 0.f)),
				static_cast<int>(this->_hitStop * 2),
				10
			}, sf::Color::Cyan);
		}
	}

	void Character::_processStallingFactor()
	{
		auto currentDist = this->_opponent->_position.distance(this->_position);
		auto nextDist = this->_opponent->_position.distance(this->_position + this->_speed);
		auto diffDist = nextDist - currentDist;

		if (diffDist < 0)
			this->_stallingFactor += diffDist * FORWARD_STALLING_FACTOR;
		else
			this->_stallingFactor += (diffDist * BACKING_STALLING_FACTOR + PASSIVE_STALLING_FACTOR) * (
				this->_position.y > MAX_STALLING_FACTOR_HEIGHT ? 1 :
				((this->_position.y * (1 - GROUND_STALLING_FACTOR) / MAX_STALLING_FACTOR_HEIGHT) + GROUND_STALLING_FACTOR)
			);
		if (this->_stallingFactor < MINIMUM_STALLING_STACKING)
			this->_stallingFactor = MINIMUM_STALLING_STACKING;
		if (this->_stallingFactor > MAXIMUM_STALLING_STACKING)
			this->_stallingFactor = MAXIMUM_STALLING_STACKING;
		if (this->_stallingFactor >= START_STALLING_THRESHOLD) {
			this->_mana -= METER_PENALTY_EQUATION(this->_stallingFactor, this->_manaMax);
			if (this->_mana < 0)
				this->_mana = 0;
		}
	}

	void Character::update()
	{
		this->_oldData = nullptr;
		this->_hasHitDuringFrame = false;
		this->_hasBeenHitDuringFrame = false;
		this->_gotHitStopReset = false;
		if (this->_hitStop) {
			this->_hitStop--;
			this->updateInputs();
			return;
		}
		if (this->_action == ACTION_IDLE) {
			if (this->_actionBlock == 0) {
				this->_timeSinceIdle++;
				if (this->_timeSinceIdle >= IDLE_ANIM_FIRST) {
					this->_timeSinceIdle = IDLE_ANIM_FIRST - random_distrib(game->battleRandom, IDLE_ANIM_CD_MIN, IDLE_ANIM_CD_MAX);
					if (this->_moves[0].size() != 1 && random_distrib(game->battleRandom, 0, 99) > IDLE_ANIM_CHANCE) {
						this->_actionBlock = random_distrib(game->battleRandom, 1, this->_moves[0].size() - 1);
						this->_animation = 0;
						this->_animationCtr = 0;
						this->_newAnim = true;
					}
				}
			}
		} else
			this->_timeSinceIdle = 0;

		auto input = this->_updateInputs();

		if (this->_matterInstallTimer)
			this->_matterInstallTimer--;
		if (this->_spiritInstallTimer)
			this->_spiritInstallTimer--;
		if (this->_voidInstallTimer)
			this->_voidInstallTimer--;
		if (this->_neutralEffectTimer)
			this->_neutralEffectTimer--;
		if (this->_matterEffectTimer)
			this->_matterEffectTimer--;
		if (this->_spiritEffectTimer)
			this->_spiritEffectTimer--;
		if (this->_voidEffectTimer) {
			if (this->_hp > 1) {
				this->_hp--;
				this->_totalDamage++;
			}
			this->_voidEffectTimer--;
		}
		if (!this->_ultimateUsed) {
			if (this->_odCooldown) {
				this->_odCooldown--;
				if (!this->_odCooldown)
					game->soundMgr.play(BASICSOUND_OVERDRIVE_RECOVER);
			}
			if (this->_guardCooldown) {
				this->_guardCooldown--;
				if (!this->_guardCooldown)
					game->soundMgr.play(BASICSOUND_GUARD_RECOVER);
				this->_guardRegenCd = 0;
			} else if (this->_guardRegenCd)
				this->_guardRegenCd--;
			else if (this->_guardBar < this->_maxGuardBar && !this->_matterEffectTimer) {
				this->_guardBar++;
				if (this->_guardBarTmp > 2)
					this->_guardBarTmp -= 2;
				else
					this->_guardBarTmp = 0;
				my_assert(this->_guardBar + this->_guardBarTmp / 2 <= this->_maxGuardBar);
			}
		} else {
			this->_barMaxOdCooldown = this->_maxOdCooldown;
			if (this->_odCooldown > 299 * this->_maxOdCooldown / 300)
				this->_odCooldown = this->_maxOdCooldown;
			else
				this->_odCooldown += this->_maxOdCooldown / 300;
			if (this->_guardCooldown > 299 * this->_maxGuardCooldown / 300)
				this->_guardCooldown = this->_maxGuardCooldown;
			else
				this->_guardCooldown += this->_maxGuardCooldown / 300;
		}

		if (!this->_ultimateUsed) {
			this->_mana += this->_manaMax * this->_regen;
			if (this->_mana > this->_manaMax)
				this->_mana = this->_manaMax;
		}

		this->_oldData = this->getCurrentFrameData();
		this->_tickMove();
		if (this->_limitEffects & VOID_LIMIT_EFFECT) {
			auto diff = this->_opponent->_position.x - this->_position.x;
			auto dist = std::abs(diff);

			this->_position.x += std::copysign(4, diff) / (std::pow(2, dist / 100));
		}
		if (this->_action == ACTION_FALLING_TECH && this->_isGrounded())
			this->_forceStartMove(ACTION_IDLE);
		if ((
			this->_action == ACTION_NEUTRAL_TECH ||
			this->_action == ACTION_FORWARD_TECH ||
			this->_action == ACTION_BACKWARD_TECH ||
			this->_action == ACTION_KNOCKED_DOWN ||
			this->_action == ACTION_BEING_KNOCKED_DOWN
		) && !this->_isGrounded())
			this->_forceStartMove(ACTION_FALLING_TECH);

		if (
			this->_action == ACTION_BEING_KNOCKED_DOWN ||
			this->_action == ACTION_KNOCKED_DOWN ||
			this->_action == ACTION_NEUTRAL_TECH ||
			this->_action == ACTION_FORWARD_TECH ||
			this->_action == ACTION_BACKWARD_TECH ||
			this->_action == ACTION_FALLING_TECH ||
			this->_action == ACTION_AIR_NEUTRAL_BLOCK ||
			this->_action == ACTION_AIR_NEUTRAL_WRONG_BLOCK ||
			this->_action == ACTION_GROUND_HIGH_NEUTRAL_BLOCK ||
			this->_action == ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK ||
			this->_action == ACTION_GROUND_LOW_NEUTRAL_BLOCK ||
			this->_action == ACTION_GROUND_LOW_NEUTRAL_WRONG_BLOCK
		)
			this->_grabInvul = GRAB_INVUL_STACK;
		else if (this->_grabInvul)
			this->_grabInvul--;

		auto limited = this->_limit[0] >= 100 || this->_limit[1] >= 100 || this->_limit[2] >= 100 || this->_limit[3] >= 100;

		if (this->_blockStun) {
			this->_blockStun--;
			if (this->_blockStun == 0) {
				if (!limited) {
					if (this->_isGrounded()) {
						if (this->_action != ACTION_AIR_HIT && this->_action != ACTION_GROUND_SLAM && this->_action != ACTION_WALL_SLAM)
							this->_forceStartMove(this->getCurrentFrameData()->dFlag.crouch ? ACTION_CROUCH : ACTION_IDLE);
					} else if (this->_restand || this->_action == ACTION_GROUND_HIGH_HIT || this->_action == ACTION_GROUND_LOW_HIT) {
						if (!this->_executeAirTech(this->_getInputs()))
							this->_forceStartMove(ACTION_FALLING);
					}
				}
				if (this->_restand) {
					if (this->_guardCooldown)
						;
					else if (this->_guardBar + TMP_GUARD_MAX / 2 >= this->_maxGuardBar)
						this->_guardBarTmp = (this->_maxGuardBar - this->_guardBar) * 2;
					else
						this->_guardBarTmp = TMP_GUARD_MAX;
					this->_restand = false;
				}
			}
		}

		if (
			(this->_action == ACTION_FORWARD_DASH || this->_action == ACTION_BACKWARD_DASH) &&
			this->_moves.at(this->_action).size() > 2 &&
			this->_actionBlock == 1 && (
			!this->_input->isPressed(
				this->_action == (this->_direction ? ACTION_BACKWARD_DASH : ACTION_FORWARD_DASH) ?
				INPUT_LEFT :
				INPUT_RIGHT
			) || !this->_isGrounded()
		)) {
			auto data = this->getCurrentFrameData();

			this->_actionBlock++;
			this->_animation = 0;
			this->_animationCtr = 0;
			my_assert2(this->_actionBlock != this->_moves.at(this->_action).size(), "Action " + actionToString(this->_action) + " is missing block 2");
			Object::_onMoveEnd(*data);
		}

		if (!this->_isGrounded() != this->getCurrentFrameData()->dFlag.airborne && this->getCurrentFrameData()->dFlag.landCancel) {
			if (this->_moves.at(this->_action).size() != this->_actionBlock + 1U) {
				this->_actionBlock++;
				this->_animation = 0;
				this->_animationCtr = 0;
				this->_newAnim = true;
			} else
				this->_forceStartMove(this->_isGrounded() ? ACTION_IDLE : ACTION_FALLING);
		}

		if (!this->_blockStun)
			this->_processInput(input);
		else if (isHitAction(this->_action)) {
			if (this->_isGrounded())
				(this->_specialInputs._an > 0 && this->_startMove(ACTION_NEUTRAL_OVERDRIVE)) ||
				(this->_specialInputs._am > 0 && this->_startMove(ACTION_MATTER_OVERDRIVE)) ||
				(this->_specialInputs._as > 0 && this->_startMove(ACTION_SPIRIT_OVERDRIVE)) ||
				(this->_specialInputs._av > 0 && this->_startMove(ACTION_VOID_OVERDRIVE));
			else
				(this->_specialInputs._an > 0 && this->_startMove(ACTION_NEUTRAL_AIR_OVERDRIVE)) ||
				(this->_specialInputs._am > 0 && this->_startMove(ACTION_MATTER_AIR_OVERDRIVE)) ||
				(this->_specialInputs._as > 0 && this->_startMove(ACTION_SPIRIT_AIR_OVERDRIVE)) ||
				(this->_specialInputs._av > 0 && this->_startMove(ACTION_VOID_AIR_OVERDRIVE));
		}
		this->_applyNewAnimFlags();
		this->_applyMoveAttributes();
		this->_processGroundSlams();
		this->_calculateCornerPriority();
		this->_processWallSlams();
		this->_processGroundedEvents();
		if (!isHitAction(this->_action) && !isBlockingAction(this->_action))
			this->_processStallingFactor();

		auto data = this->getCurrentFrameData();

		if (data->oFlag.turnAround && this->_opponent) {
			if (this->_opponent->_position.x - this->_position.x != 0)
				this->_dir = std::copysign(1, this->_opponent->_position.x - this->_position.x);
			this->_direction = this->_dir == 1;
		}
		if (
			!isHitAction(this->_action) &&
			this->_action != ACTION_UP_AIR_TECH &&
			this->_action != ACTION_DOWN_AIR_TECH &&
			this->_action != ACTION_FORWARD_AIR_TECH &&
			this->_action != ACTION_BACKWARD_AIR_TECH &&
			this->_action != ACTION_BEING_KNOCKED_DOWN &&
			this->_action != ACTION_KNOCKED_DOWN &&
			this->_action != ACTION_FORWARD_TECH &&
			this->_action != ACTION_NEUTRAL_TECH &&
			this->_action != ACTION_BACKWARD_TECH &&
			LIMIT_EFFECT_TIMER(this->_limitEffects)
		)
			DEC_LIMIT_EFFECT_TIMER(this->_limitEffects);
	}

	void Character::init(BattleManager &, const InitData &data)
	{
		this->_dir = data.side ? 1 : -1;
		this->_direction = data.side;
		this->_team = !data.side;
		this->_baseHp = this->_hp = data.maxHp;
		this->_maxJumps = data.maxJumps;
		this->_maxAirDashes = data.maxAirDash;
		this->_maxAirMovement = data.maxAirMovement;
		this->_baseGravity = this->_gravity = data.gravity;
		this->_manaMax = data.maxMana;
		this->_mana = data.startMana;
		this->_regen = data.manaRegen;
		this->_maxGuardCooldown = data.maxGuardCooldown;
		this->_guardBar = this->_maxGuardBar = data.maxGuardBar;
		this->_maxOdCooldown = data.odCd;
		this->_groundDrag = data.groundDrag;
		this->_airDrag = data.airDrag;
		this->_upDrift = data.upDrift;
		this->_downDrift = data.downDrift;
		this->_backDrift = data.backDrift;
		this->_frontDrift = data.frontDrift;
		if (data.side)
			this->_position = {200, 0};
		else
			this->_position = {800, 0};
	}

	void Character::consumeEvent(const sf::Event &event)
	{
		this->_input->consumeEvent(event);
	}

	void Character::_processAirDrift(const InputStruct &input)
	{
		if (input.verticalAxis > 0) {
			if (this->_speed.y < this->_upDrift.max) {
				this->_speed.y += this->_upDrift.accel;
				if (this->_speed.y > this->_upDrift.max)
					this->_speed.y = this->_upDrift.max;
			}
		}
		if (input.verticalAxis < 0) {
			if (this->_speed.y > this->_downDrift.max) {
				this->_speed.y -= this->_downDrift.accel;
				if (this->_speed.y < this->_downDrift.max)
					this->_speed.y = -this->_downDrift.max;
			}
		}
		if (input.horizontalAxis * this->_dir < 0) {
			if (this->_speed.x * this->_dir > -this->_backDrift.max) {
				this->_speed.x -= this->_backDrift.accel * this->_dir;
				if (this->_speed.x * this->_dir < -this->_backDrift.max)
					this->_speed.x = -this->_backDrift.max * this->_dir;
			}
		}
		if (input.horizontalAxis * this->_dir > 0) {
			if (this->_speed.x * this->_dir < this->_frontDrift.max) {
				this->_speed.x += this->_frontDrift.accel * this->_dir;
				if (this->_speed.x * this->_dir > this->_frontDrift.max)
					this->_speed.x = this->_frontDrift.max * this->_dir;
			}
		}
	}

	void Character::_processInput(InputStruct input)
	{
		auto data = this->getCurrentFrameData();
		auto airborne =
			(this->_action == ACTION_BACKWARD_AIR_JUMP || this->_action == ACTION_NEUTRAL_AIR_JUMP || this->_action == ACTION_FORWARD_AIR_JUMP) ?
			data->dFlag.airborne :
			!this->_isGrounded();

		if (airborne && (
			this->_action < ACTION_GROUND_HIGH_NEUTRAL_BLOCK ||
			this->_action == ACTION_NEUTRAL_AIR_JUMP ||
			this->_action == ACTION_FORWARD_AIR_JUMP ||
			this->_action == ACTION_BACKWARD_AIR_JUMP
		))
			this->_processAirDrift(input);
		if (this->_atkDisabled || this->_inputDisabled) {
			input.n = 0;
			input.m = 0;
			input.v = 0;
			input.s = 0;
			input.a = 0;
			if (this->_inputDisabled) {
				input.horizontalAxis = 0;
				input.verticalAxis = 0;
				input.d = 0;
			}
		}
		if ((this->_specialInputs._av > 0 || this->_specialInputs._as > 0 || this->_specialInputs._am > 0) && this->_action >= ACTION_5N && !isOverdriveAction(this->_action) && this->_mana >= INSTALL_COST) {
			this->_voidInstallTimer = 0;
			this->_spiritInstallTimer = 0;
			this->_matterInstallTimer = 0;
			if (this->_specialInputs._av > 0)
				this->_voidInstallTimer = INSTALL_DURATION;
			else if (this->_specialInputs._as > 0)
				this->_spiritInstallTimer = INSTALL_DURATION;
			else if (this->_specialInputs._am > 0)
				this->_matterInstallTimer = INSTALL_DURATION;
			this->_mana -= INSTALL_COST;
			this->_specialInputs._am = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._as = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._av = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_inputBuffer.a = 0;
			game->soundMgr.play(BASICSOUND_INSTALL_START);
		}
		if (
			(airborne && this->_executeAirborneMoves(input)) ||
			(!airborne && this->_executeGroundMoves(input))
		)
			return;
		if (this->_isGrounded())
			this->_startMove(this->_action == ACTION_CROUCH ? ACTION_STANDING_UP : ACTION_IDLE);
		else
			this->_startMove(ACTION_FALLING);
	}

	InputStruct Character::updateInputs()
	{
		return this->_updateInputs(false);
	}

	InputStruct Character::_updateInputs(bool tickBuffer)
	{
		this->_input->update();

		InputStruct input = this->_getInputs();

		if (
			!!input.n != this->_lastInputs.front().n ||
			!!input.m != this->_lastInputs.front().m ||
			!!input.s != this->_lastInputs.front().s ||
			!!input.v != this->_lastInputs.front().o ||
			!!input.d != this->_lastInputs.front().d ||
			!!input.a != this->_lastInputs.front().a ||
			std::copysign(!!input.horizontalAxis, this->_dir * input.horizontalAxis) != this->_lastInputs.front().h ||
			std::copysign(!!input.verticalAxis,   input.verticalAxis)                != this->_lastInputs.front().v
		)
			this->_lastInputs.push_front({
				0,
				!!input.n,
				!!input.m,
				!!input.s,
				!!input.v,
				!!input.d,
				!!input.a,
				static_cast<char>(std::copysign(!!input.horizontalAxis, this->_dir * input.horizontalAxis)),
				static_cast<char>(std::copysign(!!input.verticalAxis,   input.verticalAxis)),
			});
		this->_lastInputs.front().nbFrames++;
		if (this->_lastInputs.front().nbFrames > MAX_FRAME_IN_BUFFER)
			this->_lastInputs.front().nbFrames = MAX_FRAME_IN_BUFFER;

		if (
			this->_replayData.empty() ||
			!!input.n != this->_replayData.back().n ||
			!!input.m != this->_replayData.back().m ||
			!!input.s != this->_replayData.back().s ||
			!!input.v != this->_replayData.back().v ||
			!!input.d != this->_replayData.back().d ||
			!!input.a != this->_replayData.back().a ||
			std::copysign(!!input.horizontalAxis, input.horizontalAxis) != this->_replayData.back()._h ||
			std::copysign(!!input.verticalAxis,   input.verticalAxis)   != this->_replayData.back()._v ||
			this->_replayData.back().time == 63
		)
			this->_replayData.push_back({
				!!input.n,
				!!input.m,
				!!input.v,
				!!input.s,
				!!input.a,
				!!input.d,
				static_cast<char>(std::copysign(!!input.horizontalAxis, input.horizontalAxis)),
				static_cast<char>(std::copysign(!!input.verticalAxis,   input.verticalAxis)),
				0
			});
		else
			this->_replayData.back().time++;

		this->_checkSpecialInputs(tickBuffer);
		this->_hasJumped &= input.verticalAxis > 0;
		this->_inputBuffer.horizontalAxis = input.horizontalAxis;
		this->_inputBuffer.verticalAxis = input.verticalAxis;
		for (unsigned i = 2; i < sizeof(this->_inputBuffer) / sizeof(int); i++) {
			if (((int *)&this->_inputBuffer)[i])
				((int *)&this->_inputBuffer)[i] -= tickBuffer;
			else if (std::abs(((int *)&input)[i]) == 1)
				((int *)&this->_inputBuffer)[i] = NORMAL_BUFFER;
		}
		return this->_inputBuffer;
	}

	bool Character::_executeAirborneMoves(const InputStruct &input)
	{
		return  //(input.n && input.n <= 4 && this->_startMove(ACTION_j5N));
		        this->_executeAirTech(input) ||

		        (this->_specialInputs._624684n && this->_startMove(ACTION_j6321469874N)) ||
		        (this->_specialInputs._624684v && this->_startMove(ACTION_j6321469874V)) ||
		        (this->_specialInputs._624684s && this->_startMove(ACTION_j6321469874S)) ||
		        (this->_specialInputs._624684m && this->_startMove(ACTION_j6321469874M)) ||
		        (this->_specialInputs._624684d && this->_startMove(ACTION_j6321469874D)) ||
		        (this->_specialInputs._624684a && this->_startMove(ACTION_j6321469874A)) ||

		        (this->_specialInputs._624n && this->_startMove(ACTION_j63214N)) ||
		        (this->_specialInputs._426n && this->_startMove(ACTION_j41236N)) ||
		        (this->_specialInputs._624v && this->_startMove(ACTION_j63214V)) ||
		        (this->_specialInputs._426v && this->_startMove(ACTION_j41236V)) ||
		        (this->_specialInputs._624s && this->_startMove(ACTION_j63214S)) ||
		        (this->_specialInputs._426s && this->_startMove(ACTION_j41236S)) ||
		        (this->_specialInputs._624m && this->_startMove(ACTION_j63214M)) ||
		        (this->_specialInputs._426m && this->_startMove(ACTION_j41236M)) ||
		        (this->_specialInputs._624d && this->_startMove(ACTION_j63214D)) ||
		        (this->_specialInputs._426d && this->_startMove(ACTION_j41236D)) ||
		        (this->_specialInputs._624a && this->_startMove(ACTION_j63214A)) ||
		        (this->_specialInputs._426a && this->_startMove(ACTION_j41236A)) ||

		        (this->_specialInputs._623n && this->_startMove(ACTION_j623N)) ||
		        (this->_specialInputs._421n && this->_startMove(ACTION_j421N)) ||
		        (this->_specialInputs._623v && this->_startMove(ACTION_j623V)) ||
		        (this->_specialInputs._421v && this->_startMove(ACTION_j421V)) ||
		        (this->_specialInputs._623s && this->_startMove(ACTION_j623S)) ||
		        (this->_specialInputs._421s && this->_startMove(ACTION_j421S)) ||
		        (this->_specialInputs._623m && this->_startMove(ACTION_j623M)) ||
		        (this->_specialInputs._421m && this->_startMove(ACTION_j421M)) ||
		        (this->_specialInputs._623d && this->_startMove(ACTION_j623D)) ||
		        (this->_specialInputs._421d && this->_startMove(ACTION_j421D)) ||
		        (this->_specialInputs._623a && this->_startMove(ACTION_j623A)) ||
		        (this->_specialInputs._421a && this->_startMove(ACTION_j421A)) ||

		        (this->_specialInputs._236n && this->_startMove(ACTION_j236N)) ||
		        (this->_specialInputs._214n && this->_startMove(ACTION_j214N)) ||
		        (this->_specialInputs._236v && this->_startMove(ACTION_j236V)) ||
		        (this->_specialInputs._214v && this->_startMove(ACTION_j214V)) ||
		        (this->_specialInputs._236s && this->_startMove(ACTION_j236S)) ||
		        (this->_specialInputs._214s && this->_startMove(ACTION_j214S)) ||
		        (this->_specialInputs._236m && this->_startMove(ACTION_j236M)) ||
		        (this->_specialInputs._214m && this->_startMove(ACTION_j214M)) ||
		        (this->_specialInputs._236d && this->_startMove(ACTION_j236D)) ||
		        (this->_specialInputs._214d && this->_startMove(ACTION_j214D)) ||
		        (this->_specialInputs._236a && this->_startMove(ACTION_j236A)) ||
		        (this->_specialInputs._214a && this->_startMove(ACTION_j214A)) ||

		        (this->_specialInputs._an > 0 && this->_startMove(ACTION_AIR_ROMAN_CANCEL)) ||

		        this->_executeAirParry(input) ||

		        (input.n && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8N)) ||
		        (input.n && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3N)) ||
		        (input.n &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6N)) ||
		        (input.n && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_j5N)) ||
		        (input.n &&                                                                      this->_executeNeutralAttack(ACTION_j5N)) ||
		        (input.v && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8V)) ||
		        (input.v && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3V)) ||
		        (input.v &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6V)) ||
		        (input.v && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_j5V)) ||
		        (input.v &&                                                                      this->_executeNeutralAttack(ACTION_j5V)) ||
		        (input.s && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8S)) ||
		        (input.s && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3S)) ||
		        (input.s &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6S)) ||
		        (input.s && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_j5S)) ||
		        (input.s &&                                                                      this->_executeNeutralAttack(ACTION_j5S)) ||
		        (input.m && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8M)) ||
		        (input.m && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3M)) ||
		        (input.m &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6M)) ||
		        (input.m && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_j5M)) ||
		        (input.m &&                                                                      this->_executeNeutralAttack(ACTION_j5M)) ||
		        (input.a && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8A)) ||
		        (input.a && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3A)) ||
		        (input.a &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6A)) ||
		        (input.a && input.verticalAxis < 0 && this->_dir * input.horizontalAxis == 0 &&  this->_executeDownAttack(ACTION_j5A)) ||
		        (input.a &&                           this->_dir * input.horizontalAxis == 0 &&  this->_executeNeutralAttack(ACTION_j5A)) ||
		        this->_executeAirDashes(input) ||
		        this->_executeAirJump(input);
	}

	bool Character::_executeGroundMoves(const InputStruct &input)
	{
		return  //(input.n && input.n <= 4 && this->_startMove(ACTION_5N)) ||

		        (this->_specialInputs._624684n && this->_startMove(ACTION_6321469874N)) ||
		        (this->_specialInputs._624684v && this->_startMove(ACTION_6321469874V)) ||
		        (this->_specialInputs._624684s && this->_startMove(ACTION_6321469874S)) ||
		        (this->_specialInputs._624684m && this->_startMove(ACTION_6321469874M)) ||
		        (this->_specialInputs._624684d && this->_startMove(ACTION_6321469874D)) ||
		        (this->_specialInputs._624684a && this->_startMove(ACTION_6321469874A)) ||

		        (this->_specialInputs._624n && this->_startMove(ACTION_63214N)) ||
		        (this->_specialInputs._426n && this->_startMove(ACTION_41236N)) ||
		        (this->_specialInputs._624v && this->_startMove(ACTION_63214V)) ||
		        (this->_specialInputs._426v && this->_startMove(ACTION_41236V)) ||
		        (this->_specialInputs._624s && this->_startMove(ACTION_63214S)) ||
		        (this->_specialInputs._426s && this->_startMove(ACTION_41236S)) ||
		        (this->_specialInputs._624m && this->_startMove(ACTION_63214M)) ||
		        (this->_specialInputs._426m && this->_startMove(ACTION_41236M)) ||
		        (this->_specialInputs._624d && this->_startMove(ACTION_63214D)) ||
		        (this->_specialInputs._426d && this->_startMove(ACTION_41236D)) ||
		        (this->_specialInputs._624a && this->_startMove(ACTION_63214A)) ||
		        (this->_specialInputs._426a && this->_startMove(ACTION_41236A)) ||

		        (this->_specialInputs._c28n && this->_startMove(ACTION_c28N)) ||
		        (this->_specialInputs._c46n && this->_startMove(ACTION_c46N)) ||
		        (this->_specialInputs._c64n && this->_startMove(ACTION_c64N)) ||
		        (this->_specialInputs._c28v && this->_startMove(ACTION_c28V)) ||
		        (this->_specialInputs._c46v && this->_startMove(ACTION_c46V)) ||
		        (this->_specialInputs._c64v && this->_startMove(ACTION_c64V)) ||
		        (this->_specialInputs._c28s && this->_startMove(ACTION_c28S)) ||
		        (this->_specialInputs._c46s && this->_startMove(ACTION_c46S)) ||
		        (this->_specialInputs._c64s && this->_startMove(ACTION_c64S)) ||
		        (this->_specialInputs._c28m && this->_startMove(ACTION_c28M)) ||
		        (this->_specialInputs._c46m && this->_startMove(ACTION_c46M)) ||
		        (this->_specialInputs._c64m && this->_startMove(ACTION_c64M)) ||
		        (this->_specialInputs._c46a && this->_startMove(ACTION_c46A)) ||
		        (this->_specialInputs._c64a && this->_startMove(ACTION_c64A)) ||

		        (this->_specialInputs._623n     && this->_startMove(ACTION_623N)) ||
		        (this->_specialInputs._421n > 0 && this->_startMove(ACTION_421N)) ||
		        (this->_specialInputs._623v     && this->_startMove(ACTION_623V)) ||
		        (this->_specialInputs._421v > 0 && this->_startMove(ACTION_421V)) ||
		        (this->_specialInputs._623s     && this->_startMove(ACTION_623S)) ||
		        (this->_specialInputs._421s > 0 && this->_startMove(ACTION_421S)) ||
		        (this->_specialInputs._623m     && this->_startMove(ACTION_623M)) ||
		        (this->_specialInputs._421m > 0 && this->_startMove(ACTION_421M)) ||
		        (this->_specialInputs._623d     && this->_startMove(ACTION_623D)) ||
		        (this->_specialInputs._421d     && this->_startMove(ACTION_421D)) ||
		        (this->_specialInputs._623a     && this->_startMove(ACTION_623A)) ||
		        (this->_specialInputs._421a     && this->_startMove(ACTION_421A)) ||

		        (this->_specialInputs._236n && this->_startMove(ACTION_236N)) ||
		        (this->_specialInputs._214n && this->_startMove(ACTION_214N)) ||
		        (this->_specialInputs._236v && this->_startMove(ACTION_236V)) ||
		        (this->_specialInputs._214v && this->_startMove(ACTION_214V)) ||
		        (this->_specialInputs._236s && this->_startMove(ACTION_236S)) ||
		        (this->_specialInputs._214s && this->_startMove(ACTION_214S)) ||
		        (this->_specialInputs._236m && this->_startMove(ACTION_236M)) ||
		        (this->_specialInputs._214m && this->_startMove(ACTION_214M)) ||
		        (this->_specialInputs._236d && this->_startMove(ACTION_236D)) ||
		        (this->_specialInputs._214d && this->_startMove(ACTION_214D)) ||
		        (this->_specialInputs._236a && this->_startMove(ACTION_236A)) ||
		        (this->_specialInputs._214a && this->_startMove(ACTION_214A)) ||

		        (this->_specialInputs._an > 0 && this->_startMove(ACTION_ROMAN_CANCEL)) ||
		        this->_executeGroundParry(input) ||

		        (input.n && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8N)) ||
		        (input.n && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3N)) ||
		        (input.n &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6N)) ||
		        (input.n && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_5N)) ||
		        (input.n &&                                                                      this->_executeNeutralAttack(ACTION_5N)) ||
		        (input.v && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8V)) ||
		        (input.v && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3V)) ||
		        (input.v &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6V)) ||
		        (input.v && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_5V)) ||
		        (input.v &&                                                                      this->_executeNeutralAttack(ACTION_5V)) ||
		        (input.s && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8S)) ||
		        (input.s && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3S)) ||
		        (input.s &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6S)) ||
		        (input.s && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_5S)) ||
		        (input.s &&                                                                      this->_executeNeutralAttack(ACTION_5S)) ||
		        (input.m && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8M)) ||
		        (input.m && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3M)) ||
		        (input.m &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6M)) ||
		        (input.m && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_5M)) ||
		        (input.m &&                                                                      this->_executeNeutralAttack(ACTION_5M)) ||
		        (input.a && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8A)) ||
		        (input.a && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3A)) ||
		        (input.a &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6A)) ||
		        (input.a && input.verticalAxis < 0 && this->_dir * input.horizontalAxis == 0 &&  this->_executeDownAttack(ACTION_5A)) ||
		        (input.a &&                           this->_dir * input.horizontalAxis == 0 &&  this->_executeNeutralAttack(ACTION_5A)) ||
		        this->_executeGroundJump(input)   ||
		        this->_executeGroundDashes(input) ||
		        this->_executeCrouch(input)       ||
		        this->_executeWalking(input);
	}

	bool Character::_canStartMove(unsigned action, const FrameData &data)
	{
		if (
			this->_jumpCanceled &&
			!this->getCurrentFrameData()->oFlag.cancelable && (
				this->_action == ACTION_NEUTRAL_JUMP ||
				this->_action == ACTION_FORWARD_JUMP ||
				this->_action == ACTION_BACKWARD_JUMP ||
				this->_action == ACTION_NEUTRAL_HIGH_JUMP ||
				this->_action == ACTION_FORWARD_HIGH_JUMP ||
				this->_action == ACTION_BACKWARD_HIGH_JUMP ||
				this->_action == ACTION_NEUTRAL_AIR_JUMP ||
				this->_action == ACTION_FORWARD_AIR_JUMP ||
				this->_action == ACTION_BACKWARD_AIR_JUMP
			) && (
				(
					(action % 50 + 100) != ACTION_8N &&
					(action % 50 + 100) != ACTION_j8N
				) || !isHitAction(this->_opponent->_action)
			)
		)
			return false;
		if (isOverdriveAction(action)) {
			for (auto limit : this->_limit)
				if (limit >= 100)
					return false;
			return !this->_odCooldown;
		}
		if (isRomanCancelAction(action))
			return !this->_odCooldown && this->_action >= ACTION_5N && !isParryAction(this->_action) && !isRomanCancelAction(this->_action) && !isOverdriveAction(this->_action) && !isReversalAction(this->_action);
		if (this->_hp <= 0 && this->_action == ACTION_KNOCKED_DOWN)
			return false;
		if (data.subObjectSpawn < 0 && data.subObjectSpawn >= -128 && this->_subobjects[-data.subObjectSpawn - 1].first)
			return false;
		if (data.oFlag.ultimate && this->_ultimateUsed && this->_mana / this->_manaMax <= 0.1f)
			return false;
		if (this->_mana < data.manaCost)
			return false;
		if (data.manaCost && LIMIT_EFFECT_TIMER(this->_limitEffects))
			return false;
		if (action == ACTION_UP_AIR_TECH || action == ACTION_DOWN_AIR_TECH || action == ACTION_FORWARD_AIR_TECH || action == ACTION_BACKWARD_AIR_TECH) {
			for (auto limit : this->_limit)
				if (limit >= 100)
					return false;
			return (this->_action == ACTION_AIR_HIT || this->_action == ACTION_GROUND_SLAM || this->_action == ACTION_WALL_SLAM) && this->_blockStun == 0;
		}
		if (action == ACTION_NEUTRAL_TECH || action == ACTION_BACKWARD_TECH || action == ACTION_FORWARD_TECH)
			return this->_isGrounded() && this->_action == ACTION_KNOCKED_DOWN;
		if (action == ACTION_FALLING_TECH)
			return !this->_isGrounded() && (this->_action == ACTION_NEUTRAL_TECH || this->_action == ACTION_FORWARD_TECH || this->_action == ACTION_BACKWARD_TECH);
		if (action == ACTION_IDLE && this->_action == ACTION_STANDING_UP)
			return false;
		if (action == ACTION_CROUCHING && this->_action == ACTION_CROUCH)
			return false;
		if (this->_canCancel(action))
			return true;
		if (action >= ACTION_AIR_DASH_1 && action <= ACTION_AIR_DASH_9)
			return this->_airDashesUsed < this->_maxAirDashes && isMovementCancelable(this->_action, this->getCurrentFrameData()->oFlag.cancelable) && this->_airMovementUsed < this->_maxAirMovement;
		if ((action >= ACTION_NEUTRAL_JUMP && action <= ACTION_BACKWARD_HIGH_JUMP) || (action >= ACTION_NEUTRAL_AIR_JUMP && action <= ACTION_BACKWARD_AIR_JUMP))
			return this->_jumpsUsed < this->_maxJumps && this->_airMovementUsed < this->_maxAirMovement && isMovementCancelable(this->_action, this->getCurrentFrameData()->oFlag.cancelable);
		if (this->_action == action)
			return false;
		if (isBlockingAction(action))
			return this->getCurrentFrameData()->dFlag.canBlock;
		if (isParryAction(action) && this->getCurrentFrameData()->dFlag.canBlock)
			return !this->_guardCooldown && !this->_blockStun && !isParryAction(this->_action);
		if (isBlockingAction(this->_action))
			return !this->_blockStun;
		if (action <= ACTION_WALK_BACKWARD || action == ACTION_FALLING || action == ACTION_LANDING)
			return (action || this->_action != ACTION_LANDING) && (this->_action <= ACTION_WALK_BACKWARD || this->_action == ACTION_FALLING || this->_action == ACTION_LANDING);
		if (this->_action <= ACTION_WALK_BACKWARD || this->_action == ACTION_FALLING || this->_action == ACTION_LANDING)
			return true;
		if (
			(
				// Can cancel jumpsquat by move
				(this->getCurrentFrameData()->dFlag.grabInvulnerable && (
					this->_action == ACTION_NEUTRAL_JUMP ||
					this->_action == ACTION_FORWARD_JUMP ||
					this->_action == ACTION_BACKWARD_JUMP ||
					this->_action == ACTION_NEUTRAL_HIGH_JUMP ||
					this->_action == ACTION_FORWARD_HIGH_JUMP ||
					this->_action == ACTION_BACKWARD_HIGH_JUMP ||
					this->_action == ACTION_NEUTRAL_AIR_JUMP ||
					this->_action == ACTION_FORWARD_AIR_JUMP ||
					this->_action == ACTION_BACKWARD_AIR_JUMP
				)) ||
				// Can cancel jumps by moves
				isMovementCancelable(this->_action, this->getCurrentFrameData()->oFlag.cancelable)
			) && action >= 100
		)
			return true;
		return false;
	}

	void Character::_onMoveEnd(const FrameData &lastData)
	{
		game->logger.verbose(std::to_string(this->_action) + " ended");
		if (this->_action == ACTION_BEING_KNOCKED_DOWN) {
			this->_blockStun = 0;
			return this->_forceStartMove(ACTION_KNOCKED_DOWN);
		}
		if (
			(
				this->_action == ACTION_AIR_HIT ||
				this->_action == ACTION_FALLING
			) &&
			this->_actionBlock == 1 &&
			this->_moves.at(this->_action).size() > 2
		) {
			this->_actionBlock++;
			this->_animation = 0;
			this->_animationCtr = 0;
			this->_newAnim = true;
			return;
		}

		if ((this->_action == ACTION_FORWARD_DASH || this->_action == ACTION_BACKWARD_DASH) && this->_moves.at(this->_action).size() > 1) {
			if (this->_actionBlock == 0)
				this->_actionBlock++;
			if (this->_actionBlock == 1 && this->_moves.at(this->_action).size() > 2)
				return Object::_onMoveEnd(lastData);
		}

		if ((this->_action == ACTION_FALLING_TECH || this->_blockStun) && !this->_actionBlock && this->_action != ACTION_AIR_HIT) {
			this->_actionBlock++;
			my_assert2(this->_moves.at(this->_action).size() > 1, "Action " + actionToString(this->_action) + " is missing block 1");
			Object::_onMoveEnd(lastData);
			return;
		}

		if (this->_action == ACTION_KNOCKED_DOWN) {
			if (this->_hp <= 0)
				return Object::_onMoveEnd(lastData);

			auto inputs = this->_getInputs();

			if (this->_hardKD || (this->_limitEffects & MATTER_LIMIT_EFFECT)) {
				this->_forceStartMove(ACTION_NEUTRAL_TECH);
				this->_actionBlock = 1;
				my_assert2(this->_moves.at(this->_action).size() > 1, "Action " + actionToString(this->_action) + " is missing block 1");
				return;
			}

			switch (this->_dummyGroundTech) {
			case GROUNDTECH_NONE:
				break;
			case GROUNDTECH_FORWARD:
				if (this->_startMove(ACTION_FORWARD_TECH))
					return;
				break;
			case GROUNDTECH_BACKWARD:
				if (this->_startMove(ACTION_BACKWARD_TECH))
					return;
				break;
			case GROUNDTECH_RANDOM:
				switch (game->random() % 3) {
				case 0:
					break;
				case 1:
					if (this->_startMove(ACTION_FORWARD_TECH))
						return;
					break;
				case 2:
					if (this->_startMove(ACTION_BACKWARD_TECH))
						return;
					break;
				}
				break;
			}
			if (this->_atkDisabled || this->_inputDisabled || (!inputs.a && !inputs.s && !inputs.d && !inputs.m && !inputs.n && !inputs.v) || !inputs.horizontalAxis)
				return this->_forceStartMove(ACTION_NEUTRAL_TECH);
			if (this->_startMove(inputs.horizontalAxis * this->_dir < 0 ? ACTION_BACKWARD_TECH : ACTION_FORWARD_TECH))
				return;
			return this->_forceStartMove(ACTION_NEUTRAL_TECH);
		}

		if (this->_action == ACTION_CROUCHING)
			return this->_forceStartMove(ACTION_CROUCH);

		auto idleAction = this->_isGrounded() ? (lastData.dFlag.crouch ? ACTION_CROUCH : ACTION_IDLE) : ACTION_FALLING;

		if (this->_action == ACTION_BACKWARD_AIR_TECH || this->_action == ACTION_FORWARD_AIR_TECH || this->_action == ACTION_UP_AIR_TECH || this->_action == ACTION_DOWN_AIR_TECH)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_AIR_REVERSAL || this->_action == ACTION_GROUND_HIGH_REVERSAL || this->_action == ACTION_GROUND_LOW_REVERSAL)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_LANDING)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_BACKWARD_TECH || this->_action == ACTION_FORWARD_TECH || this->_action == ACTION_NEUTRAL_TECH)
			return this->_forceStartMove(idleAction);
		if (isParryAction(this->_action))
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_STANDING_UP)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_AIR_TECH_LANDING_LAG)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_FORWARD_DASH)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_BACKWARD_DASH)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_HARD_LAND)
			return this->_forceStartMove(idleAction);
		if (
			(this->_action >= ACTION_5N && this->_action < ACTION_WIN_MATCH1) ||
			this->_action == ACTION_LANDING
		)
			return this->_forceStartMove(idleAction);

		if (this->_action >= ACTION_AIR_DASH_1 && this->_action <= ACTION_AIR_DASH_9)
			return this->_forceStartMove(this->_isGrounded() ? ACTION_HARD_LAND : ACTION_FALLING);
		if (this->_action == ACTION_NEUTRAL_JUMP || this->_action == ACTION_FORWARD_JUMP || this->_action == ACTION_BACKWARD_JUMP)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_NEUTRAL_AIR_JUMP || this->_action == ACTION_FORWARD_AIR_JUMP || this->_action == ACTION_BACKWARD_AIR_JUMP)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_NEUTRAL_HIGH_JUMP || this->_action == ACTION_FORWARD_HIGH_JUMP || this->_action == ACTION_BACKWARD_HIGH_JUMP)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_IDLE)
			this->_actionBlock = 0;
		Object::_onMoveEnd(lastData);
	}

	void Character::hit(Object &other, const FrameData *data)
	{
		auto chr = dynamic_cast<Character *>(&other);
		auto isHit = isHitAction(other._action);
		auto realData = this->getCurrentFrameData();

		if (!this->_hasBeenHitDuringFrame)
			this->_speed.x += data->pushBack * -this->_dir;
		this->_hasHitDuringFrame = true;
		if (!this->_hasHit && (
			(realData->oFlag.nextBlockOnHit && (!chr || isHit)) ||
			(realData->oFlag.nextBlockOnBlock && chr && !isHit)
		)) {
			this->_actionBlock++;
			my_assert2(this->_actionBlock != this->_moves.at(this->_action).size(), "Action " + actionToString(this->_action) + " is missing block " + std::to_string(this->_actionBlock));
			this->_animationCtr = 0;
			Object::_onMoveEnd(*data);
			Object::hit(other, data);
			this->_newAnim = true;
		} else
			Object::hit(other, data);
		if (
			chr &&
			(isHitAction(chr->_action) || isBlockingAction(chr->_action)) &&
			this->_stallingFactor < STALLING_BLOCK_WIPE_THRESHOLD &&
			this->_stallingFactor > 0
		)
			this->_stallingFactor = 0;
		else if (chr && isHitAction(chr->_action))
			this->_stallingFactor = maxi(MINIMUM_STALLING_STACKING, this->_stallingFactor - STALLING_HIT_REMOVE);
		else if (chr && isBlockingAction(chr->_action))
			this->_stallingFactor = maxi(MINIMUM_STALLING_STACKING, this->_stallingFactor - STALLING_BLOCK_REMOVE);
	}

	void Character::getHit(Object &other, const FrameData *dat)
	{
		char buffer[48];

		sprintf(buffer, "0x%08llX is hit by 0x%08llX", (unsigned long long)this, (unsigned long long)&other);
		game->logger.debug(buffer);
		if (!dat)
			return;

		FrameData data;
		auto myData = this->getCurrentFrameData();

		data.setSlave();
		data = *dat;
		this->_mutateHitFramedata(data);
		if (myData->dFlag.invulnerableArmor) {
			other._hitStop += data.hitPlayerHitStop;
			this->_hitStop += data.hitOpponentHitStop;
			return;
		}
		this->_restand = false;
		if (
			!this->_isBlocking() ||
			(myData->dFlag.airborne && data.oFlag.airUnblockable) ||
			data.oFlag.unblockable ||
			data.oFlag.grab
		)
			this->_getHitByMove(&other, data);
		else
			this->_blockMove(&other, data);
	}

	bool Character::_isBlocking() const
	{
		auto *data = this->getCurrentFrameData();
		auto dir = std::copysign(1, this->_opponent->_position.x - this->_position.x);

		if (this->_input->isPressed(dir == 1 ? INPUT_LEFT : INPUT_RIGHT) && data->dFlag.canBlock)
			return true;
		if ((this->_forceBlock & 7) && ((this->_forceBlock & RANDOM_BLOCK) == 0 || game->random() % 8 != 0) && data->dFlag.canBlock)
			return true;
		if (data->dFlag.neutralBlock || data->dFlag.spiritBlock || data->dFlag.matterBlock || data->dFlag.voidBlock)
			return true;
		if (data->dFlag.lowBlock || data->dFlag.highBlock)
			return true;
		return isBlockingAction(this->_action);
	}

	void Character::_forceStartMove(unsigned int action)
	{
		my_assert2(this->_moves.find(action) != this->_moves.end(), "Invalid action: Action " + actionToString(action) + " was not found.");

		auto anim = this->_moves.at(this->_action)[this->_actionBlock].size() == this->_animation ? this->_animation - 1 : this->_animation;
		auto &data = this->_moves[action][0][0];

		if (data.oFlag.turnAround && this->_opponent) {
			if (this->_opponent->_position.x - this->_position.x != 0)
				this->_dir = std::copysign(1, this->_opponent->_position.x - this->_position.x);
			this->_direction = this->_dir == 1;
		}

		if (
			action == ACTION_AIR_HIT &&
			this->_speed.y > 0 &&
			this->_moves.at(action).size() > 2
		) {
			Object::_forceStartMove(action);
			this->_actionBlock = 2;
			return;
		}
		if (action >= ACTION_5N)
			this->_guardBarTmp = 0;
		if (this->_opponent->_comboCtr) {
			auto it = this->_usedMoves.find(this->_action);

			if (it == this->_usedMoves.end())
				this->_usedMoves[this->_action] = 1;
			else
				it->second++;
		}
		this->startedAttack |= action >= ACTION_5N;
		this->_armorUsed = false;
		this->_jumpCanceled = this->_moves.at(this->_action)[this->_actionBlock][anim].oFlag.jumpCancelable && (
			action == ACTION_NEUTRAL_JUMP ||
			action == ACTION_FORWARD_JUMP ||
			action == ACTION_BACKWARD_JUMP ||
			action == ACTION_NEUTRAL_HIGH_JUMP ||
			action == ACTION_FORWARD_HIGH_JUMP ||
			action == ACTION_BACKWARD_HIGH_JUMP ||
			action == ACTION_NEUTRAL_AIR_JUMP ||
			action == ACTION_FORWARD_AIR_JUMP ||
			action == ACTION_BACKWARD_AIR_JUMP
		);
		game->logger.debug("Starting action " + actionToString(action));
		if (isParryAction(action)) {
			unsigned loss = ((action == ACTION_AIR_NEUTRAL_PARRY || action == ACTION_GROUND_HIGH_NEUTRAL_PARRY || action == ACTION_GROUND_LOW_NEUTRAL_PARRY) + 1) * PARRY_COST;

			this->_specialInputs._an = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._am = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._as = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._av = -SPECIAL_INPUT_BUFFER_PERSIST;
			game->soundMgr.play(BASICSOUND_PARRY);
			this->_reduceGuard(loss, REGEN_CD_PARRY, true);
		}

		// Bits magic to allow the semi cyclic cancel tree (S -> M -> V -> S -> ...) to work
		// The 3 bit combinations are as follows:
		//    When starting with S: S 0111 -> M 1001 -> V 1110
		//    When starting with M: M 0101 -> V 1010 -> S 1111
		//    When starting with V: V 0110 -> S 1011 -> M 1101
		// This works by using bits 0-1 to store which type was used last (0 is none, 1 is M, 2 is V and 3 is S)
		// and bits 2-3 to store how many cancels have occurred (0 to 3)
		// The cancel function then adds some cancel priority based on these.
		switch (action) {
		case ACTION_6S:
		case ACTION_8S:
		case ACTION_3S:
		case ACTION_j6S:
		case ACTION_j8S:
		case ACTION_j3S:
		case ACTION_j2S:
			this->_normalTreeFlag += 4;
			this->_normalTreeFlag = this->_normalTreeFlag | 3;
			break;
		case ACTION_6M:
		case ACTION_8M:
		case ACTION_3M:
		case ACTION_j6M:
		case ACTION_j8M:
		case ACTION_j3M:
		case ACTION_j2M:
			this->_normalTreeFlag += 4;
			this->_normalTreeFlag = (this->_normalTreeFlag & ~3) | 1;
			break;
		case ACTION_6V:
		case ACTION_8V:
		case ACTION_3V:
		case ACTION_j6V:
		case ACTION_j8V:
		case ACTION_j3V:
		case ACTION_j2V:
			this->_normalTreeFlag += 4;
			this->_normalTreeFlag = (this->_normalTreeFlag & ~3) | 2;
			break;
		}

		if (action < ACTION_5N)
			this->_normalTreeFlag = 0;

		if (
			action == ACTION_IDLE ||
			action == ACTION_WALK_FORWARD ||
			action == ACTION_WALK_BACKWARD ||
			action == ACTION_NEUTRAL_JUMP ||
			action == ACTION_FORWARD_JUMP ||
			action == ACTION_BACKWARD_JUMP ||
			action == ACTION_NEUTRAL_HIGH_JUMP ||
			action == ACTION_FORWARD_HIGH_JUMP ||
			action == ACTION_BACKWARD_HIGH_JUMP
		) {
			if (this->_moves.at(this->_action)[this->_actionBlock][anim].dFlag.airborne) {
				game->soundMgr.play(BASICSOUND_LAND);
				if (this->_action == ACTION_FALLING && action == ACTION_IDLE)
					return this->_forceStartMove(ACTION_LANDING);
			}
		}
		if (isOverdriveAction(action) || isRomanCancelAction(action)) {
			auto currentCd = this->_maxOdCooldown;

			if (
				action != ACTION_NEUTRAL_OVERDRIVE &&
				action != ACTION_NEUTRAL_AIR_OVERDRIVE
			)
				currentCd /= 2;
			this->_blockStun = 0;
			this->_odCooldown = this->_barMaxOdCooldown = currentCd;
		} else if (
			action == ACTION_NEUTRAL_JUMP ||
			action == ACTION_FORWARD_JUMP ||
			action == ACTION_BACKWARD_JUMP ||
			action == ACTION_NEUTRAL_AIR_JUMP ||
			action == ACTION_FORWARD_AIR_JUMP ||
			action == ACTION_BACKWARD_AIR_JUMP
		) {
			this->_jumpsUsed++;
			this->_airMovementUsed += !this->_isGrounded();
			this->_hasJumped = true;
		} else if (action == ACTION_NEUTRAL_HIGH_JUMP || action == ACTION_FORWARD_HIGH_JUMP || action == ACTION_BACKWARD_HIGH_JUMP) {
			this->_jumpsUsed += 2;
			this->_hasJumped = true;
		} else if (action >= ACTION_AIR_DASH_1 && action <= ACTION_AIR_DASH_9) {
			this->_airDashesUsed++;
			this->_airMovementUsed++;
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
		} else if (action >= ACTION_5N) {
			this->_hasJumped = true;
			// We add some anti buffer to RC and installs for the shared keys
			if ((action >= ACTION_5N && action < ACTION_5M) || (action >= ACTION_5A && action < ACTION_214D))
				this->_specialInputs._an = -COMBINATION_LENIENCY;
			else if ((action >= ACTION_5M && action < ACTION_5S) || (action >= ACTION_5A && action < ACTION_214D))
				this->_specialInputs._am = -COMBINATION_LENIENCY;
			else if ((action >= ACTION_5S && action < ACTION_5V) || (action >= ACTION_5A && action < ACTION_214D))
				this->_specialInputs._as = -COMBINATION_LENIENCY;
			else if ((action >= ACTION_5V && action < ACTION_5A) || (action >= ACTION_5A && action < ACTION_214D))
				this->_specialInputs._av = -COMBINATION_LENIENCY;
		}
		if (
			!isHitAction(action) &&
			action != ACTION_UP_AIR_TECH &&
			action != ACTION_DOWN_AIR_TECH &&
			action != ACTION_FORWARD_AIR_TECH &&
			action != ACTION_BACKWARD_AIR_TECH &&
			action != ACTION_BEING_KNOCKED_DOWN &&
			action != ACTION_KNOCKED_DOWN
		) {
			this->_comboCtr = 0;
			this->_prorate = 1;
			this->_totalDamage = 0;
			this->_limit.fill(0);
			this->_opponent->_usedMoves.clear();
			this->_counter = false;
			this->_hardKD = false;
			if (action != ACTION_FORWARD_TECH && action != ACTION_NEUTRAL_TECH && action != ACTION_BACKWARD_TECH)
				this->_limitEffects &= ~(MATTER_LIMIT_EFFECT | NEUTRAL_LIMIT_EFFECT | VOID_LIMIT_EFFECT | SPIRIT_LIMIT_EFFECT);
		}
		Object::_forceStartMove(action);
	}

	void Character::setOpponent(Character *opponent)
	{
		this->_opponent = opponent;
	}

	bool Character::_canCancel(unsigned int action)
	{
		auto currentData = this->_oldData;

		if (!currentData)
			currentData = this->getCurrentFrameData();
		if (!currentData->oFlag.cancelable)
			return false;
		if (!this->_hasHit && !currentData->dFlag.karaCancel)
			return false;
		if (currentData->oFlag.backDashCancelable && action == ACTION_BACKWARD_DASH)
			return true;
		if (currentData->oFlag.dashCancelable && action >= ACTION_AIR_DASH_1 && action <= ACTION_AIR_DASH_9 && this->_airDashesUsed < this->_maxAirDashes && this->_airMovementUsed < this->_maxAirMovement)
			return true;
		if (currentData->oFlag.dashCancelable && action == ACTION_FORWARD_DASH)
			return true;
		if (currentData->oFlag.jumpCancelable && action >= ACTION_NEUTRAL_AIR_JUMP && action <= ACTION_BACKWARD_AIR_JUMP && this->_jumpsUsed < this->_maxJumps && this->_airMovementUsed < this->_maxAirMovement)
			return true;
		if (currentData->oFlag.jumpCancelable && action >= ACTION_NEUTRAL_JUMP && action <= ACTION_BACKWARD_HIGH_JUMP)
			return true;
		if (action < 100)
			return false;
		if (action == this->_action && currentData->oFlag.jab)
			return true;
		if (this->getAttackTier(action) > this->getAttackTier(this->_action))
			return true;
		if (currentData->oFlag.hitSwitch && this->_action != action && this->getAttackTier(action) == this->getAttackTier(this->_action))
			return true;
		return false;
	}

	int Character::getAttackTier(unsigned int action) const
	{
		const FrameData *data;

		if (action < 100)
			return -1;

		try {
			data = &this->_moves.at(action).at(0).at(0);
		} catch (...) {
			return -1;
		}
		if (data->priority)
			return *data->priority;

		if (data->oFlag.ultimate)
			return 800;
		if (data->oFlag.super)
			return 700;
		if (action == ACTION_AIR_REVERSAL || action == ACTION_GROUND_HIGH_REVERSAL || action == ACTION_GROUND_LOW_REVERSAL)
			return 700;
		switch (action) {
		case ACTION_5N:
		case ACTION_2N:
		case ACTION_j5N:
			return 0;
		case ACTION_5S:
		case ACTION_2S:
		case ACTION_j5S:
		case ACTION_5M:
		case ACTION_2M:
		case ACTION_j5M:
		case ACTION_5V:
		case ACTION_2V:
		case ACTION_j5V:
			return 100;
		case ACTION_6N:
		case ACTION_8N:
		case ACTION_3N:
		case ACTION_j6N:
		case ACTION_j8N:
		case ACTION_j3N:
		case ACTION_j2N:
			return 200;
		case ACTION_6S:
		case ACTION_8S:
		case ACTION_3S:
		case ACTION_j6S:
		case ACTION_j8S:
		case ACTION_j3S:
		case ACTION_j2S:
			if ((this->_normalTreeFlag >> 2 & 3) == 3)
				return 300 + ((this->_normalTreeFlag & 3) == 0) * 200;
			if ((this->_normalTreeFlag & 3) == 3)
				return 300 + ((this->_normalTreeFlag >> 2 & 3) - 1) * 100;
			if ((this->_normalTreeFlag & 3) == 2)
				return 300 + (this->_normalTreeFlag >> 2 & 3) * 100;
			return 300;
		case ACTION_6M:
		case ACTION_8M:
		case ACTION_3M:
		case ACTION_j6M:
		case ACTION_j8M:
		case ACTION_j3M:
		case ACTION_j2M:
			if ((this->_normalTreeFlag >> 2 & 3) == 3)
				return 300 + ((this->_normalTreeFlag & 3) == 1) * 200;
			if ((this->_normalTreeFlag & 3) == 1)
				return 300 + ((this->_normalTreeFlag >> 2 & 3) - 1) * 100;
			if ((this->_normalTreeFlag & 3) == 3)
				return 300 + (this->_normalTreeFlag >> 2 & 3) * 100;
			return 300;
		case ACTION_6V:
		case ACTION_8V:
		case ACTION_3V:
		case ACTION_j6V:
		case ACTION_j8V:
		case ACTION_j3V:
		case ACTION_j2V:
			if ((this->_normalTreeFlag >> 2 & 3) == 3)
				return 300 + ((this->_normalTreeFlag & 3) == 2) * 200;
			if ((this->_normalTreeFlag & 3) == 2)
				return 300 + ((this->_normalTreeFlag >> 2 & 3) - 1) * 100;
			if ((this->_normalTreeFlag & 3) == 1)
				return 300 + (this->_normalTreeFlag >> 2 & 3) * 100;
			return 300;
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
		case ACTION_c28V:
		case ACTION_c46V:
		case ACTION_c64V:
		case ACTION_214V:
		case ACTION_236V:
		case ACTION_421V:
		case ACTION_623V:
		case ACTION_41236V:
		case ACTION_63214V:
		case ACTION_6321469874V:
		case ACTION_j214V:
		case ACTION_j236V:
		case ACTION_j421V:
		case ACTION_j623V:
		case ACTION_j41236V:
		case ACTION_j63214V:
		case ACTION_j6321469874V:
		case ACTION_c28S:
		case ACTION_c46S:
		case ACTION_c64S:
		case ACTION_214S:
		case ACTION_236S:
		case ACTION_421S:
		case ACTION_623S:
		case ACTION_41236S:
		case ACTION_63214S:
		case ACTION_6321469874S:
		case ACTION_j214S:
		case ACTION_j236S:
		case ACTION_j421S:
		case ACTION_j623S:
		case ACTION_j41236S:
		case ACTION_j63214S:
		case ACTION_j6321469874S:
		case ACTION_c28M:
		case ACTION_c46M:
		case ACTION_c64M:
		case ACTION_214M:
		case ACTION_236M:
		case ACTION_421M:
		case ACTION_623M:
		case ACTION_41236M:
		case ACTION_63214M:
		case ACTION_6321469874M:
		case ACTION_j214M:
		case ACTION_j236M:
		case ACTION_j421M:
		case ACTION_j623M:
		case ACTION_j41236M:
		case ACTION_j63214M:
		case ACTION_j6321469874M:
			return 600;
		default:
			return -1;
		}
	}

	void Character::_checkPlatforms(Vector2f oldPos)
	{
		auto updatedY = this->_position.y;

		if (this->_specialInputs._22) {
			if (this->_isOnPlatform())
				this->_position.y -= 0.01;
			return;
		}
		this->_position.y = oldPos.y;

		auto plat = !this->_isOnPlatform();
		auto down = this->_input->isPressed(INPUT_DOWN);
		auto result = plat && down;

		this->_position.y = updatedY;
		if (result) {
			if (this->_isOnPlatform())
				this->_position.y -= 0.01;
			return;
		}
		Object::_checkPlatforms(oldPos);
	}

	void Character::_checkAllAXMacro(bool tickBuffer)
	{
		auto input = this->_input->getInputs();

		if (this->_specialInputs._an)
			this->_specialInputs._an -= std::copysign(tickBuffer, this->_specialInputs._an);
		if (this->_specialInputs._am)
			this->_specialInputs._am -= std::copysign(tickBuffer, this->_specialInputs._am);
		if (this->_specialInputs._as)
			this->_specialInputs._as -= std::copysign(tickBuffer, this->_specialInputs._as);
		if (this->_specialInputs._av)
			this->_specialInputs._av -= std::copysign(tickBuffer, this->_specialInputs._av);

		if (input.a && input.a < COMBINATION_LENIENCY) {
			if (this->_specialInputs._an >= 0 && input.n && input.n < COMBINATION_LENIENCY)
				this->_specialInputs._an = SPECIAL_INPUT_BUFFER_PERSIST;
			if (this->_specialInputs._am >= 0 && input.m && input.m < COMBINATION_LENIENCY)
				this->_specialInputs._am = SPECIAL_INPUT_BUFFER_PERSIST;
			if (this->_specialInputs._as >= 0 && input.s && input.s < COMBINATION_LENIENCY)
				this->_specialInputs._as = SPECIAL_INPUT_BUFFER_PERSIST;
			if (this->_specialInputs._av >= 0 && input.v && input.v < COMBINATION_LENIENCY)
				this->_specialInputs._av = SPECIAL_INPUT_BUFFER_PERSIST;
		}
	}

	void Character::_checkAllc28Input(bool tickBuffer)
	{
		if (this->_specialInputs._c28n)
			this->_specialInputs._c28n -= tickBuffer;
		else
			this->_specialInputs._c28n = this->_checkc28Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c28m)
			this->_specialInputs._c28m -= tickBuffer;
		else
			this->_specialInputs._c28m = this->_checkc28Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c28s)
			this->_specialInputs._c28s -= tickBuffer;
		else
			this->_specialInputs._c28s = this->_checkc28Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c28v)
			this->_specialInputs._c28v -= tickBuffer;
		else
			this->_specialInputs._c28v = this->_checkc28Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c28d)
			this->_specialInputs._c28d -= tickBuffer;
		else
			this->_specialInputs._c28d = this->_checkc28Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c28a)
			this->_specialInputs._c28a -= tickBuffer;
		else
			this->_specialInputs._c28a = this->_checkc28Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAllc46Input(bool tickBuffer)
	{
		if (this->_specialInputs._c46n)
			this->_specialInputs._c46n -= tickBuffer;
		else
			this->_specialInputs._c46n = this->_checkc46Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c46m)
			this->_specialInputs._c46m -= tickBuffer;
		else
			this->_specialInputs._c46m = this->_checkc46Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c46s)
			this->_specialInputs._c46s -= tickBuffer;
		else
			this->_specialInputs._c46s = this->_checkc46Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c46v)
			this->_specialInputs._c46v -= tickBuffer;
		else
			this->_specialInputs._c46v = this->_checkc46Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c46d)
			this->_specialInputs._c46d -= tickBuffer;
		else
			this->_specialInputs._c46d = this->_checkc46Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c46a)
			this->_specialInputs._c46a -= tickBuffer;
		else
			this->_specialInputs._c46a = this->_checkc46Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAllc64Input(bool tickBuffer)
	{
		if (this->_specialInputs._c64n)
			this->_specialInputs._c64n -= tickBuffer;
		else
			this->_specialInputs._c64n = this->_checkc64Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c64m)
			this->_specialInputs._c64m -= tickBuffer;
		else
			this->_specialInputs._c64m = this->_checkc64Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c64s)
			this->_specialInputs._c64s -= tickBuffer;
		else
			this->_specialInputs._c64s = this->_checkc64Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c64v)
			this->_specialInputs._c64v -= tickBuffer;
		else
			this->_specialInputs._c64v = this->_checkc64Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c64d)
			this->_specialInputs._c64d -= tickBuffer;
		else
			this->_specialInputs._c64d = this->_checkc64Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c64a)
			this->_specialInputs._c64a -= tickBuffer;
		else
			this->_specialInputs._c64a = this->_checkc64Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll236Input(bool tickBuffer)
	{
		if (this->_specialInputs._236n)
			this->_specialInputs._236n -= tickBuffer;
		else
			this->_specialInputs._236n = this->_check236Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._236m)
			this->_specialInputs._236m -= tickBuffer;
		else
			this->_specialInputs._236m = this->_check236Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._236s)
			this->_specialInputs._236s -= tickBuffer;
		else
			this->_specialInputs._236s = this->_check236Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._236v)
			this->_specialInputs._236v -= tickBuffer;
		else
			this->_specialInputs._236v = this->_check236Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._236d)
			this->_specialInputs._236d -= tickBuffer;
		else
			this->_specialInputs._236d = this->_check236Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._236a)
			this->_specialInputs._236a -= tickBuffer;
		else
			this->_specialInputs._236a = this->_check236Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll214Input(bool tickBuffer)
	{
		if (this->_specialInputs._214n)
			this->_specialInputs._214n -= tickBuffer;
		else
			this->_specialInputs._214n = this->_check214Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._214m)
			this->_specialInputs._214m -= tickBuffer;
		else
			this->_specialInputs._214m = this->_check214Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._214s)
			this->_specialInputs._214s -= tickBuffer;
		else
			this->_specialInputs._214s = this->_check214Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._214v)
			this->_specialInputs._214v -= tickBuffer;
		else
			this->_specialInputs._214v = this->_check214Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._214d)
			this->_specialInputs._214d -= tickBuffer;
		else
			this->_specialInputs._214d = this->_check214Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._214a)
			this->_specialInputs._214a -= tickBuffer;
		else
			this->_specialInputs._214a = this->_check214Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll623Input(bool tickBuffer)
	{
		if (this->_specialInputs._623n)
			this->_specialInputs._623n -= tickBuffer;
		else
			this->_specialInputs._623n = this->_check623Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._623m)
			this->_specialInputs._623m--;
		else
			this->_specialInputs._623m = this->_check623Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._623s)
			this->_specialInputs._623s -= tickBuffer;
		else
			this->_specialInputs._623s = this->_check623Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._623v)
			this->_specialInputs._623v -= tickBuffer;
		else
			this->_specialInputs._623v = this->_check623Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._623d)
			this->_specialInputs._623d -= tickBuffer;
		else
			this->_specialInputs._623d = this->_check623Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._623a)
			this->_specialInputs._623a -= tickBuffer;
		else
			this->_specialInputs._623a = this->_check623Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll421Input(bool tickBuffer)
	{
		if (this->_specialInputs._421n)
			this->_specialInputs._421n -= tickBuffer;
		else
			this->_specialInputs._421n = this->_check421Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._421m)
			this->_specialInputs._421m -= tickBuffer;
		else
			this->_specialInputs._421m = this->_check421Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._421s)
			this->_specialInputs._421s -= tickBuffer;
		else
			this->_specialInputs._421s = this->_check421Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._421v)
			this->_specialInputs._421v -= tickBuffer;
		else
			this->_specialInputs._421v = this->_check421Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._421d)
			this->_specialInputs._421d -= tickBuffer;
		else
			this->_specialInputs._421d = this->_check421Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._421a)
			this->_specialInputs._421a -= tickBuffer;
		else
			this->_specialInputs._421a = this->_check421Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll624Input(bool tickBuffer)
	{
		if (this->_specialInputs._624n)
			this->_specialInputs._624n -= tickBuffer;
		else
			this->_specialInputs._624n = this->_check624Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624m)
			this->_specialInputs._624m -= tickBuffer;
		else
			this->_specialInputs._624m = this->_check624Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624s)
			this->_specialInputs._624s -= tickBuffer;
		else
			this->_specialInputs._624s = this->_check624Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624v)
			this->_specialInputs._624v -= tickBuffer;
		else
			this->_specialInputs._624v = this->_check624Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624d)
			this->_specialInputs._624d -= tickBuffer;
		else
			this->_specialInputs._624d = this->_check624Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624a)
			this->_specialInputs._624a -= tickBuffer;
		else
			this->_specialInputs._624a = this->_check624Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;

	}

	void Character::_checkAll426Input(bool tickBuffer)
	{
		if (this->_specialInputs._426n)
			this->_specialInputs._426n -= tickBuffer;
		else
			this->_specialInputs._426n = this->_check426Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._426m)
			this->_specialInputs._426m -= tickBuffer;
		else
			this->_specialInputs._426m = this->_check426Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._426s)
			this->_specialInputs._426s -= tickBuffer;
		else
			this->_specialInputs._426s = this->_check426Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._426v)
			this->_specialInputs._426v -= tickBuffer;
		else
			this->_specialInputs._426v = this->_check426Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._426d)
			this->_specialInputs._426d -= tickBuffer;
		else
			this->_specialInputs._426d = this->_check426Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._426a)
			this->_specialInputs._426a -= tickBuffer;
		else
			this->_specialInputs._426a = this->_check426Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll624684Input(bool tickBuffer)
	{
		if (this->_specialInputs._624684n)
			this->_specialInputs._624684n -= tickBuffer;
		else
			this->_specialInputs._624684n = this->_check624684Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624684m)
			this->_specialInputs._624684m -= tickBuffer;
		else
			this->_specialInputs._624684m = this->_check624684Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624684s)
			this->_specialInputs._624684s -= tickBuffer;
		else
			this->_specialInputs._624684s = this->_check624684Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624684v)
			this->_specialInputs._624684v -= tickBuffer;
		else
			this->_specialInputs._624684v = this->_check624684Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624684d)
			this->_specialInputs._624684d -= tickBuffer;
		else
			this->_specialInputs._624684d = this->_check624684Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624684a)
			this->_specialInputs._624684a -= tickBuffer;
		else
			this->_specialInputs._624684a = this->_check624684Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAllHJInput(bool tickBuffer)
	{
		if (this->_specialInputs._27)
			this->_specialInputs._27 -= tickBuffer;
		else
			this->_specialInputs._27 = this->_check27Input() * HJ_BUFFER_PERSIST;

		if (this->_specialInputs._28)
			this->_specialInputs._28 -= tickBuffer;
		else
			this->_specialInputs._28 = (this->_check28Input() || this->_dummyState == DUMMYSTATE_HIGH_JUMP) * HJ_BUFFER_PERSIST;

		if (this->_specialInputs._29)
			this->_specialInputs._29 -= tickBuffer;
		else
			this->_specialInputs._29 = this->_check29Input() * HJ_BUFFER_PERSIST;
	}

	void Character::_checkAllDashInput(bool tickBuffer)
	{
		if (this->_specialInputs._44)
			this->_specialInputs._44 -= tickBuffer;
		else
			this->_specialInputs._44 = this->_check44Input() * DASH_BUFFER_PERSIST;

		if (this->_specialInputs._66)
			this->_specialInputs._66 -= tickBuffer;
		else
			this->_specialInputs._66 = this->_check66Input() * DASH_BUFFER_PERSIST;
	}

	void Character::_checkSpecialInputs(bool tickBuffer)
	{
		if (this->_inputDisabled) {
			memset(this->_specialInputs._value, 0, sizeof(this->_specialInputs._value));
			return;
		}

		this->_clearLastInputs();
		if (
			this->_action == ACTION_AIR_HIT ||
			this->_action == ACTION_BEING_KNOCKED_DOWN ||
			this->_action == ACTION_KNOCKED_DOWN ||
			this->_action == ACTION_NEUTRAL_TECH ||
			this->_action == ACTION_FORWARD_TECH ||
			this->_action == ACTION_BACKWARD_TECH
		)
			this->_specialInputs._22 = false;
		else
			this->_specialInputs._22 = this->_check22Input();

		this->_checkAllDashInput(tickBuffer);
		this->_checkAllHJInput(tickBuffer);

		if (this->_atkDisabled) {
			memset(&this->_specialInputs._value[3], 0, sizeof(this->_specialInputs._value) - 3);
			return;
		}
		this->_checkAllAXMacro(tickBuffer);
		this->_checkAllc28Input(tickBuffer);
		this->_checkAllc46Input(tickBuffer);
		this->_checkAllc64Input(tickBuffer);
		this->_checkAll236Input(tickBuffer);
		this->_checkAll214Input(tickBuffer);
		this->_checkAll623Input(tickBuffer);
		this->_checkAll421Input(tickBuffer);
		this->_checkAll624Input(tickBuffer);
		this->_checkAll426Input(tickBuffer);
		this->_checkAll624684Input(tickBuffer);
	}

	bool Character::_check236Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found2 = false;
		bool found3 = false;
		bool found6 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			if (foundAtk && atkInput(input))
				return false;
			foundAtk |= atkInput(input);
			found6 |= foundAtk && !input.v && input.h > 0;
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

	bool Character::_check214Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found2 = false;
		bool found1 = false;
		bool found4 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			if (foundAtk && atkInput(input))
				return false;
			foundAtk |= atkInput(input);
			found4 |= foundAtk && !input.v && input.h < 0;
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

	bool Character::_check623Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found2 = false;
		bool found3 = false;
		bool found6 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			if (foundAtk && atkInput(input))
				return false;
			foundAtk |= atkInput(input);
			found3 |= foundAtk && input.v < 0 && input.h > 0;
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

	bool Character::_check421Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found2 = false;
		bool found1 = false;
		bool found4 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			if (foundAtk && atkInput(input))
				return false;
			foundAtk |= atkInput(input);
			found1 |= foundAtk && input.v < 0 && input.h < 0;
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

	bool Character::_check624Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found2 = false;
		bool found4 = false;
		bool found6 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			if (foundAtk && atkInput(input))
				return false;
			foundAtk |= atkInput(input);
			found4 |= foundAtk && !input.v && input.h < 0;
			found2 |= found4 && input.v < 0;
			found6 |= found2 && !input.v && input.h > 0;
			if (!input.v && input.h < 0 && found2 && found4)
				break;
			if (found2 && found4 && found6)
				return true;
			total += input.nbFrames;
			if (total > HALF_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check426Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found2 = false;
		bool found4 = false;
		bool found6 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			if (foundAtk && atkInput(input))
				return false;
			foundAtk |= atkInput(input);
			found6 |= foundAtk && !input.v && input.h > 0;
			found2 |= found6 && input.v < 0;
			found4 |= found2 && !input.v && input.h < 0;
			if (!input.v && input.h > 0 && found2 && found6)
				break;
			if (found2 && found4 && found6)
				return true;
			total += input.nbFrames;
			if (total > HALF_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check624684Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found6_1 = false;
		bool found2 = false;
		bool found4_1 = false;
		bool found6_2 = false;
		bool found8 = false;
		bool found4_2 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			if (foundAtk && atkInput(input))
				return false;
			foundAtk |= atkInput(input);
			found4_2 |= foundAtk && !input.v && input.h < 0;
			found8   |= found4_2 && input.v > 0;
			found6_2 |= found8   && !input.v && input.h > 0;
			found4_1 |= found6_2 && !input.v && input.h < 0;
			found2   |= found4_1 && input.v < 0;
			found6_1 |= found2   && !input.v && input.h > 0;
			if (found6_1 && found2 && found4_1 && found6_2 && found8 && found4_2)
				return true;
			total += input.nbFrames;
			if (total > SPIRAL_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check22Input()
	{
		unsigned total = 0;
		bool found2 = false;
		bool foundOther = false;

		for (auto &input : this->_lastInputs) {
			total += input.nbFrames;
			if (total > DASH_BUFFER)
				return false;
			if (found2 && foundOther && input.v < 0)
				return true;
			found2 |= input.v < 0;
			foundOther |= found2 && input.v >= 0;
		}
		return false;
	}

	bool Character::_check44Input()
	{
		unsigned total = 0;
		bool found4 = false;
		bool foundOther = false;

		for (auto &input : this->_lastInputs) {
			total += input.nbFrames;
			if (total > DASH_BUFFER)
				break;
			if (input.h < 0 && input.v != 0)
				return false;
			if (found4 && foundOther && input.h < 0)
				return true;
			found4 |= input.h < 0;
			foundOther |= found4 && input.h >= 0;
		}
		return false;
	}

	bool Character::_check66Input()
	{
		unsigned total = 0;
		bool found6 = false;
		bool foundOther = false;

		for (auto &input : this->_lastInputs) {
			total += input.nbFrames;
			if (total > DASH_BUFFER)
				break;
			if (input.h > 0 && input.v != 0)
				return false;
			if (found6 && foundOther && input.h > 0)
				return true;
			found6 |= input.h > 0;
			foundOther |= found6 && input.h <= 0;
		}
		return false;
	}

	bool Character::_check27Input()
	{
		unsigned total = 0;
		bool found7 = false;

		for (auto &input : this->_lastInputs) {
			if (input.a || input.n || input.m || input.s || input.o)
				return false;
			total += input.nbFrames;
			if (total > HJ_BUFFER)
				break;
			found7 |= input.v > 0 && input.h < 0;
			if (input.v < 0 && found7)
				return true;
		}
		return false;
	}

	bool Character::_check28Input()
	{
		unsigned total = 0;
		bool found8 = false;

		for (auto &input : this->_lastInputs) {
			if (input.a || input.n || input.m || input.s || input.o)
				return false;
			total += input.nbFrames;
			if (total > HJ_BUFFER)
				break;
			found8 |= input.v > 0;
			if (found8 && input.v < 0)
				return true;
		}
		return false;
	}

	bool Character::_check29Input()
	{
		unsigned total = 0;
		bool found9 = false;

		for (auto &input : this->_lastInputs) {
			if (input.a || input.n || input.m || input.s || input.o)
				return false;
			total += input.nbFrames;
			if (total > HJ_BUFFER)
				break;
			found9 |= input.v > 0 && input.h > 0;
			if (found9 && input.v < 0)
				return true;
		}
		return false;
	}

	bool Character::_checkc28Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned timer2 = 0;
		unsigned timer = 0;
		unsigned total = 0;
		bool found8 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			if (foundAtk && atkInput(input))
				return false;
			foundAtk |= atkInput(input);
			found8 |= foundAtk && input.v > 0;
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

	bool Character::_checkc46Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned timer2 = 0;
		unsigned timer = 0;
		unsigned total = 0;
		bool found6 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			if (foundAtk && atkInput(input))
				return false;
			foundAtk |= atkInput(input);
			found6 |= foundAtk && input.h > 0;
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

	bool Character::_checkc64Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned timer2 = 0;
		unsigned timer = 0;
		unsigned total = 0;
		bool found4 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			if (foundAtk && atkInput(input))
				return false;
			foundAtk |= atkInput(input);
			found4 |= foundAtk && input.h < 0;
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

	void Character::_clearLastInputs()
	{
		auto it = this->_lastInputs.begin();
		unsigned total = 0;

		while (it != this->_lastInputs.end() && total < MAX_FRAME_IN_BUFFER) {
			total += it->nbFrames;
			it++;
		}
		this->_lastInputs.erase(it, this->_lastInputs.end());
	}

	bool Character::_executeAirDashes(const InputStruct &input)
	{
		if (this->_specialInputs._44 && this->_startMove(ACTION_AIR_DASH_4))
			return true;
		if (this->_specialInputs._66 && this->_startMove(ACTION_AIR_DASH_6))
			return true;
		if (!this->_input->isPressed(INPUT_DASH) || (!input.verticalAxis && !input.horizontalAxis))
			return false;
		return  (input.verticalAxis > 0 && this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_AIR_DASH_9)) ||
			(input.verticalAxis > 0 && this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_AIR_DASH_7)) ||
			(input.verticalAxis > 0 &&                                          this->_startMove(ACTION_AIR_DASH_8)) ||
			(input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_AIR_DASH_3)) ||
			(input.verticalAxis < 0 && this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_AIR_DASH_1)) ||
			(input.verticalAxis < 0 &&                                          this->_startMove(ACTION_AIR_DASH_2)) ||
			(                          this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_AIR_DASH_6)) ||
			(                          this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_AIR_DASH_4));
	}

	bool Character::_executeAirParry(const InputStruct &input)
	{
		if (!this->_input->isPressed(INPUT_ASCEND) || input.horizontalAxis * this->_dir >= 0)
			return false;
		if (input.n) {
			if (this->_action == ACTION_AIR_NEUTRAL_PARRY || this->_startMove(ACTION_AIR_NEUTRAL_PARRY))
				return true;
		}
		if (input.m) {
			if (
				this->_action == ACTION_AIR_MATTER_PARRY || this->_startMove(ACTION_AIR_MATTER_PARRY) ||
				this->_action == ACTION_AIR_NEUTRAL_PARRY || this->_startMove(ACTION_AIR_NEUTRAL_PARRY)
			)
				return true;
		}
		if (input.s) {
			if (
				this->_action == ACTION_AIR_SPIRIT_PARRY || this->_startMove(ACTION_AIR_SPIRIT_PARRY) ||
				this->_action == ACTION_AIR_NEUTRAL_PARRY || this->_startMove(ACTION_AIR_NEUTRAL_PARRY)
			)
				return true;
		}
		if (input.v) {
			if (
				this->_action == ACTION_AIR_VOID_PARRY || this->_startMove(ACTION_AIR_VOID_PARRY) ||
				this->_action == ACTION_AIR_NEUTRAL_PARRY || this->_startMove(ACTION_AIR_NEUTRAL_PARRY)
			)
				return true;
		}
		return false;
	}

	bool Character::_executeAirJump(const InputStruct &input)
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

	bool Character::_executeGroundDashes(const InputStruct &input)
	{
		if (this->_specialInputs._44 && this->_startMove(ACTION_BACKWARD_DASH))
			return true;
		if (this->_specialInputs._66 && this->_startMove(ACTION_FORWARD_DASH))
			return true;
		if (!this->_input->isPressed(INPUT_DASH) || !input.horizontalAxis)
			return false;
		return this->_startMove(this->_dir * input.horizontalAxis > 0 ? ACTION_FORWARD_DASH : ACTION_BACKWARD_DASH);
	}

	bool Character::_executeGroundParry(const InputStruct &input)
	{
		if (!this->_input->isPressed(INPUT_ASCEND) || input.horizontalAxis * this->_dir >= 0)
			return false;
		if (input.n) {
			auto move = input.verticalAxis < 0 ? ACTION_GROUND_LOW_NEUTRAL_PARRY : ACTION_GROUND_HIGH_NEUTRAL_PARRY;

			if (this->_action == move || this->_startMove(move))
				return true;
		}
		if (input.m) {
			auto move = input.verticalAxis < 0 ? ACTION_GROUND_LOW_MATTER_PARRY : ACTION_GROUND_HIGH_MATTER_PARRY;
			auto move2 = input.verticalAxis < 0 ? ACTION_GROUND_LOW_NEUTRAL_PARRY : ACTION_GROUND_HIGH_NEUTRAL_PARRY;

			if (
				this->_action == move  || this->_startMove(move) ||
				this->_action == move2 || this->_startMove(move2)
			)
				return true;
		}
		if (input.s) {
			auto move = input.verticalAxis < 0 ? ACTION_GROUND_LOW_SPIRIT_PARRY : ACTION_GROUND_HIGH_SPIRIT_PARRY;
			auto move2 = input.verticalAxis < 0 ? ACTION_GROUND_LOW_NEUTRAL_PARRY : ACTION_GROUND_HIGH_NEUTRAL_PARRY;

			if (
				this->_action == move  || this->_startMove(move) ||
				this->_action == move2 || this->_startMove(move2)
			)
				return true;
		}
		if (input.v) {
			auto move = input.verticalAxis < 0 ? ACTION_GROUND_LOW_VOID_PARRY : ACTION_GROUND_HIGH_VOID_PARRY;
			auto move2 = input.verticalAxis < 0 ? ACTION_GROUND_LOW_NEUTRAL_PARRY : ACTION_GROUND_HIGH_NEUTRAL_PARRY;

			if (
				this->_action == move  || this->_startMove(move) ||
				this->_action == move2 || this->_startMove(move2)
			)
				return true;
		}
		return false;
	}

	bool Character::_executeGroundJump(const InputStruct &input)
	{
		if (this->_specialInputs._29 && this->_startMove(ACTION_FORWARD_HIGH_JUMP))
			return true;
		if (this->_specialInputs._27 && this->_startMove(ACTION_BACKWARD_HIGH_JUMP))
			return true;
		if (this->_specialInputs._28 && this->_startMove(ACTION_NEUTRAL_HIGH_JUMP))
			return true;
		if (input.verticalAxis <= 0)
			return false;
		if (this->_input->isPressed(INPUT_DASH)) {
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

	bool Character::_executeCrouch(const InputStruct &input)
	{
		if (input.verticalAxis >= 0)
			return false;
		this->_startMove(ACTION_CROUCHING);
		return this->_action == ACTION_CROUCHING || this->_action == ACTION_CROUCH;
	}

	bool Character::_executeWalking(const InputStruct &input)
	{
		if (!input.horizontalAxis)
			return false;
		this->_startMove(std::copysign(1, input.horizontalAxis) == std::copysign(1, this->_dir) ? ACTION_WALK_FORWARD : ACTION_WALK_BACKWARD);
		return true;
	}

	bool Character::_executeAirTech(const InputStruct &input)
	{
		switch (this->_dummyAirTech) {
		case AIRTECH_NONE:
			break;
		case AIRTECH_FORWARD:
			if (this->_startMove(ACTION_FORWARD_AIR_TECH))
				return true;
			break;
		case AIRTECH_BACKWARD:
			if (this->_startMove(ACTION_BACKWARD_AIR_TECH))
				return true;
			break;
		case AIRTECH_UP:
			if (this->_startMove(ACTION_UP_AIR_TECH))
				return true;
			break;
		case AIRTECH_DOWN:
			if (this->_startMove(ACTION_DOWN_AIR_TECH))
				return true;
			break;
		case AIRTECH_RANDOM:
			switch (game->random() % 4) {
			case 0:
				if (this->_startMove(ACTION_FORWARD_AIR_TECH))
					return true;
				break;
			case 1:
				if (this->_startMove(ACTION_BACKWARD_AIR_TECH))
					return true;
				break;
			case 2:
				if (this->_startMove(ACTION_UP_AIR_TECH))
					return true;
				break;
			case 3:
				if (this->_startMove(ACTION_DOWN_AIR_TECH))
					return true;
				break;
			}
			break;
		}
		if (!input.d & !input.n & !input.v & !input.m & !input.a & !input.s)
			return false;
		return  (input.verticalAxis > 0 &&                this->_startMove(ACTION_UP_AIR_TECH)) ||
			(input.verticalAxis < 0 &&                this->_startMove(ACTION_DOWN_AIR_TECH)) ||
			(this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_FORWARD_AIR_TECH)) ||
			(this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_BACKWARD_AIR_TECH));
	}

	bool Character::hits(const Object &other) const
	{
		auto otherChr = dynamic_cast<const Character *>(&other);

		if (otherChr) {
			for (auto limit : otherChr->_limit)
				if (limit >= 100)
					return false;

			auto odata = otherChr->getCurrentFrameData();
			auto mdata = this->getCurrentFrameData();

			if (isRomanCancelAction(otherChr->_action) && (this->_action == ACTION_NEUTRAL_OVERDRIVE || this->_action == ACTION_NEUTRAL_AIR_OVERDRIVE))
				return false;
			if (isRomanCancelAction(this->_action) && (otherChr->_action == ACTION_NEUTRAL_OVERDRIVE || otherChr->_action == ACTION_NEUTRAL_AIR_OVERDRIVE))
				return Object::hits(other);

			if (this->_action == ACTION_NEUTRAL_OVERDRIVE || this->_action == ACTION_NEUTRAL_AIR_OVERDRIVE)
				return Object::hits(other);
			if (otherChr->_action == ACTION_NEUTRAL_OVERDRIVE || otherChr->_action == ACTION_NEUTRAL_AIR_OVERDRIVE)
				return false;

			if (
				(odata->oFlag.voidElement || odata->oFlag.spiritElement) &&
				(this->_action == ACTION_VOID_OVERDRIVE || this->_action == ACTION_VOID_AIR_OVERDRIVE)
			)
				return false;
			if (
				(odata->oFlag.spiritElement || odata->oFlag.matterElement) &&
				(this->_action == ACTION_SPIRIT_OVERDRIVE || this->_action == ACTION_SPIRIT_AIR_OVERDRIVE)
			)
				return false;
			if (
				(odata->oFlag.matterElement || odata->oFlag.voidElement) &&
				(this->_action == ACTION_MATTER_OVERDRIVE || this->_action == ACTION_MATTER_AIR_OVERDRIVE)
			)
				return false;

			if (
				(mdata->oFlag.voidElement || mdata->oFlag.spiritElement) &&
				(otherChr->_action == ACTION_VOID_OVERDRIVE  || otherChr->_action == ACTION_VOID_AIR_OVERDRIVE)
			)
				return Object::hits(other);
			if (
				(mdata->oFlag.spiritElement || mdata->oFlag.matterElement) &&
				(otherChr->_action == ACTION_SPIRIT_OVERDRIVE || otherChr->_action == ACTION_SPIRIT_AIR_OVERDRIVE)
			)
				return Object::hits(other);
			if (
				(mdata->oFlag.matterElement || mdata->oFlag.voidElement) &&
				(otherChr->_action == ACTION_MATTER_OVERDRIVE || otherChr->_action == ACTION_MATTER_AIR_OVERDRIVE)
			)
				return Object::hits(other);

			if (isOverdriveAction(otherChr->_action))
				return false;
		}
		if (isRomanCancelAction(this->_action))
			return false;
		return Object::hits(other);
	}

	void Character::postUpdate()
	{
		if (this->_blockStun == 0 && (this->_action == ACTION_GROUND_HIGH_HIT || this->_action == ACTION_GROUND_LOW_HIT) && this->_opponent->_action < ACTION_5N)
			this->_blockStun = 60;
		if (this->_position.x < STAGE_X_MIN)
			this->_position.x = STAGE_X_MIN;
		else if (this->_position.x > STAGE_X_MAX)
			this->_position.x = STAGE_X_MAX;
		if (this->_position.y < STAGE_Y_MIN)
			this->_position.y = STAGE_Y_MIN;
		else if (this->_position.y > STAGE_Y_MAX)
			this->_position.y = STAGE_Y_MAX;

		auto data = this->getCurrentFrameData();
		char buffer[8194];

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
			"AirMovUsed: %i/%i\n"
			"AirDashUsed: %i/%i\n"
			"Jumped: %s\n"
			"Restand: %s\n"
			"Action: %i\n"
			"ActionBlock: %i/%zu\n"
			"Animation: %i/%zu\n"
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
			"BaseGravityY: %f\n"
			"Overdrive CD: %u/%u\n"
			"NormalFlag: %x\n"
			"JumpCancel: %s\n"
			"Time since idle: %i\n"
			"Stalling factor: %f\n",
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
			this->_airMovementUsed,
			this->_maxAirMovement,
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
			this->_baseGravity.y,
			this->_odCooldown,
			this->_maxOdCooldown,
			this->_normalTreeFlag,
			this->_jumpCanceled ? "true" : "false",
			this->_timeSinceIdle,
			this->_stallingFactor
		);
		this->_text.setString(buffer);
		this->_text.setPosition({static_cast<float>(this->_team * 850) + 50, 50});

		sprintf(
			buffer,
			"mana: %.2f/%u\n"
			"specialMarker: %u\n"
			"blockStun: %u\n"
			"hitStun: %u\n"
			"prorate: %3.f\n"
			"neutralLimit: %u\n"
			"voidLimit: %u\n"
			"spiritLimit: %u\n"
			"matterLimit: %u\n"
			"pushBack: %i\n"
			"pushBlock: %i\n"
			"subObjectSpawn: %u\n"
			"manaGain: %u\n"
			"manaCost: %u\n"
			"hitPlayerHitStop: %u\n"
			"hitOpponentHitStop: %u\n"
			"blockPlayerHitStop: %u\n"
			"blockOpponentHitStop: %u\n"
			"damage: %u\n",
			this->_mana,
			this->_manaMax,
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
			data->hitPlayerHitStop,
			data->hitOpponentHitStop,
			data->blockPlayerHitStop,
			data->blockOpponentHitStop,
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
		this->_text2.setPosition({static_cast<float>(this->_team * 500 + 200) , 50});

		for (auto &obj : this->_subobjects)
			if (obj.second && obj.second->isDead()) {
				obj.first = 0;
				obj.second.reset();
			}
	}

	unsigned char Character::_checkHitPos(const Object *other) const
	{
		if (!other)
			return 0;

		auto *oData = this->getCurrentFrameData();
		auto *mData = other->getCurrentFrameData();
		auto mCenter = other->_position;
		auto oCenter = this->_position;
		auto mScale = Vector2f{
			static_cast<float>(mData->size.x) / mData->textureBounds.size.x,
			static_cast<float>(mData->size.y) / mData->textureBounds.size.y
		};
		auto oScale = Vector2f{
			static_cast<float>(oData->size.x) / oData->textureBounds.size.x,
			static_cast<float>(oData->size.y) / oData->textureBounds.size.y
		};
		bool found = false;
		SpiralOfFate::Rectangle hurtbox;
		SpiralOfFate::Rectangle hitbox;

		mCenter.y *= -1;
		mCenter += Vector2f{
			mData->size.x / -2.f - mData->offset.x * !other->_direction * 2.f,
			-static_cast<float>(mData->size.y) + mData->offset.y
		};
		mCenter += Vector2f{
			mData->textureBounds.size.x * mScale.x / 2,
			mData->textureBounds.size.y * mScale.y / 2
		};
		oCenter.y *= -1;
		oCenter += Vector2f{
			oData->size.x / -2.f - oData->offset.x * !this->_direction * 2.f,
			-static_cast<float>(oData->size.y) + oData->offset.y
		};
		oCenter += Vector2f{
			oData->textureBounds.size.x * oScale.x / 2,
			oData->textureBounds.size.y * oScale.y / 2
		};

		for (auto &hurtBox : oData->hurtBoxes) {
			auto _hurtBox = this->_applyModifiers(hurtBox);
			SpiralOfFate::Rectangle __hurtBox;

			__hurtBox.pt1 = _hurtBox.pos.rotation(this->_rotation, oCenter)                                                      + Vector2f{this->_position.x, -this->_position.y};
			__hurtBox.pt2 = (_hurtBox.pos + Vector2f{0, static_cast<float>(_hurtBox.size.y)}).rotation(this->_rotation, oCenter) + Vector2f{this->_position.x, -this->_position.y};
			__hurtBox.pt3 = (_hurtBox.pos + _hurtBox.size).rotation(this->_rotation, oCenter)                                    + Vector2f{this->_position.x, -this->_position.y};
			__hurtBox.pt4 = (_hurtBox.pos + Vector2f{static_cast<float>(_hurtBox.size.x), 0}).rotation(this->_rotation, oCenter) + Vector2f{this->_position.x, -this->_position.y};
			hurtbox = __hurtBox;
			for (auto &hitBox : mData->hitBoxes) {
				auto _hitBox = other->_applyModifiers(hitBox);
				SpiralOfFate::Rectangle __hitBox;

				__hitBox.pt1 = _hitBox.pos.rotation(other->_rotation, mCenter)                                                     + Vector2f{other->_position.x, -other->_position.y};
				__hitBox.pt2 = (_hitBox.pos + Vector2f{0, static_cast<float>(_hitBox.size.y)}).rotation(other->_rotation, mCenter) + Vector2f{other->_position.x, -other->_position.y};
				__hitBox.pt3 = (_hitBox.pos + _hitBox.size).rotation(other->_rotation, mCenter)                                    + Vector2f{other->_position.x, -other->_position.y};
				__hitBox.pt4 = (_hitBox.pos + Vector2f{static_cast<float>(_hitBox.size.x), 0}).rotation(other->_rotation, mCenter) + Vector2f{other->_position.x, -other->_position.y};
				if (__hurtBox.intersect(__hitBox) || __hurtBox.isIn(__hitBox) || __hitBox.isIn(__hurtBox)) {
					hitbox = __hitBox;
					found = true;
					break;
				}
			}
			if (found)
				break;
		}
		my_assert(found);

		auto height = 0;
		auto pts = hurtbox.getIntersectionPoints(hitbox);
		auto center = this->_position.y + oData->offset.y + oData->size.y / 2;

		for (auto &arr : pts)
			for (auto &pt : arr)
				height += (pt.y > center) - (pt.y < center);
		if (height == 0)
			return 0;
		if (pts.size() == 1)
			return 0; // TODO: Handle this case
		return height > 0 ? 1 : 2;
	}

	void Character::_blockMove(Object *other, const FrameData &data)
	{
		auto myData = this->getCurrentFrameData();
		bool blkAction = isBlockingAction(this->_action);
		bool low = (
			this->_input->isPressed(INPUT_DOWN) ||
			(this->_forceBlock & 7) == LOW_BLOCK ||
			((this->_forceBlock & 7) == RANDOM_HEIGHT_BLOCK && game->random() % 2)
		);
		unsigned char height = data.oFlag.lowHit | (data.oFlag.highHit << 1);
		auto chr = dynamic_cast<Character *>(other);

		if (chr)
			chr->_forceCH = false;
		this->_forceCH = false;
		if (chr) {
			chr->_mana += data.manaGain / 4;
			if (this->_spiritEffectTimer)
				chr->_mana += data.manaGain / 4;
			if (chr->_mana > chr->_manaMax)
				chr->_mana = chr->_manaMax;
		}
		if (!isParryAction(this->_action)) {
			if (data.oFlag.spiritElement || data.oFlag.matterElement || data.oFlag.voidElement) {
				int tier = 0;
				auto neutral = data.oFlag.spiritElement == data.oFlag.matterElement && data.oFlag.matterElement == data.oFlag.voidElement;

				if (data.priority)
					tier = *data.priority;
				else if (chr)
					tier = chr->getAttackTier(chr->_actionCache);
				if (tier < 0)
					tier = 100;
				else
					tier += 200;
				this->_neutralEffectTimer = neutral * tier / 2;
				this->_spiritEffectTimer = (!neutral && data.oFlag.spiritElement) * tier / 4;
				this->_matterEffectTimer = (!neutral && data.oFlag.matterElement) * tier / 4;
				this->_voidEffectTimer = (!neutral && data.oFlag.voidElement) * tier / 4;
			}
		}
		this->_hitStop = std::max<unsigned char>(this->_hitStop, data.blockPlayerHitStop);
		other->_hitStop = std::max<unsigned char>(other->_hitStop, data.blockOpponentHitStop);
		if (data.oFlag.autoHitPos)
			height |= this->_checkHitPos(other);
		if ((this->_forceBlock & 7) == ALL_RIGHT_BLOCK)
			low = height & 1;
		if ((this->_forceBlock & 7) == ALL_WRONG_BLOCK)
			low = height & 2;

		auto dir = std::copysign(1, this->_opponent->_position.x - this->_position.x);
		bool wrongBlock =
			((height & 1) && !myData->dFlag.lowBlock && (!low || (!blkAction && !myData->dFlag.canBlock))) ||
			((height & 2) && !myData->dFlag.highBlock && (low || (!blkAction && !myData->dFlag.canBlock)));

		if ((
			(
				(this->_forceBlock & 7) ||
				this->_input->isPressed(dir == 1 ? INPUT_LEFT : INPUT_RIGHT)
			) && myData->dFlag.canBlock) || blkAction
		) {
			if (wrongBlock) {
				if (this->_guardCooldown)
					return this->_getHitByMove(other, data);
				if (this->_isGrounded())
					this->_forceStartMove(low ? ACTION_GROUND_LOW_NEUTRAL_WRONG_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK);
				else
					this->_forceStartMove(ACTION_AIR_NEUTRAL_WRONG_BLOCK);
				this->_blockStun = std::max<unsigned>(this->_blockStun, data.blockStun * 5 / 3);
				game->soundMgr.play(BASICSOUND_WRONG_BLOCK);
			} else {
				if (this->_isGrounded())
					this->_forceStartMove(low ? ACTION_GROUND_LOW_NEUTRAL_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_BLOCK);
				else
					this->_forceStartMove(ACTION_AIR_NEUTRAL_BLOCK);
				this->_blockStun = std::max<unsigned>(this->_blockStun, data.blockStun);
				game->soundMgr.play(BASICSOUND_BLOCK);
			}
			this->_reduceGuard(data.guardDmg, REGEN_CD_BLOCK, true);
		} else if (wrongBlock)
			return this->_getHitByMove(other, data);
		else if (isParryAction(this->_action) && this->_animation == 0) {
			this->_speed.x += data.pushBlock * -this->_dir;
			this->_parryEffect(other);
			return;
		} else if (data.oFlag.matterElement && data.oFlag.voidElement && data.oFlag.spiritElement) {//TRUE NEUTRAL
			if (myData->dFlag.neutralBlock)
				game->soundMgr.play(BASICSOUND_BLOCK);
			else
				return this->_getHitByMove(other, data);
		} else if (data.oFlag.matterElement) {
			if (myData->dFlag.matterBlock)
				game->soundMgr.play(BASICSOUND_BLOCK);
			else if (myData->dFlag.voidBlock || myData->dFlag.neutralBlock || myData->dFlag.spiritBlock)
				return this->_getHitByMove(other, data);
		} else if (data.oFlag.voidElement) {
			if (myData->dFlag.voidBlock)
				game->soundMgr.play(BASICSOUND_BLOCK);
			else if (myData->dFlag.spiritBlock || myData->dFlag.neutralBlock || myData->dFlag.matterBlock)
				return this->_getHitByMove(other, data);
		} else if (data.oFlag.spiritElement) {
			if (myData->dFlag.spiritBlock)
				game->soundMgr.play(BASICSOUND_BLOCK);
			else if (myData->dFlag.matterBlock || myData->dFlag.neutralBlock || myData->dFlag.voidBlock)
				return this->_getHitByMove(other, data);
		} else
			game->soundMgr.play(BASICSOUND_BLOCK);
		if (isParryAction(this->_action)) {
			if (this->_isGrounded())
				this->_forceStartMove(low ? ACTION_GROUND_LOW_NEUTRAL_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_BLOCK);
			else
				this->_forceStartMove(ACTION_AIR_NEUTRAL_BLOCK);
			this->_blockStun = data.blockStun;
		}
		if (data.oFlag.opposingPush)
			this->_speed.x += data.pushBlock * std::copysign(1, this->_position.x - other->_position.x);
		else
			this->_speed.x += data.pushBlock * -this->_dir;
		if (data.chipDamage < this->_hp)
			this->_hp -= data.chipDamage;
		else
			this->_hp = 0;
		this->_stallingFactor = maxi(MINIMUM_STALLING_STACKING, this->_stallingFactor - STALLING_BLOCKING_REMOVE);
	}

	bool Character::_isOnPlatform() const
	{
		if (this->_specialInputs._22)
			return false;
		return Object::_isOnPlatform();
	}

	void Character::_getHitByMove(Object *obj, const FrameData &data)
	{
		if (data.oFlag.ultimate)
			this->_prorate = maxi(0.25, this->_prorate);

		auto myData = this->getCurrentFrameData();
		auto chr = dynamic_cast<Character *>(obj);

		if (this->_isGrounded() && myData->dFlag.crouch && this->_action != ACTION_GROUND_LOW_HIT)
			this->_prorate = 1.15f;
		this->_prorate = maxi(data.minProrate / 100, this->_prorate);

		auto counter = this->_counterHit == 1;
		auto forcedCounter = false;
		float damage = data.damage * this->_prorate;
		auto stun = data.hitStun;

		if (data.snap)
			this->_position = obj->_position + Vector2f{data.snap->x * obj->_dir, data.snap->y};
		this->_forceCH = false;
		if (chr) {
			forcedCounter = chr->_forceCH;
			counter |= forcedCounter;
			chr->_forceCH = false;
			chr->_mana += data.manaGain;
			if (this->_spiritEffectTimer)
				chr->_mana += data.manaGain;
			if (chr->_mana > chr->_manaMax)
				chr->_mana = chr->_manaMax;
		}
		my_assert(!data.oFlag.ultimate || chr);
		if (data.oFlag.spiritElement || data.oFlag.matterElement || data.oFlag.voidElement) {
			int tier = 0;
			auto neutral = data.oFlag.spiritElement == data.oFlag.matterElement && data.oFlag.matterElement == data.oFlag.voidElement;

			if (data.priority)
				tier = *data.priority;
			else if (chr)
				tier = chr->getAttackTier(chr->_actionCache);
			if (tier <= 0)
				tier = 100;
			else
				tier += 200;
			this->_neutralEffectTimer = neutral * tier;
			this->_spiritEffectTimer = (!neutral && data.oFlag.spiritElement) * tier / 2;
			this->_matterEffectTimer = (!neutral && data.oFlag.matterElement) * tier / 2;
			this->_voidEffectTimer = (!neutral && data.oFlag.voidElement) * tier / 2;
		}
		counter &= this->_action != ACTION_AIR_HIT;
		counter &= this->_action != ACTION_WALL_SLAM;
		counter &= this->_action != ACTION_GROUND_SLAM;
		counter &= this->_action != ACTION_GROUND_LOW_HIT;
		counter &= this->_action != ACTION_GROUND_HIGH_HIT;

		bool noArmor = data.oFlag.grab || (
			(!myData->dFlag.superarmor || this->_armorUsed) &&
			(!myData->dFlag.neutralArmor || data.oFlag.spiritElement != data.oFlag.matterElement || data.oFlag.spiritElement != data.oFlag.voidElement) &&
			(!myData->dFlag.matterArmor || data.oFlag.matterElement) &&
			(!myData->dFlag.spiritArmor || data.oFlag.spiritElement) &&
			(!myData->dFlag.voidArmor || data.oFlag.voidElement)
		);

		this->_hasBeenHitDuringFrame = true;
		if (!this->_gotHitStopReset) {
			this->_gotHitStopReset = true;
			if (chr)
				chr->_gotHitStopReset = true;
			obj->_hitStop = 0;
			this->_hitStop = 0;
		}
		if (
			(myData->dFlag.counterHit || counter) &&
			(data.oFlag.canCounterHit || forcedCounter) &&
			this->_counterHit != 2 &&
			(forcedCounter || noArmor)
		) {
			game->soundMgr.play(BASICSOUND_COUNTER_HIT);
			this->_willGroundSlam = data.oFlag.groundSlamCH;
			this->_willWallSplat = data.oFlag.wallSplatCH;
			this->_speed.x = -data.counterHitSpeed.x * this->_dir;
			this->_speed.y =  data.counterHitSpeed.y;
			if (this->_isGrounded() && data.counterHitSpeed.y <= 0)
				this->_forceStartMove(myData->dFlag.crouch ? ACTION_GROUND_LOW_HIT : ACTION_GROUND_HIGH_HIT);
			else {
				this->_forceStartMove(ACTION_AIR_HIT);
				this->_restand = data.oFlag.restand;
				if (this->_restand && this->_moves.at(ACTION_AIR_HIT).size() > 3 && this->_limit[0] < 100 && this->_limit[1] < 100 && this->_limit[2] < 100 && this->_limit[3] < 100)
					this->_actionBlock = 3;
				else if (this->_speed.y < 0)
					this->_actionBlock = 1;
				stun = data.untech;
			}
			this->_counter = true;
			damage *= forcedCounter ? 2 : 1.5;
			stun *= forcedCounter ? 2 : 1.5;
			obj->_hitStop = std::max<unsigned char>(obj->_hitStop, data.hitPlayerHitStop * 1.5);
			this->_hitStop = std::max<unsigned char>(this->_hitStop, data.hitOpponentHitStop * (forcedCounter ? 2 : 1.5));
			game->logger.debug("Counter hit !: " + std::to_string(this->_blockStun) + " hitstun frames");
		} else {
			game->soundMgr.play(data.hitSoundHandle);
			if (noArmor) {
				this->_willGroundSlam = data.oFlag.groundSlam;
				this->_willWallSplat = data.oFlag.wallSplat;
				this->_speed.x = -data.hitSpeed.x * this->_dir;
				this->_speed.y =  data.hitSpeed.y;
				if (this->_isGrounded() && data.hitSpeed.y <= 0)
					this->_forceStartMove(myData->dFlag.crouch ? ACTION_GROUND_LOW_HIT : ACTION_GROUND_HIGH_HIT);
				else {
					this->_forceStartMove(ACTION_AIR_HIT);
					this->_restand = data.oFlag.restand;
					if (this->_restand && this->_moves.at(ACTION_AIR_HIT).size() > 3 && this->_limit[0] < 100 && this->_limit[1] < 100 && this->_limit[2] < 100 && this->_limit[3] < 100)
						this->_actionBlock = 3;
					else if (this->_speed.y < 0)
						this->_actionBlock = 1;
					stun = data.untech;
				}
			} else
				stun = 0;
			obj->_hitStop = std::max<unsigned char>(obj->_hitStop, data.hitPlayerHitStop);
			this->_hitStop = std::max<unsigned char>(this->_hitStop, data.hitOpponentHitStop);
			game->logger.debug(std::to_string(this->_blockStun) + " hitstun frames");
		}

		unsigned nb = 0;

		//TODO: Handle projectiles properly
		if (chr) {
			auto it = chr->_usedMoves.find(chr->_action);

			if (chr->_action >= ACTION_5N && chr->_action < ACTION_5A && (
				chr->_action % 50 == ACTION_5N % 50 ||
				chr->_action % 50 == ACTION_2N % 50 ||
				chr->_action % 50 == ACTION_j5N % 50
			));
			else if (it != chr->_usedMoves.end())
				nb = it->second;
		}
		this->_blockStun = stun;
		this->_totalDamage += damage;
		if (this->_comboCtr == 0)
			this->_limitEffects = 0;
		this->_comboCtr++;
		this->_prorate *= std::pow(data.prorate / 100, (nb + 1));
		this->_limit[0] += data.neutralLimit * (nb + 1);
		this->_limit[1] += data.voidLimit * (nb + 1);
		this->_limit[2] += data.matterLimit * (nb + 1);
		this->_limit[3] += data.spiritLimit * (nb + 1);
		if (this->_limit[0] >= 100) {
			this->_opponent->Object::_forceStartMove(this->_opponent->_isGrounded() ? ACTION_ROMAN_CANCEL : ACTION_AIR_ROMAN_CANCEL);
			this->_limitEffects |= NEUTRAL_LIMIT_EFFECT;
			this->_opponent->_doubleGravity = true;
		}
		if (this->_limit[1] >= 100)
			this->_limitEffects |= VOID_LIMIT_EFFECT;
		if (this->_limit[2] >= 100)
			this->_limitEffects |= MATTER_LIMIT_EFFECT;
		if (this->_limit[3] >= 100) {
			this->_limitEffects |= SPIRIT_LIMIT_EFFECT;
			this->_limitEffects |= MAX_LIMIT_EFFECT_TIMER;
		}
		this->_hardKD = data.oFlag.hardKnockDown;
		if (this->_hp > damage)
			this->_hp -= damage;
		else
			this->_hp = 0;
		this->_armorUsed = true;
		this->_stallingFactor = maxi(MINIMUM_STALLING_STACKING, this->_stallingFactor - STALLING_BEING_HIT_REMOVE);
	}

	void Character::_processWallSlams()
	{
		if (this->_position.x < STAGE_X_MIN)
			this->_position.x = STAGE_X_MIN;
		else if (this->_position.x > STAGE_X_MAX)
			this->_position.x = STAGE_X_MAX;

		if (this->_position.x == STAGE_X_MIN || this->_position.x == STAGE_X_MAX) {
			if (
				!this->_willWallSplat ||
				(this->_action != ACTION_AIR_HIT && this->_action != ACTION_GROUND_HIGH_HIT && this->_action != ACTION_GROUND_LOW_HIT)
			)
				return;
			this->_speed.x *= -0.1;
			this->_speed.y = 7.5;
			game->soundMgr.play(BASICSOUND_WALL_BOUNCE);
			this->_forceStartMove(ACTION_WALL_SLAM);
			this->_blockStun += WALL_SLAM_HITSTUN_INCREASE;
		}
	}

	void Character::_processGroundSlams()
	{
		if (this->_isGrounded()) {
			if (this->_position.y < STAGE_Y_MIN)
				this->_position.y = STAGE_Y_MIN;
		} else if (this->_position.y > STAGE_Y_MAX)
			this->_position.y = STAGE_Y_MAX;

		if (this->_position.y == STAGE_Y_MAX || this->_isGrounded()) {
			if (
				!this->_willGroundSlam ||
				(this->_action != ACTION_AIR_HIT && this->_action != ACTION_GROUND_HIGH_HIT && this->_action != ACTION_GROUND_LOW_HIT)
			) {
				if (this->_isGrounded())
					this->_speed.y = 0;
				return;
			}
			this->_speed.x *= 0.1;
			this->_speed.y *= -0.8;
			game->soundMgr.play(BASICSOUND_GROUND_SLAM);
			this->_forceStartMove(ACTION_GROUND_SLAM);
			this->_blockStun += GROUND_SLAM_HITSTUN_INCREASE;
		}
	}

	void Character::_calculateCornerPriority()
	{
		auto newPriority = (this->_position.x >= STAGE_X_MAX) - (this->_position.x <= STAGE_X_MIN);

		this->_justGotCorner = newPriority && !this->_cornerPriority;
		if (newPriority && this->_justGotCorner) {
			if (newPriority == this->_opponent->_cornerPriority) {
				if (this->_team == 1 && this->_opponent->_justGotCorner && this->_opponent->_cornerPriority == 1) {
					this->_cornerPriority = newPriority;
					this->_opponent->_justGotCorner = false;
					this->_opponent->_cornerPriority = 0;
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
	}

	void Character::_applyMoveAttributes()
	{
		auto data = this->getCurrentFrameData();
		auto gravity = this->_gravity;

		if (this->_doubleGravity)
			this->_gravity.y *= 2;
		Object::_applyMoveAttributes();
		this->_gravity = gravity;
		if (!this->_hadUltimate && data->oFlag.ultimate) {
			game->soundMgr.play(BASICSOUND_ULTIMATE);
			this->_mana = 0;
		}
		this->_hadUltimate = data->oFlag.ultimate;
		this->_ultimateUsed |= data->oFlag.ultimate;
		if (this->_mana >= data->manaCost) {
			this->_mana -= data->manaCost;
			return;
		}
		this->_mana = 0;
		this->_forceStartMove(data->dFlag.crouch ? ACTION_CROUCH : ACTION_IDLE);
	}

	std::pair<unsigned int, std::shared_ptr<Object>> Character::_spawnSubObject(BattleManager &manager, unsigned int id, bool needRegister)
	{
		my_assert2(this->_projectileData.find(id) != this->_projectileData.end(), "Cannot find subobject " + std::to_string(id));

		auto &pdat = this->_projectileData[id];
		bool dir = this->_getProjectileDirection(pdat);

		try {
			return manager.registerObject<Projectile>(
				needRegister,
				this->_subObjectsData.at(pdat.action),
				this->_team,
				dir,
				this->_calcProjectilePosition(pdat, dir ? 1 : -1),
				this->_team,
				this,
				id,
				pdat.json
			);
		} catch (std::out_of_range &e) {
			throw std::invalid_argument("Cannot find subobject action id " + std::to_string(id));
		}
	}

	const std::shared_ptr<IInput> &Character::getInput() const
	{
		return this->_input;
	}

	std::shared_ptr<IInput> &Character::getInput()
	{
		return this->_input;
	}

	const std::map<unsigned, std::vector<std::vector<FrameData>>> &Character::getFrameData()
	{
		return this->_moves;
	}

	void Character::setAttacksDisabled(bool disabled)
	{
		this->_atkDisabled = disabled;
	}

	void Character::disableInputs(bool disabled)
	{
		this->_inputDisabled = disabled;
	}

	unsigned int Character::getBufferSize() const
	{
		return Object::getBufferSize() +
			sizeof(Data) +
			sizeof(LastInput) * this->_lastInputs.size() +
			sizeof(ReplayData) * this->_replayData.size() +
			sizeof(unsigned) * 2 * this->_usedMoves.size();
	}

	void Character::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());
		size_t i = 0;

		Object::copyToBuffer(data);
		game->logger.verbose("Saving Character (Data size: " + std::to_string(sizeof(Data) + sizeof(LastInput) * this->_lastInputs.size()) + ") @" + std::to_string((uintptr_t)dat));
		dat->_hardKD = this->_hardKD;
		dat->_guardBarTmp = this->_guardBarTmp;
		dat->_limitEffects = this->_limitEffects;
		dat->_airMovementUsed = this->_airMovementUsed;
		dat->_neutralEffectTimer = this->_neutralEffectTimer;
		dat->_matterEffectTimer = this->_matterEffectTimer;
		dat->_spiritEffectTimer = this->_spiritEffectTimer;
		dat->_voidEffectTimer = this->_voidEffectTimer;
		dat->_timeSinceIdle = this->_timeSinceIdle;
		dat->_matterInstallTimer = this->_matterInstallTimer;
		dat->_spiritInstallTimer = this->_spiritInstallTimer;
		dat->_voidInstallTimer = this->_voidInstallTimer;
		dat->_jumpCanceled = this->_jumpCanceled;
		dat->_hadUltimate = this->_hadUltimate;
		dat->_grabInvul = this->_grabInvul;
		dat->_hitStop = this->_hitStop;
		dat->_ultimateUsed = this->_ultimateUsed;
		dat->_normalTreeFlag = this->_normalTreeFlag;
		dat->_nbReplayInputs = this->_replayData.size();
		dat->_inputBuffer = this->_inputBuffer;
		dat->_guardRegenCd = this->_guardRegenCd;
		dat->_odCooldown = this->_odCooldown;
		dat->_counter = this->_counter;
		dat->_blockStun = this->_blockStun;
		dat->_jumpsUsed = this->_jumpsUsed;
		dat->_airDashesUsed = this->_airDashesUsed;
		dat->_comboCtr = this->_comboCtr;
		dat->_totalDamage = this->_totalDamage;
		dat->_prorate = this->_prorate;
		dat->_armorUsed = this->_armorUsed;
		dat->_atkDisabled = this->_atkDisabled;
		dat->_inputDisabled = this->_inputDisabled;
		dat->_hasJumped = this->_hasJumped;
		dat->_restand = this->_restand;
		dat->_justGotCorner = this->_justGotCorner;
		dat->_regen = this->_regen;
		dat->_mana = this->_mana;
		dat->_guardCooldown = this->_guardCooldown;
		dat->_guardBar = this->_guardBar;
		dat->_stallingFactor = this->_stallingFactor;
		dat->_willGroundSlam = this->_willGroundSlam;
		dat->_willWallSplat = this->_willWallSplat;
		dat->_doubleGravity = this->_doubleGravity;
		memcpy(dat->_specialInputs, this->_specialInputs._value, sizeof(dat->_specialInputs));
		dat->_nbUsedMoves = this->_usedMoves.size();
		dat->_nbLastInputs = this->_lastInputs.size();
		for (i = 0; i < this->_limit.size(); i++)
			dat->_limit[i] = this->_limit[i];
		i = 0;
		for (auto it = this->_lastInputs.begin(); it != this->_lastInputs.end(); it++, i++)
			((LastInput *)&dat[1])[i] = *it;
		for (i = 0; i < this->_subobjects.size(); i++) {
			if (this->_subobjects[i].first && this->_subobjects[i].second)
				dat->_subObjects[i] = this->_subobjects[i].first;
			else
				dat->_subObjects[i] = 0;
		}
		memcpy(&((LastInput *)&dat[1])[dat->_nbLastInputs], this->_replayData.data(), this->_replayData.size() * sizeof(ReplayData));

		auto p = (unsigned *)(((ptrdiff_t)&((LastInput *)&dat[1])[dat->_nbLastInputs]) + this->_replayData.size() * sizeof(ReplayData));

		for (auto &t : this->_usedMoves) {
			p[0] = t.first;
			p[1] = t.second;
			p += 2;
		}
	}

	void Character::restoreFromBuffer(void *data)
	{
		Object::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		this->_hardKD = dat->_hardKD;
		this->_airMovementUsed = dat->_airMovementUsed;
		this->_guardBarTmp = dat->_guardBarTmp;
		this->_limitEffects = dat->_limitEffects;
		this->_neutralEffectTimer = dat->_neutralEffectTimer;
		this->_matterEffectTimer = dat->_matterEffectTimer;
		this->_spiritEffectTimer = dat->_spiritEffectTimer;
		this->_voidEffectTimer = dat->_voidEffectTimer;
		this->_timeSinceIdle = dat->_timeSinceIdle;
		this->_matterInstallTimer = dat->_matterInstallTimer;
		this->_spiritInstallTimer = dat->_spiritInstallTimer;
		this->_voidInstallTimer = dat->_voidInstallTimer;
		this->_hitStop = dat->_hitStop;
		this->_jumpCanceled = dat->_jumpCanceled;
		this->_hadUltimate = dat->_hadUltimate;
		this->_grabInvul = dat->_grabInvul;
		this->_ultimateUsed = dat->_ultimateUsed;
		this->_normalTreeFlag = dat->_normalTreeFlag;
		this->_inputBuffer = dat->_inputBuffer;
		this->_guardRegenCd = dat->_guardRegenCd;
		this->_odCooldown = dat->_odCooldown;
		this->_counter = dat->_counter;
		this->_blockStun = dat->_blockStun;
		this->_jumpsUsed = dat->_jumpsUsed;
		this->_airDashesUsed = dat->_airDashesUsed;
		this->_comboCtr = dat->_comboCtr;
		this->_armorUsed = dat->_armorUsed;
		this->_totalDamage = dat->_totalDamage;
		this->_prorate = dat->_prorate;
		this->_stallingFactor = dat->_stallingFactor;
		this->_atkDisabled = dat->_atkDisabled;
		this->_inputDisabled = dat->_inputDisabled;
		this->_hasJumped = dat->_hasJumped;
		this->_restand = dat->_restand;
		this->_justGotCorner = dat->_justGotCorner;
		this->_regen = dat->_regen;
		this->_mana = dat->_mana;
		this->_guardCooldown = dat->_guardCooldown;
		this->_guardBar = dat->_guardBar;
		this->_willGroundSlam = dat->_willGroundSlam;
		this->_willWallSplat = dat->_willWallSplat;
		this->_doubleGravity = dat->_doubleGravity;
		memcpy(this->_specialInputs._value, dat->_specialInputs, sizeof(dat->_specialInputs));
		this->_lastInputs.clear();
		for (size_t i = 0; i < dat->_nbLastInputs; i++)
			this->_lastInputs.push_back(((LastInput *)&dat[1])[i]);
		for (size_t i = 0; i < this->_limit.size(); i++)
			this->_limit[i] = dat->_limit[i];
		for (size_t i = 0; i < this->_subobjects.size(); i++) {
			this->_subobjects[i].first = dat->_subObjects[i];
			this->_subobjects[i].second.reset();
		}
		this->_replayData.clear();
		this->_replayData.reserve(dat->_nbReplayInputs);
		for (size_t i = 0; i < dat->_nbReplayInputs; i++)
			this->_replayData.push_back((
				(ReplayData *)(&(
					(LastInput *)&dat[1]
				)[dat->_nbLastInputs])
			)[i]);

		auto p = (unsigned *)&(
			(ReplayData *)(&(
				(LastInput *)&dat[1]
			)[dat->_nbLastInputs])
		)[dat->_nbReplayInputs];

		this->_usedMoves.clear();
		for (size_t i = 0; i < dat->_nbUsedMoves; i++) {
			this->_usedMoves[p[0]] = p[1];
			p += 2;
		}
		game->logger.verbose("Restored Character @" + std::to_string((uintptr_t)dat));
	}

	void Character::resolveSubObjects(const BattleManager &manager)
	{
		for (auto &subobject : this->_subobjects)
			if (subobject.first)
				subobject.second = manager.getObjectFromId(subobject.first);
	}

	unsigned int Character::getClassId() const
	{
		return 1;
	}

	void Character::_removeSubobjects()
	{
		for (auto &obj : this->_subobjects)
			obj.second.reset();
	}

	void Character::_parryVoidEffect(Object *other, bool isStrongest)
	{
		auto oData = other->getCurrentFrameData();

		other->_hp = maxi(1, static_cast<int>(other->_hp - REFLECT_PERCENT * oData->damage / 100));
		this->_forceCH = isStrongest;
	}

	void Character::_parryMatterEffect(Object *other, bool isStrongest)
	{
		if (other->getTeam() == !this->_team) {
			this->_opponent->_mana -= this->_opponent->_manaMax * (10 + isStrongest * 10) / 100;
			if (this->_opponent->_mana < 0)
				this->_opponent->_manaCrush();
		}
	}

	void Character::_parrySpiritEffect(Object *other, bool isStrongest)
	{
		if (isStrongest) {
			auto pos = other->_position;

			other->_position = this->_position;
			this->_position = pos;
			this->_dir *= -1;
			this->_direction = !this->_direction;
		} else {
			other->_speed.x += this->_dir * 10;
			this->_speed.x -= this->_dir * 10;
		}
	}

	void Character::_parryEffect(Object *other)
	{
		auto data = this->getCurrentFrameData();
		auto oData = other->getCurrentFrameData();
		bool isStrongest = false;
		bool isWeakest = false;
		auto sObj = dynamic_cast<SubObject *>(other);

		my_assert_eq(data->dFlag.neutralBlock + data->dFlag.voidBlock + data->dFlag.spiritBlock + data->dFlag.matterBlock, 1);
		if (oData->oFlag.matterElement == oData->oFlag.voidElement && oData->oFlag.voidElement == oData->oFlag.spiritElement) {
			// Neutral attack
			isStrongest = (data->dFlag.neutralBlock && oData->oFlag.spiritElement) || (!data->dFlag.neutralBlock && !oData->oFlag.spiritElement);
			isWeakest = false;
		} else if (oData->oFlag.spiritElement) {
			// Spirit attack
			isStrongest = data->dFlag.matterBlock;
			isWeakest   = data->dFlag.voidBlock;
		} else if (oData->oFlag.voidElement) {
			// Void attack
			isStrongest = data->dFlag.spiritBlock;
			isWeakest   = data->dFlag.matterBlock;
		} else if (oData->oFlag.matterElement) {
			// Matter attack
			isStrongest = data->dFlag.voidBlock;
			isWeakest   = data->dFlag.spiritBlock;
		}

		if (sObj && sObj->getCurrentFrameData()->dFlag.projectile);
		else if (isWeakest);
		else if (data->dFlag.voidBlock)
			this->_parryVoidEffect(other, isStrongest);
		else if (data->dFlag.spiritBlock)
			this->_parrySpiritEffect(other, isStrongest);
		else if (data->dFlag.matterBlock)
			this->_parryMatterEffect(other, isStrongest);

		if (!isWeakest && (!data->dFlag.neutralBlock || isStrongest)) {
			unsigned loss = (data->dFlag.neutralBlock + 1) * PARRY_COST;

			if (this->_guardCooldown) {
				this->_guardCooldown = 0;
				this->_guardBar = loss;
			} else {
				this->_guardBar += loss;
				if (this->_guardBar > this->_maxGuardBar)
					this->_guardBar = this->_maxGuardBar;
			}
		}
		if (isStrongest) {
			if (sObj && sObj->getCurrentFrameData()->dFlag.projectile) {
				this->_hitStop = 15;
				if (other->_team <= 1)
					other->_team = this->_team;
				other->_speed.x *= -1;
				other->_dir *= -1;
				other->_direction = !other->_direction;
			} else
				this->_hitStop = 20;
			other->_hitStop = 25;
			game->soundMgr.play(BASICSOUND_BEST_PARRY);
		} else if (isWeakest)
			game->soundMgr.play(BASICSOUND_WORST_PARRY);
		else
			game->soundMgr.play(BASICSOUND_BLOCK);

		memset(&this->_specialInputs._value, 0, sizeof(this->_specialInputs._value));
		memset(&this->_inputBuffer, 0, sizeof(this->_inputBuffer));

		if (isStrongest)
			this->_forceStartMove(this->_isGrounded() ? (data->dFlag.crouch ? ACTION_CROUCH : ACTION_IDLE) : ACTION_FALLING);
		else if (data->dFlag.neutralBlock) {
			this->_hitStop = 20;
			other->_hitStop = 20;
			this->_forceStartMove(this->_getReversalAction());
		} else {
			this->_forceStartMove(this->_isGrounded() ? (data->dFlag.crouch ? ACTION_GROUND_LOW_NEUTRAL_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_BLOCK) : ACTION_AIR_NEUTRAL_BLOCK);
			this->_blockStun = oData->blockStun * (3 + isWeakest) / 3;
		}
	}

	unsigned Character::_getReversalAction()
	{
		return this->_isGrounded() ? (
			this->getCurrentFrameData()->dFlag.crouch ?
			ACTION_GROUND_LOW_REVERSAL :
			ACTION_GROUND_HIGH_REVERSAL
		) : ACTION_AIR_REVERSAL;
	}

	InputStruct Character::_getInputs()
	{
		auto result = this->_input->getInputs();

		if (this->_dummyState == DUMMYSTATE_JUMP)
			result.verticalAxis = 1;
		if (this->_dummyState == DUMMYSTATE_CROUCH)
			result.verticalAxis = -1;
		return result;
	}

	bool Character::isHitAction(unsigned int action)
	{
		switch (action) {
		case ACTION_GROUND_HIGH_HIT:
		case ACTION_GROUND_LOW_HIT:
		case ACTION_AIR_HIT:
		case ACTION_WALL_SLAM:
		case ACTION_GROUND_SLAM:
			return true;
		default:
			return false;
		}
	}

	bool Character::isBlockingAction(unsigned int action)
	{
		switch (action) {
		case ACTION_GROUND_HIGH_NEUTRAL_BLOCK:
		case ACTION_GROUND_LOW_NEUTRAL_BLOCK:
		case ACTION_AIR_NEUTRAL_BLOCK:
		case ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK:
		case ACTION_GROUND_LOW_NEUTRAL_WRONG_BLOCK:
		case ACTION_AIR_NEUTRAL_WRONG_BLOCK:
			return true;
		default:
			return false;
		}
	}

	bool Character::isParryAction(unsigned int action)
	{
		switch (action) {
		case ACTION_GROUND_HIGH_NEUTRAL_PARRY:
		case ACTION_GROUND_HIGH_SPIRIT_PARRY:
		case ACTION_GROUND_HIGH_MATTER_PARRY:
		case ACTION_GROUND_HIGH_VOID_PARRY:
		case ACTION_GROUND_LOW_NEUTRAL_PARRY:
		case ACTION_GROUND_LOW_SPIRIT_PARRY:
		case ACTION_GROUND_LOW_MATTER_PARRY:
		case ACTION_GROUND_LOW_VOID_PARRY:
		case ACTION_AIR_NEUTRAL_PARRY:
		case ACTION_AIR_SPIRIT_PARRY:
		case ACTION_AIR_MATTER_PARRY:
		case ACTION_AIR_VOID_PARRY:
			return true;
		default:
			return false;
		}
	}

	bool Character::isOverdriveAction(unsigned int action)
	{
		switch (action) {
		case ACTION_NEUTRAL_OVERDRIVE:
		case ACTION_MATTER_OVERDRIVE:
		case ACTION_SPIRIT_OVERDRIVE:
		case ACTION_VOID_OVERDRIVE:
		case ACTION_NEUTRAL_AIR_OVERDRIVE:
		case ACTION_MATTER_AIR_OVERDRIVE:
		case ACTION_SPIRIT_AIR_OVERDRIVE:
		case ACTION_VOID_AIR_OVERDRIVE:
			return true;
		default:
			return false;
		}
	}

	bool Character::isRomanCancelAction(unsigned int action)
	{
		switch (action) {
		case ACTION_ROMAN_CANCEL:
		case ACTION_AIR_ROMAN_CANCEL:
			return true;
		default:
			return false;
		}
	}

	const std::vector<Character::ReplayData> &Character::getReplayData() const
	{
		return this->_replayData;
	}

	bool Character::_executeNeutralAttack(unsigned int base)
	{
		//5X, 2X, 6X, 3X, 8X
		char tries[] = {0, 4, 1, 3, 2};

		for (auto tr : tries)
			if (this->_hasMove(base + tr))
				return this->_startMove(base + tr);
		return false;
	}

	bool Character::_executeDownAttack(unsigned int base)
	{
		//2X, 5X, 3X, 6X, 8X
		char tries[] = {4, 0, 3, 1, 2};

		for (auto tr : tries)
			if (this->_hasMove(base + tr))
				return this->_startMove(base + tr);
		return false;
	}

	void Character::_applyNewAnimFlags()
	{
		if (!this->_newAnim)
			return;

		auto data = this->getCurrentFrameData();

		Object::_applyNewAnimFlags();
		if (data->oFlag.forceTurnAround) {
			this->_dir *= -1;
			this->_direction = this->_dir == 1;
		} else if (data->oFlag.turnAround && this->_opponent) {
			if (this->_opponent->_position.x - this->_position.x != 0)
				this->_dir = std::copysign(1, this->_opponent->_position.x - this->_position.x);
			this->_direction = this->_dir == 1;
		}
		if (!this->_ultimateUsed && data->oFlag.ultimate) {
			game->soundMgr.play(BASICSOUND_ULTIMATE);
			this->_mana = 0;
		}
		this->_ultimateUsed |= data->oFlag.ultimate;
		if (data->subObjectSpawn > 0) {
			if (data->subObjectSpawn <= 64 && this->_subobjects[data->subObjectSpawn - 1].first)
				return;
			else if (data->subObjectSpawn <= 128 && this->_subobjects[data->subObjectSpawn - 1].first)
				this->_subobjects[data->subObjectSpawn - 1].second->kill();

			auto obj = this->_spawnSubObject(*game->battleMgr, data->subObjectSpawn - 1, true);

			if (data->subObjectSpawn > 128)
				return;
			this->_subobjects[data->subObjectSpawn - 1] = obj;
		}
		if (data->particleGenerator > 0) {
			my_assert(data->particleGenerator <= this->_generators.size());
			game->battleMgr->registerObject<ParticleGenerator>(true, this->_generators[data->particleGenerator - 1], *this);
		}
	}

	void Character::_manaCrush()
	{
		this->_mana = this->_manaMax / 10;
		if (this->_isGrounded()) {
			this->_blockStun = 60;
			this->_forceStartMove(this->getCurrentFrameData()->dFlag.crouch ? ACTION_GROUND_LOW_HIT : ACTION_GROUND_HIGH_HIT);
			this->_speed = {this->_dir * -1, 0};
		} else {
			this->_blockStun = 12000;
			this->_forceStartMove(ACTION_AIR_HIT);
			this->_speed = {this->_dir * -1, 20};
		}
		game->soundMgr.play(BASICSOUND_GUARD_BREAK);
	}

	void Character::onMatchEnd()
	{
		std::vector<unsigned> actions;

		for (int i = 0; i < 4; i++)
			if (this->_hasMove(ACTION_WIN_MATCH1 + i))
				actions.emplace_back(ACTION_WIN_MATCH1 + i);
		if (actions.empty())
			return;
		this->_forceStartMove(actions[random_distrib(game->battleRandom, 0, actions.size())]);
	}


	bool Character::matchEndUpdate()
	{
		if (this->_action < ACTION_WIN_MATCH1)
			return false;
		this->_tickMove();
		return this->_animation + this->_animationCtr;
	}

	void Character::_mutateHitFramedata(FrameData &) const
	{
	}

	void Character::_tickMove()
	{
		if (
			(this->_action == ACTION_GROUND_HIGH_HIT || this->_action == ACTION_GROUND_LOW_HIT || isBlockingAction(this->_action)) &&
			this->_actionBlock == 1 &&
			this->_moves.at(this->_action).size() > 2
		) {
			auto data = this->getCurrentFrameData();

			if (data->specialMarker >= this->_blockStun) {
				this->_actionBlock++;
				this->_animation = 0;
				this->_animationCtr = 0;
				this->_newAnim = true;
				return;
			}
		}
		if (
			(
				this->_action == ACTION_AIR_HIT ||
				this->_action == ACTION_FALLING
			) &&
			this->_actionBlock == 0 &&
			this->_moves.at(this->_action).size() > 2 &&
			this->_speed.y < 0
		) {
			this->_actionBlock++;
			this->_animation = 0;
			this->_animationCtr = 0;
			this->_newAnim = true;
			return;
		}
		Object::_tickMove();
	}

	size_t Character::printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const
	{
		auto length = Object::printDifference(msgStart, data1, data2, startOffset);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		game->logger.info("Character @" + std::to_string(startOffset + length));
		if (dat1->_limitEffects != dat2->_limitEffects)
			game->logger.fatal(std::string(msgStart) + "Character::_limitEffects: " + std::to_string(dat1->_limitEffects) + " vs " + std::to_string(dat2->_limitEffects));
		if (dat1->_hardKD != dat2->_hardKD)
			game->logger.fatal(std::string(msgStart) + "Character::_hardKD: " + std::to_string(dat1->_hardKD) + " vs " + std::to_string(dat2->_hardKD));
		if (dat1->_neutralEffectTimer != dat2->_neutralEffectTimer)
			game->logger.fatal(std::string(msgStart) + "Character::_neutralEffectTimer: " + std::to_string(dat1->_neutralEffectTimer) + " vs " + std::to_string(dat2->_neutralEffectTimer));
		if (dat1->_matterEffectTimer != dat2->_matterEffectTimer)
			game->logger.fatal(std::string(msgStart) + "Character::_matterEffectTimer: " + std::to_string(dat1->_matterEffectTimer) + " vs " + std::to_string(dat2->_matterEffectTimer));
		if (dat1->_spiritEffectTimer != dat2->_spiritEffectTimer)
			game->logger.fatal(std::string(msgStart) + "Character::_spiritEffectTimer: " + std::to_string(dat1->_spiritEffectTimer) + " vs " + std::to_string(dat2->_spiritEffectTimer));
		if (dat1->_voidEffectTimer != dat2->_voidEffectTimer)
			game->logger.fatal(std::string(msgStart) + "Character::_voidEffectTimer: " + std::to_string(dat1->_voidEffectTimer) + " vs " + std::to_string(dat2->_voidEffectTimer));
		if (dat1->_hitStop != dat2->_hitStop)
			game->logger.fatal(std::string(msgStart) + "Character::_hitStop: " + std::to_string(dat1->_hitStop) + " vs " + std::to_string(dat2->_hitStop));
		if (dat1->_jumpCanceled != dat2->_jumpCanceled)
			game->logger.fatal(std::string(msgStart) + "Character::_jumpCanceled: " + std::to_string(dat1->_jumpCanceled) + " vs " + std::to_string(dat2->_jumpCanceled));
		if (dat1->_hadUltimate != dat2->_hadUltimate)
			game->logger.fatal(std::string(msgStart) + "Character::_hadUltimate: " + std::to_string(dat1->_hadUltimate) + " vs " + std::to_string(dat2->_hadUltimate));
		if (dat1->_grabInvul != dat2->_grabInvul)
			game->logger.fatal(std::string(msgStart) + "Character::_grabInvul: " + std::to_string(dat1->_grabInvul) + " vs " + std::to_string(dat2->_grabInvul));
		if (dat1->_ultimateUsed != dat2->_ultimateUsed)
			game->logger.fatal(std::string(msgStart) + "Character::_ultimateUsed: " + std::to_string(dat1->_ultimateUsed) + " vs " + std::to_string(dat2->_ultimateUsed));
		if (dat1->_normalTreeFlag != dat2->_normalTreeFlag)
			game->logger.fatal(std::string(msgStart) + "Character::_normalTreeFlag: " + std::to_string(dat1->_normalTreeFlag) + " vs " + std::to_string(dat2->_normalTreeFlag));
		if (dat1->_inputBuffer.horizontalAxis != dat2->_inputBuffer.horizontalAxis)
			game->logger.fatal(std::string(msgStart) + "Character::_inputBuffer::horizontalAxis: " + std::to_string(dat1->_inputBuffer.horizontalAxis) + " vs " + std::to_string(dat2->_inputBuffer.horizontalAxis));
		if (dat1->_inputBuffer.verticalAxis != dat2->_inputBuffer.verticalAxis)
			game->logger.fatal(std::string(msgStart) + "Character::_inputBuffer::verticalAxis: " + std::to_string(dat1->_inputBuffer.verticalAxis) + " vs " + std::to_string(dat2->_inputBuffer.verticalAxis));
		if (dat1->_inputBuffer.n != dat2->_inputBuffer.n)
			game->logger.fatal(std::string(msgStart) + "Character::_inputBuffer::n: " + std::to_string(dat1->_inputBuffer.n) + " vs " + std::to_string(dat2->_inputBuffer.n));
		if (dat1->_inputBuffer.m != dat2->_inputBuffer.m)
			game->logger.fatal(std::string(msgStart) + "Character::_inputBuffer::m: " + std::to_string(dat1->_inputBuffer.m) + " vs " + std::to_string(dat2->_inputBuffer.m));
		if (dat1->_inputBuffer.s != dat2->_inputBuffer.s)
			game->logger.fatal(std::string(msgStart) + "Character::_inputBuffer::s: " + std::to_string(dat1->_inputBuffer.s) + " vs " + std::to_string(dat2->_inputBuffer.s));
		if (dat1->_inputBuffer.v != dat2->_inputBuffer.v)
			game->logger.fatal(std::string(msgStart) + "Character::_inputBuffer::v: " + std::to_string(dat1->_inputBuffer.v) + " vs " + std::to_string(dat2->_inputBuffer.v));
		if (dat1->_inputBuffer.a != dat2->_inputBuffer.a)
			game->logger.fatal(std::string(msgStart) + "Character::_inputBuffer::a: " + std::to_string(dat1->_inputBuffer.a) + " vs " + std::to_string(dat2->_inputBuffer.a));
		if (dat1->_inputBuffer.d != dat2->_inputBuffer.d)
			game->logger.fatal(std::string(msgStart) + "Character::_inputBuffer::d: " + std::to_string(dat1->_inputBuffer.d) + " vs " + std::to_string(dat2->_inputBuffer.d));
		if (dat1->_inputBuffer.pause != dat2->_inputBuffer.pause)
			game->logger.fatal(std::string(msgStart) + "Character::_inputBuffer::pause: " + std::to_string(dat1->_inputBuffer.pause) + " vs " + std::to_string(dat2->_inputBuffer.pause));
		if (dat1->_guardRegenCd != dat2->_guardRegenCd)
			game->logger.fatal(std::string(msgStart) + "Character::_guardRegenCd: " + std::to_string(dat1->_guardRegenCd) + " vs " + std::to_string(dat2->_guardRegenCd));
		if (dat1->_odCooldown != dat2->_odCooldown)
			game->logger.fatal(std::string(msgStart) + "Character::_odCooldown: " + std::to_string(dat1->_odCooldown) + " vs " + std::to_string(dat2->_odCooldown));
		if (dat1->_counter != dat2->_counter)
			game->logger.fatal(std::string(msgStart) + "Character::_counter: " + std::to_string(dat1->_counter) + " vs " + std::to_string(dat2->_counter));
		if (dat1->_blockStun != dat2->_blockStun)
			game->logger.fatal(std::string(msgStart) + "Character::_blockStun: " + std::to_string(dat1->_blockStun) + " vs " + std::to_string(dat2->_blockStun));
		if (dat1->_jumpsUsed != dat2->_jumpsUsed)
			game->logger.fatal(std::string(msgStart) + "Character::_jumpsUsed: " + std::to_string(dat1->_jumpsUsed) + " vs " + std::to_string(dat2->_jumpsUsed));
		if (dat1->_airDashesUsed != dat2->_airDashesUsed)
			game->logger.fatal(std::string(msgStart) + "Character::_airDashesUsed: " + std::to_string(dat1->_airDashesUsed) + " vs " + std::to_string(dat2->_airDashesUsed));
		if (dat1->_comboCtr != dat2->_comboCtr)
			game->logger.fatal(std::string(msgStart) + "Character::_comboCtr: " + std::to_string(dat1->_comboCtr) + " vs " + std::to_string(dat2->_comboCtr));
		if (dat1->_totalDamage != dat2->_totalDamage)
			game->logger.fatal(std::string(msgStart) + "Character::_totalDamage: " + std::to_string(dat1->_totalDamage) + " vs " + std::to_string(dat2->_totalDamage));
		if (dat1->_prorate != dat2->_prorate)
			game->logger.fatal(std::string(msgStart) + "Character::_prorate: " + std::to_string(dat1->_prorate) + " vs " + std::to_string(dat2->_prorate));
		if (dat1->_atkDisabled != dat2->_atkDisabled)
			game->logger.fatal(std::string(msgStart) + "Character::_atkDisabled: " + std::to_string(dat1->_atkDisabled) + " vs " + std::to_string(dat2->_atkDisabled));
		if (dat1->_inputDisabled != dat2->_inputDisabled)
			game->logger.fatal(std::string(msgStart) + "Character::_inputDisabled: " + std::to_string(dat1->_inputDisabled) + " vs " + std::to_string(dat2->_inputDisabled));
		if (dat1->_hasJumped != dat2->_hasJumped)
			game->logger.fatal(std::string(msgStart) + "Character::_hasJumped: " + std::to_string(dat1->_hasJumped) + " vs " + std::to_string(dat2->_hasJumped));
		if (dat1->_restand != dat2->_restand)
			game->logger.fatal(std::string(msgStart) + "Character::_restand: " + std::to_string(dat1->_restand) + " vs " + std::to_string(dat2->_restand));
		if (dat1->_justGotCorner != dat2->_justGotCorner)
			game->logger.fatal(std::string(msgStart) + "Character::_justGotCorner: " + std::to_string(dat1->_justGotCorner) + " vs " + std::to_string(dat2->_justGotCorner));
		if (dat1->_regen != dat2->_regen)
			game->logger.fatal(std::string(msgStart) + "Character::_regen: " + std::to_string(dat1->_regen) + " vs " + std::to_string(dat2->_regen));
		if (dat1->_mana != dat2->_mana)
			game->logger.fatal(std::string(msgStart) + "Character::_mana: " + std::to_string(dat1->_mana) + " vs " + std::to_string(dat2->_mana));
		if (dat1->_guardCooldown != dat2->_guardCooldown)
			game->logger.fatal(std::string(msgStart) + "Character::_guardCooldown: " + std::to_string(dat1->_guardCooldown) + " vs " + std::to_string(dat2->_guardCooldown));
		if (dat1->_guardBar != dat2->_guardBar)
			game->logger.fatal(std::string(msgStart) + "Character::_guardBar: " + std::to_string(dat1->_guardBar) + " vs " + std::to_string(dat2->_guardBar));
		if (dat1->_stallingFactor != dat2->_stallingFactor)
			game->logger.fatal(std::string(msgStart) + "Character::_stallingFactor: " + std::to_string(dat1->_stallingFactor) + " vs " + std::to_string(dat2->_stallingFactor));
		if (dat1->_willGroundSlam != dat2->_willGroundSlam)
			game->logger.fatal(std::string(msgStart) + "Character::_willGroundSlam: " + std::to_string(dat1->_willGroundSlam) + " vs " + std::to_string(dat2->_willGroundSlam));
		if (dat1->_willWallSplat != dat2->_willWallSplat)
			game->logger.fatal(std::string(msgStart) + "Character::_willWallSplat: " + std::to_string(dat1->_willWallSplat) + " vs " + std::to_string(dat2->_willWallSplat));
		if (dat1->_doubleGravity != dat2->_doubleGravity)
			game->logger.fatal(std::string(msgStart) + "Character::_doubleGravity: " + std::to_string(dat1->_doubleGravity) + " vs " + std::to_string(dat2->_doubleGravity));
		if (dat1->_guardBarTmp != dat2->_guardBarTmp)
			game->logger.fatal(std::string(msgStart) + "Character::_guardBarTmp: " + std::to_string(dat1->_guardBarTmp) + " vs " + std::to_string(dat2->_guardBarTmp));
		if (dat1->_airMovementUsed != dat2->_airMovementUsed)
			game->logger.fatal(std::string(msgStart) + "Character::_airMovementUsed: " + std::to_string(dat1->_airMovementUsed) + " vs " + std::to_string(dat2->_airMovementUsed));
		if (memcmp(dat1->_specialInputs, dat2->_specialInputs, sizeof(dat1->_specialInputs)) != 0) {
			char number1[3];
			char number2[3];
			auto *ptr1 = (SpecialInputs *)dat1->_specialInputs;
			auto *ptr2 = (SpecialInputs *)dat2->_specialInputs;

			for (unsigned i = 0; i < sizeof(dat1->_specialInputs); i++) {
				if (dat1->_specialInputs[i] == dat2->_specialInputs[i])
					continue;
				sprintf(number1, "%02X", dat1->_specialInputs[i]);
				sprintf(number2, "%02X", dat2->_specialInputs[i]);
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs[" + std::to_string(i) + "]: " + number1 + " vs " + number2);
			}
			if (ptr1->_22 != ptr2->_22)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_22: " + std::to_string(ptr1->_22) + " vs " + std::to_string(ptr2->_22));
			if (ptr1->_44 != ptr2->_44)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_44: " + std::to_string(ptr1->_44) + " vs " + std::to_string(ptr2->_44));
			if (ptr1->_66 != ptr2->_66)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_66: " + std::to_string(ptr1->_66) + " vs " + std::to_string(ptr2->_66));
			if (ptr1->_27 != ptr2->_27)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_27: " + std::to_string(ptr1->_27) + " vs " + std::to_string(ptr2->_27));
			if (ptr1->_28 != ptr2->_28)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_28: " + std::to_string(ptr1->_28) + " vs " + std::to_string(ptr2->_28));
			if (ptr1->_29 != ptr2->_29)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_29: " + std::to_string(ptr1->_29) + " vs " + std::to_string(ptr2->_29));
			if (ptr1->_an != ptr2->_an)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_an: " + std::to_string(ptr1->_an) + " vs " + std::to_string(ptr2->_an));
			if (ptr1->_am != ptr2->_am)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_am: " + std::to_string(ptr1->_am) + " vs " + std::to_string(ptr2->_am));
			if (ptr1->_as != ptr2->_as)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_as: " + std::to_string(ptr1->_as) + " vs " + std::to_string(ptr2->_as));
			if (ptr1->_av != ptr2->_av)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_av: " + std::to_string(ptr1->_av) + " vs " + std::to_string(ptr2->_av));
			if (ptr1->_c28n != ptr2->_c28n)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c28n: " + std::to_string(ptr1->_c28n) + " vs " + std::to_string(ptr2->_c28n));
			if (ptr1->_c28m != ptr2->_c28m)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c28m: " + std::to_string(ptr1->_c28m) + " vs " + std::to_string(ptr2->_c28m));
			if (ptr1->_c28s != ptr2->_c28s)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c28s: " + std::to_string(ptr1->_c28s) + " vs " + std::to_string(ptr2->_c28s));
			if (ptr1->_c28v != ptr2->_c28v)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c28v: " + std::to_string(ptr1->_c28v) + " vs " + std::to_string(ptr2->_c28v));
			if (ptr1->_c28d != ptr2->_c28d)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c28d: " + std::to_string(ptr1->_c28d) + " vs " + std::to_string(ptr2->_c28d));
			if (ptr1->_c28a != ptr2->_c28a)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c28a: " + std::to_string(ptr1->_c28a) + " vs " + std::to_string(ptr2->_c28a));
			if (ptr1->_c46n != ptr2->_c46n)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c46n: " + std::to_string(ptr1->_c46n) + " vs " + std::to_string(ptr2->_c46n));
			if (ptr1->_c46m != ptr2->_c46m)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c46m: " + std::to_string(ptr1->_c46m) + " vs " + std::to_string(ptr2->_c46m));
			if (ptr1->_c46s != ptr2->_c46s)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c46s: " + std::to_string(ptr1->_c46s) + " vs " + std::to_string(ptr2->_c46s));
			if (ptr1->_c46v != ptr2->_c46v)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c46v: " + std::to_string(ptr1->_c46v) + " vs " + std::to_string(ptr2->_c46v));
			if (ptr1->_c46d != ptr2->_c46d)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c46d: " + std::to_string(ptr1->_c46d) + " vs " + std::to_string(ptr2->_c46d));
			if (ptr1->_c46a != ptr2->_c46a)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c46a: " + std::to_string(ptr1->_c46a) + " vs " + std::to_string(ptr2->_c46a));
			if (ptr1->_c64n != ptr2->_c64n)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c64n: " + std::to_string(ptr1->_c64n) + " vs " + std::to_string(ptr2->_c64n));
			if (ptr1->_c64m != ptr2->_c64m)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c64m: " + std::to_string(ptr1->_c64m) + " vs " + std::to_string(ptr2->_c64m));
			if (ptr1->_c64s != ptr2->_c64s)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c64s: " + std::to_string(ptr1->_c64s) + " vs " + std::to_string(ptr2->_c64s));
			if (ptr1->_c64v != ptr2->_c64v)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c64v: " + std::to_string(ptr1->_c64v) + " vs " + std::to_string(ptr2->_c64v));
			if (ptr1->_c64d != ptr2->_c64d)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c64d: " + std::to_string(ptr1->_c64d) + " vs " + std::to_string(ptr2->_c64d));
			if (ptr1->_c64a != ptr2->_c64a)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_c64a: " + std::to_string(ptr1->_c64a) + " vs " + std::to_string(ptr2->_c64a));
			if (ptr1->_236n != ptr2->_236n)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_236n: " + std::to_string(ptr1->_236n) + " vs " + std::to_string(ptr2->_236n));
			if (ptr1->_236m != ptr2->_236m)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_236m: " + std::to_string(ptr1->_236m) + " vs " + std::to_string(ptr2->_236m));
			if (ptr1->_236s != ptr2->_236s)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_236s: " + std::to_string(ptr1->_236s) + " vs " + std::to_string(ptr2->_236s));
			if (ptr1->_236v != ptr2->_236v)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_236v: " + std::to_string(ptr1->_236v) + " vs " + std::to_string(ptr2->_236v));
			if (ptr1->_236d != ptr2->_236d)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_236d: " + std::to_string(ptr1->_236d) + " vs " + std::to_string(ptr2->_236d));
			if (ptr1->_236a != ptr2->_236a)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_236a: " + std::to_string(ptr1->_236a) + " vs " + std::to_string(ptr2->_236a));
			if (ptr1->_214n != ptr2->_214n)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_214n: " + std::to_string(ptr1->_214n) + " vs " + std::to_string(ptr2->_214n));
			if (ptr1->_214m != ptr2->_214m)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_214m: " + std::to_string(ptr1->_214m) + " vs " + std::to_string(ptr2->_214m));
			if (ptr1->_214s != ptr2->_214s)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_214s: " + std::to_string(ptr1->_214s) + " vs " + std::to_string(ptr2->_214s));
			if (ptr1->_214v != ptr2->_214v)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_214v: " + std::to_string(ptr1->_214v) + " vs " + std::to_string(ptr2->_214v));
			if (ptr1->_214d != ptr2->_214d)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_214d: " + std::to_string(ptr1->_214d) + " vs " + std::to_string(ptr2->_214d));
			if (ptr1->_214a != ptr2->_214a)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_214a: " + std::to_string(ptr1->_214a) + " vs " + std::to_string(ptr2->_214a));
			if (ptr1->_623n != ptr2->_623n)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_623n: " + std::to_string(ptr1->_623n) + " vs " + std::to_string(ptr2->_623n));
			if (ptr1->_623m != ptr2->_623m)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_623m: " + std::to_string(ptr1->_623m) + " vs " + std::to_string(ptr2->_623m));
			if (ptr1->_623s != ptr2->_623s)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_623s: " + std::to_string(ptr1->_623s) + " vs " + std::to_string(ptr2->_623s));
			if (ptr1->_623v != ptr2->_623v)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_623v: " + std::to_string(ptr1->_623v) + " vs " + std::to_string(ptr2->_623v));
			if (ptr1->_623d != ptr2->_623d)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_623d: " + std::to_string(ptr1->_623d) + " vs " + std::to_string(ptr2->_623d));
			if (ptr1->_623a != ptr2->_623a)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_623a: " + std::to_string(ptr1->_623a) + " vs " + std::to_string(ptr2->_623a));
			if (ptr1->_421n != ptr2->_421n)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_421n: " + std::to_string(ptr1->_421n) + " vs " + std::to_string(ptr2->_421n));
			if (ptr1->_421m != ptr2->_421m)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_421m: " + std::to_string(ptr1->_421m) + " vs " + std::to_string(ptr2->_421m));
			if (ptr1->_421s != ptr2->_421s)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_421s: " + std::to_string(ptr1->_421s) + " vs " + std::to_string(ptr2->_421s));
			if (ptr1->_421v != ptr2->_421v)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_421v: " + std::to_string(ptr1->_421v) + " vs " + std::to_string(ptr2->_421v));
			if (ptr1->_421d != ptr2->_421d)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_421d: " + std::to_string(ptr1->_421d) + " vs " + std::to_string(ptr2->_421d));
			if (ptr1->_421a != ptr2->_421a)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_421a: " + std::to_string(ptr1->_421a) + " vs " + std::to_string(ptr2->_421a));
			if (ptr1->_624n != ptr2->_624n)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_624n: " + std::to_string(ptr1->_624n) + " vs " + std::to_string(ptr2->_624n));
			if (ptr1->_624m != ptr2->_624m)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_624m: " + std::to_string(ptr1->_624m) + " vs " + std::to_string(ptr2->_624m));
			if (ptr1->_624s != ptr2->_624s)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_624s: " + std::to_string(ptr1->_624s) + " vs " + std::to_string(ptr2->_624s));
			if (ptr1->_624v != ptr2->_624v)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_624v: " + std::to_string(ptr1->_624v) + " vs " + std::to_string(ptr2->_624v));
			if (ptr1->_624d != ptr2->_624d)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_624d: " + std::to_string(ptr1->_624d) + " vs " + std::to_string(ptr2->_624d));
			if (ptr1->_624a != ptr2->_624a)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_624a: " + std::to_string(ptr1->_624a) + " vs " + std::to_string(ptr2->_624a));
			if (ptr1->_426n != ptr2->_426n)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_426n: " + std::to_string(ptr1->_426n) + " vs " + std::to_string(ptr2->_426n));
			if (ptr1->_426m != ptr2->_426m)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_426m: " + std::to_string(ptr1->_426m) + " vs " + std::to_string(ptr2->_426m));
			if (ptr1->_426s != ptr2->_426s)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_426s: " + std::to_string(ptr1->_426s) + " vs " + std::to_string(ptr2->_426s));
			if (ptr1->_426v != ptr2->_426v)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_426v: " + std::to_string(ptr1->_426v) + " vs " + std::to_string(ptr2->_426v));
			if (ptr1->_426d != ptr2->_426d)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_426d: " + std::to_string(ptr1->_426d) + " vs " + std::to_string(ptr2->_426d));
			if (ptr1->_426a != ptr2->_426a)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_426a: " + std::to_string(ptr1->_426a) + " vs " + std::to_string(ptr2->_426a));
			if (ptr1->_624684n != ptr2->_624684n)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_624684n: " + std::to_string(ptr1->_624684n) + " vs " + std::to_string(ptr2->_624684n));
			if (ptr1->_624684m != ptr2->_624684m)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_624684m: " + std::to_string(ptr1->_624684m) + " vs " + std::to_string(ptr2->_624684m));
			if (ptr1->_624684s != ptr2->_624684s)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_624684s: " + std::to_string(ptr1->_624684s) + " vs " + std::to_string(ptr2->_624684s));
			if (ptr1->_624684v != ptr2->_624684v)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_624684v: " + std::to_string(ptr1->_624684v) + " vs " + std::to_string(ptr2->_624684v));
			if (ptr1->_624684d != ptr2->_624684d)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_624684d: " + std::to_string(ptr1->_624684d) + " vs " + std::to_string(ptr2->_624684d));
			if (ptr1->_624684a != ptr2->_624684a)
				game->logger.fatal(std::string(msgStart) + "Character::_specialInputs::_624684a: " + std::to_string(ptr1->_624684a) + " vs " + std::to_string(ptr2->_624684a));
		}
		if (dat1->_limit[0] != dat2->_limit[0])
			game->logger.fatal(std::string(msgStart) + "Character::_limit[0]: " + std::to_string(dat1->_limit[0]) + " vs " + std::to_string(dat2->_limit[0]));
		if (dat1->_limit[1] != dat2->_limit[1])
			game->logger.fatal(std::string(msgStart) + "Character::_limit[1]: " + std::to_string(dat1->_limit[1]) + " vs " + std::to_string(dat2->_limit[1]));
		if (dat1->_limit[2] != dat2->_limit[2])
			game->logger.fatal(std::string(msgStart) + "Character::_limit[2]: " + std::to_string(dat1->_limit[2]) + " vs " + std::to_string(dat2->_limit[2]));
		if (dat1->_limit[3] != dat2->_limit[3])
			game->logger.fatal(std::string(msgStart) + "Character::_limit[3]: " + std::to_string(dat1->_limit[3]) + " vs " + std::to_string(dat2->_limit[3]));

		if (dat1->_nbReplayInputs != dat2->_nbReplayInputs)
			game->logger.fatal(std::string(msgStart) + "Character::_nbReplayInputs: " + std::to_string(dat1->_nbReplayInputs) + " vs " + std::to_string(dat2->_nbReplayInputs));
		if (dat1->_nbLastInputs != dat2->_nbLastInputs)
			game->logger.fatal(std::string(msgStart) + "Character::_nbLastInputs: " + std::to_string(dat1->_nbLastInputs) + " vs " + std::to_string(dat2->_nbLastInputs));
		if (dat1->_nbUsedMoves != dat2->_nbUsedMoves)
			game->logger.fatal(std::string(msgStart) + "Character::_nbUsedMoves: " + std::to_string(dat1->_nbUsedMoves) + " vs " + std::to_string(dat2->_nbUsedMoves));

		if (dat1->_nbReplayInputs != dat2->_nbReplayInputs || dat1->_nbLastInputs != dat2->_nbLastInputs || dat1->_nbUsedMoves != dat2->_nbUsedMoves)
			return 0;

		return length +
			sizeof(Data) +
			sizeof(LastInput) * dat1->_nbLastInputs +
			sizeof(ReplayData) * dat1->_nbReplayInputs +
			sizeof(unsigned) * 2 * dat1->_nbUsedMoves;
	}

	Character::SubObjectAnchor Character::anchorFromString(const std::string &str)
	{
		if (str == "owner")
			return ANCHOR_OWNER;
		if (str == "opponent")
			return ANCHOR_OPPONENT;
		if (str == "border_front")
			return ANCHOR_BORDER_FRONT;
		if (str == "border_back")
			return ANCHOR_BORDER_BACK;
		if (str == "stage_min")
			return ANCHOR_STAGE_MIN;
		if (str == "stage_max")
			return ANCHOR_STAGE_MAX;
		if (str == "stage_center")
			return ANCHOR_STAGE_CENTER;
		throw std::invalid_argument("Invalid anchor '" + str + "'");
	}

	Character::SubObjectDirection Character::directionFromString(const std::string &str)
	{
		if (str == "front")
			return DIRECTION_FRONT;
		if (str == "back")
			return DIRECTION_BACK;
		if (str == "left")
			return DIRECTION_LEFT;
		if (str == "right")
			return DIRECTION_RIGHT;
		if (str == "op_front")
			return DIRECTION_OP_FRONT;
		if (str == "op_back")
			return DIRECTION_OP_BACK;
		throw std::invalid_argument("Invalid dir '" + str + "'");
	}

	void Character::_loadProjectileData(const std::string &path)
	{
		nlohmann::json j = nlohmann::json::parse(game->fileMgr.readFull(path));

		for (auto &i : j)  {
			SubObjectData pdat;

			pdat.json = i;
			pdat.action = i["action"];
			pdat.offset.x = i["spawn_offset"]["x"];
			pdat.offset.y = i["spawn_offset"]["y"];
			pdat.anchor.x = Character::anchorFromString(i["spawn_offset"]["anchorX"]);
			pdat.anchor.y = Character::anchorFromString(i["spawn_offset"]["anchorY"]);
			pdat.dir = Character::directionFromString(i["dir"]);
			this->_projectileData[i["index"]] = pdat;
		}
	}

	void Character::_loadParticleData(const std::string &path, const std::string &folder)
	{
		nlohmann::json j = nlohmann::json::parse(game->fileMgr.readFull(path));

		this->_generators.reserve(j.size());
		for (auto &i : j)
			this->_generators.emplace_back(i, folder);
	}

	Vector2f Character::_calcProjectilePosition(const SubObjectData &pdat, float dir)
	{
		Vector2f pos{
			this->_getAnchoredPos(pdat, false),
			this->_getAnchoredPos(pdat, true)
		};

		pos.x += pdat.offset.x * dir;
		pos.y += pdat.offset.y;
		return pos;
	}

	float Character::_getAnchoredPos(const Character::SubObjectData &data, bool y)
	{
		switch ((&data.anchor.x)[y]) {
		case ANCHOR_OWNER:
			return (&this->_position.x)[y];
		case ANCHOR_OPPONENT:
			return (&this->_opponent->_position.x)[y];
		case ANCHOR_BORDER_FRONT:
			return this->_direction * STAGE_X_MAX;
		case ANCHOR_BORDER_BACK:
			return !this->_direction * STAGE_X_MAX;
		case ANCHOR_STAGE_MIN:
			return y ? STAGE_Y_MIN : STAGE_X_MIN;
		case ANCHOR_STAGE_MAX:
			return y ? STAGE_Y_MAX : STAGE_X_MAX;
		case ANCHOR_STAGE_CENTER:
			return y ? (STAGE_Y_MIN + STAGE_Y_MAX) / 2 : (STAGE_X_MIN + STAGE_X_MAX) / 2;
		}
		my_assert(false);
	}

	bool Character::_getProjectileDirection(const Character::SubObjectData &data)
	{
		switch (data.dir) {
		case DIRECTION_FRONT:
			return this->_direction;
		case DIRECTION_BACK:
			return !this->_direction;
		case DIRECTION_LEFT:
			return false;
		case DIRECTION_RIGHT:
			return true;
		case DIRECTION_OP_FRONT:
			return this->_opponent->_direction;
		case DIRECTION_OP_BACK:
			return !this->_opponent->_direction;
		}
		my_assert(false);
	}

	bool Character::isHit() const
	{
		return isHitAction(this->_action);
	}

	int Character::getLayer() const
	{
		return 0;
	}

	void Character::drawSpecialHUD(sf::RenderTarget &)
	{
	}

	void Character::drawSpecialHUDNoReverse(sf::RenderTarget &texture)
	{
		if (this->showAttributes) {
			texture.draw(this->_text);
			texture.draw(this->_text2);
		}
	}

	void Character::_computeFrameDataCache()
	{
		Object::_computeFrameDataCache();
		this->_fdCache.oFlag.voidElement   &= !this->_neutralEffectTimer;
		this->_fdCache.oFlag.matterElement &= !this->_neutralEffectTimer;
		this->_fdCache.oFlag.spiritElement &= !this->_neutralEffectTimer;
		if (this->_voidInstallTimer || this->_matterInstallTimer || this->_spiritInstallTimer) {
			this->_fdCache.oFlag.voidElement = this->_voidInstallTimer;
			this->_fdCache.oFlag.matterElement = this->_matterInstallTimer;
			this->_fdCache.oFlag.spiritElement = this->_spiritInstallTimer;
		}
		this->_fdCache.dFlag.grabInvulnerable |= this->_grabInvul;
	}

	std::array<unsigned, 4> Character::getLimit() const
	{
		return this->_limit;
	}

	void Character::_onSubObjectHit()
	{
	}

	bool Character::isMovementCancelable(unsigned int action, bool cancelable)
	{
		switch (action) {
		case ACTION_WALK_FORWARD:
		case ACTION_WALK_BACKWARD:
		case ACTION_IDLE:
		case ACTION_FALLING:
		case ACTION_LANDING:
			return true;
		case ACTION_NEUTRAL_JUMP:
		case ACTION_FORWARD_JUMP:
		case ACTION_BACKWARD_JUMP:
		case ACTION_NEUTRAL_HIGH_JUMP:
		case ACTION_FORWARD_HIGH_JUMP:
		case ACTION_BACKWARD_HIGH_JUMP:
		case ACTION_AIR_DASH_1:
		case ACTION_AIR_DASH_2:
		case ACTION_AIR_DASH_3:
		case ACTION_AIR_DASH_4:
		case ACTION_AIR_DASH_6:
		case ACTION_AIR_DASH_7:
		case ACTION_AIR_DASH_8:
		case ACTION_AIR_DASH_9:
		case ACTION_NEUTRAL_AIR_JUMP:
		case ACTION_FORWARD_AIR_JUMP:
		case ACTION_BACKWARD_AIR_JUMP:
			return cancelable;
		default:
			return false;
		}
	}

	bool Character::isReversalAction(unsigned int action)
	{
		switch (action) {
		case ACTION_GROUND_LOW_REVERSAL:
		case ACTION_GROUND_HIGH_REVERSAL:
		case ACTION_AIR_REVERSAL:
			return true;
		default:
			return false;
		}
	}

	size_t Character::printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const
	{
		auto length = Object::printContent(msgStart, data, startOffset, dataSize);

		if (length == 0)
			return 0;

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + length);
		auto len = length +
		           sizeof(Data) +
		           sizeof(LastInput) * dat->_nbLastInputs +
		           sizeof(ReplayData) * dat->_nbReplayInputs +
		           sizeof(unsigned) * 2 * dat->_nbUsedMoves;

		if (startOffset + len >= dataSize)
			game->logger.warn("Object is " + std::to_string(startOffset + len - dataSize) + " bytes bigger than input");
		game->logger.info("Character @" + std::to_string(startOffset + length));
		game->logger.info(std::string(msgStart) + "Character::_guardBarTmp: " + std::to_string(dat->_guardBarTmp));
		game->logger.info(std::string(msgStart) + "Character::_hardKD: " + std::to_string(dat->_hardKD));
		game->logger.info(std::string(msgStart) + "Character::_limitEffects: " + std::to_string(dat->_limitEffects));
		game->logger.info(std::string(msgStart) + "Character::_neutralEffectTimer: " + std::to_string(dat->_neutralEffectTimer));
		game->logger.info(std::string(msgStart) + "Character::_matterEffectTimer: " + std::to_string(dat->_matterEffectTimer));
		game->logger.info(std::string(msgStart) + "Character::_spiritEffectTimer: " + std::to_string(dat->_spiritEffectTimer));
		game->logger.info(std::string(msgStart) + "Character::_voidEffectTimer: " + std::to_string(dat->_voidEffectTimer));
		game->logger.info(std::string(msgStart) + "Character::_hitStop: " + std::to_string(dat->_hitStop));
		game->logger.info(std::string(msgStart) + "Character::_jumpCanceled: " + std::to_string(dat->_jumpCanceled));
		game->logger.info(std::string(msgStart) + "Character::_hadUltimate: " + std::to_string(dat->_hadUltimate));
		game->logger.info(std::string(msgStart) + "Character::_grabInvul: " + std::to_string(dat->_grabInvul));
		game->logger.info(std::string(msgStart) + "Character::_ultimateUsed: " + std::to_string(dat->_ultimateUsed));
		game->logger.info(std::string(msgStart) + "Character::_normalTreeFlag: " + std::to_string(dat->_normalTreeFlag));
		game->logger.info(std::string(msgStart) + "Character::_inputBuffer::horizontalAxis: " + std::to_string(dat->_inputBuffer.horizontalAxis));
		game->logger.info(std::string(msgStart) + "Character::_inputBuffer::verticalAxis: " + std::to_string(dat->_inputBuffer.verticalAxis));
		game->logger.info(std::string(msgStart) + "Character::_inputBuffer::n: " + std::to_string(dat->_inputBuffer.n));
		game->logger.info(std::string(msgStart) + "Character::_inputBuffer::m: " + std::to_string(dat->_inputBuffer.m));
		game->logger.info(std::string(msgStart) + "Character::_inputBuffer::s: " + std::to_string(dat->_inputBuffer.s));
		game->logger.info(std::string(msgStart) + "Character::_inputBuffer::v: " + std::to_string(dat->_inputBuffer.v));
		game->logger.info(std::string(msgStart) + "Character::_inputBuffer::a: " + std::to_string(dat->_inputBuffer.a));
		game->logger.info(std::string(msgStart) + "Character::_inputBuffer::d: " + std::to_string(dat->_inputBuffer.d));
		game->logger.info(std::string(msgStart) + "Character::_inputBuffer::pause: " + std::to_string(dat->_inputBuffer.pause));
		game->logger.info(std::string(msgStart) + "Character::_guardRegenCd: " + std::to_string(dat->_guardRegenCd));
		game->logger.info(std::string(msgStart) + "Character::_odCooldown: " + std::to_string(dat->_odCooldown));
		game->logger.info(std::string(msgStart) + "Character::_counter: " + std::to_string(dat->_counter));
		game->logger.info(std::string(msgStart) + "Character::_blockStun: " + std::to_string(dat->_blockStun));
		game->logger.info(std::string(msgStart) + "Character::_jumpsUsed: " + std::to_string(dat->_jumpsUsed));
		game->logger.info(std::string(msgStart) + "Character::_airDashesUsed: " + std::to_string(dat->_airDashesUsed));
		game->logger.info(std::string(msgStart) + "Character::_comboCtr: " + std::to_string(dat->_comboCtr));
		game->logger.info(std::string(msgStart) + "Character::_totalDamage: " + std::to_string(dat->_totalDamage));
		game->logger.info(std::string(msgStart) + "Character::_prorate: " + std::to_string(dat->_prorate));
		game->logger.info(std::string(msgStart) + "Character::_atkDisabled: " + std::to_string(dat->_atkDisabled));
		game->logger.info(std::string(msgStart) + "Character::_inputDisabled: " + std::to_string(dat->_inputDisabled));
		game->logger.info(std::string(msgStart) + "Character::_hasJumped: " + std::to_string(dat->_hasJumped));
		game->logger.info(std::string(msgStart) + "Character::_restand: " + std::to_string(dat->_restand));
		game->logger.info(std::string(msgStart) + "Character::_justGotCorner: " + std::to_string(dat->_justGotCorner));
		game->logger.info(std::string(msgStart) + "Character::_regen: " + std::to_string(dat->_regen));
		game->logger.info(std::string(msgStart) + "Character::_mana: " + std::to_string(dat->_mana));
		game->logger.info(std::string(msgStart) + "Character::_guardCooldown: " + std::to_string(dat->_guardCooldown));
		game->logger.info(std::string(msgStart) + "Character::_guardBar: " + std::to_string(dat->_guardBar));
		game->logger.info(std::string(msgStart) + "Character::_stallingFactor: " + std::to_string(dat->_stallingFactor));
		game->logger.info(std::string(msgStart) + "Character::_willGroundSlam: " + std::to_string(dat->_willGroundSlam));
		game->logger.info(std::string(msgStart) + "Character::_willWallSplat: " + std::to_string(dat->_willWallSplat));
		game->logger.info(std::string(msgStart) + "Character::_doubleGravity: " + std::to_string(dat->_doubleGravity));
		game->logger.info(std::string(msgStart) + "Character::_airMovementUsed: " + std::to_string(dat->_airMovementUsed));

		char number[3];
		auto *ptr = (SpecialInputs *)dat->_specialInputs;

		for (unsigned i = 0; i < sizeof(dat->_specialInputs); i++) {
			sprintf(number, "%02X", dat->_specialInputs[i]);
			game->logger.info(std::string(msgStart) + "Character::_specialInputs[" + std::to_string(i) + "]: " + number);
		}
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_22: " + std::to_string(ptr->_22));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_44: " + std::to_string(ptr->_44));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_66: " + std::to_string(ptr->_66));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_27: " + std::to_string(ptr->_27));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_28: " + std::to_string(ptr->_28));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_29: " + std::to_string(ptr->_29));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_an: " + std::to_string(ptr->_an));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_am: " + std::to_string(ptr->_am));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_as: " + std::to_string(ptr->_as));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_av: " + std::to_string(ptr->_av));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c28n: " + std::to_string(ptr->_c28n));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c28m: " + std::to_string(ptr->_c28m));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c28s: " + std::to_string(ptr->_c28s));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c28v: " + std::to_string(ptr->_c28v));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c28d: " + std::to_string(ptr->_c28d));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c28a: " + std::to_string(ptr->_c28a));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c46n: " + std::to_string(ptr->_c46n));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c46m: " + std::to_string(ptr->_c46m));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c46s: " + std::to_string(ptr->_c46s));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c46v: " + std::to_string(ptr->_c46v));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c46d: " + std::to_string(ptr->_c46d));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c46a: " + std::to_string(ptr->_c46a));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c64n: " + std::to_string(ptr->_c64n));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c64m: " + std::to_string(ptr->_c64m));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c64s: " + std::to_string(ptr->_c64s));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c64v: " + std::to_string(ptr->_c64v));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c64d: " + std::to_string(ptr->_c64d));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_c64a: " + std::to_string(ptr->_c64a));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_236n: " + std::to_string(ptr->_236n));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_236m: " + std::to_string(ptr->_236m));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_236s: " + std::to_string(ptr->_236s));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_236v: " + std::to_string(ptr->_236v));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_236d: " + std::to_string(ptr->_236d));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_236a: " + std::to_string(ptr->_236a));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_214n: " + std::to_string(ptr->_214n));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_214m: " + std::to_string(ptr->_214m));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_214s: " + std::to_string(ptr->_214s));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_214v: " + std::to_string(ptr->_214v));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_214d: " + std::to_string(ptr->_214d));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_214a: " + std::to_string(ptr->_214a));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_623n: " + std::to_string(ptr->_623n));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_623m: " + std::to_string(ptr->_623m));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_623s: " + std::to_string(ptr->_623s));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_623v: " + std::to_string(ptr->_623v));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_623d: " + std::to_string(ptr->_623d));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_623a: " + std::to_string(ptr->_623a));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_421n: " + std::to_string(ptr->_421n));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_421m: " + std::to_string(ptr->_421m));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_421s: " + std::to_string(ptr->_421s));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_421v: " + std::to_string(ptr->_421v));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_421d: " + std::to_string(ptr->_421d));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_421a: " + std::to_string(ptr->_421a));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_624n: " + std::to_string(ptr->_624n));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_624m: " + std::to_string(ptr->_624m));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_624s: " + std::to_string(ptr->_624s));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_624v: " + std::to_string(ptr->_624v));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_624d: " + std::to_string(ptr->_624d));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_624a: " + std::to_string(ptr->_624a));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_426n: " + std::to_string(ptr->_426n));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_426m: " + std::to_string(ptr->_426m));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_426s: " + std::to_string(ptr->_426s));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_426v: " + std::to_string(ptr->_426v));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_426d: " + std::to_string(ptr->_426d));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_426a: " + std::to_string(ptr->_426a));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_624684n: " + std::to_string(ptr->_624684n));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_624684m: " + std::to_string(ptr->_624684m));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_624684s: " + std::to_string(ptr->_624684s));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_624684v: " + std::to_string(ptr->_624684v));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_624684d: " + std::to_string(ptr->_624684d));
		game->logger.info(std::string(msgStart) + "Character::_specialInputs::_624684a: " + std::to_string(ptr->_624684a));

		game->logger.info(std::string(msgStart) + "Character::_limit[0]: " + std::to_string(dat->_limit[0]));
		game->logger.info(std::string(msgStart) + "Character::_limit[1]: " + std::to_string(dat->_limit[1]));
		game->logger.info(std::string(msgStart) + "Character::_limit[2]: " + std::to_string(dat->_limit[2]));
		game->logger.info(std::string(msgStart) + "Character::_limit[3]: " + std::to_string(dat->_limit[3]));

		game->logger.info(std::string(msgStart) + "Character::_nbReplayInputs: " + std::to_string(dat->_nbReplayInputs));
		game->logger.info(std::string(msgStart) + "Character::_nbLastInputs: " + std::to_string(dat->_nbLastInputs));
		game->logger.info(std::string(msgStart) + "Character::_nbUsedMoves: " + std::to_string(dat->_nbUsedMoves));

		if (startOffset + len >= dataSize) {
			game->logger.fatal("Invalid input frame");
			return 0;
		}
		return len;
	}

	void Character::_processGroundedEvents()
	{
		if (!this->_isGrounded())
			return;

		auto limited = this->_limit[0] >= 100 || this->_limit[1] >= 100 || this->_limit[2] >= 100 || this->_limit[3] >= 100;

		this->_doubleGravity = false;
		this->_airDashesUsed = 0;
		this->_airMovementUsed = 0;
		if (this->_action >= ACTION_UP_AIR_TECH && this->_action <= ACTION_BACKWARD_AIR_TECH)
			this->_forceStartMove(ACTION_AIR_TECH_LANDING_LAG);
		if (this->_action >= ACTION_AIR_DASH_1 && this->_action <= ACTION_AIR_DASH_9)
			this->_forceStartMove(ACTION_HARD_LAND);
		else if (
			this->_speed.y <= 0 &&
			this->_action != ACTION_BEING_KNOCKED_DOWN &&
			this->_action != ACTION_KNOCKED_DOWN &&
			this->_action != ACTION_NEUTRAL_TECH &&
			this->_action != ACTION_FORWARD_TECH &&
			this->_action != ACTION_BACKWARD_TECH &&
			this->_action != ACTION_FALLING_TECH && (
				this->_action == ACTION_AIR_HIT ||
				this->_action == ACTION_GROUND_SLAM ||
				this->_action == ACTION_WALL_SLAM ||
				limited ||
				this->_hp <= 0
			)
		) {
			if (!this->_restand) {
				this->_blockStun = 0;
				game->soundMgr.play(BASICSOUND_KNOCKDOWN);
				this->_forceStartMove(ACTION_BEING_KNOCKED_DOWN);
			} else {
				this->_forceStartMove(ACTION_GROUND_HIGH_HIT);
				this->_actionBlock = 1;
			}
		} else if ((
			this->_action == ACTION_AIR_NEUTRAL_BLOCK ||
			this->_action == ACTION_AIR_NEUTRAL_PARRY ||
			this->_action == ACTION_AIR_MATTER_PARRY ||
			this->_action == ACTION_AIR_VOID_PARRY ||
			this->_action == ACTION_AIR_SPIRIT_PARRY ||
			this->_action == ACTION_AIR_NEUTRAL_WRONG_BLOCK
		)) {
			this->_blockStun = 0;
			this->_forceStartMove(ACTION_IDLE);
		} else if (
			this->_action != ACTION_NEUTRAL_JUMP &&
			this->_action != ACTION_FORWARD_JUMP &&
			this->_action != ACTION_BACKWARD_JUMP &&
			this->_action != ACTION_NEUTRAL_HIGH_JUMP &&
			this->_action != ACTION_FORWARD_HIGH_JUMP &&
			this->_action != ACTION_BACKWARD_HIGH_JUMP
		)
			this->_jumpsUsed = 0;
	}

	void Character::_renderExtraEffects(const Vector2f &pos) const
	{
		if (this->_neutralEffectTimer)
			this->_renderEffect(pos, this->_neutralEffect);
		else if (this->_spiritEffectTimer)
			this->_renderEffect(pos, this->_spiritEffect);
		else if (this->_matterEffectTimer)
			this->_renderEffect(pos, this->_matterEffect);
		else if (this->_voidEffectTimer)
			this->_renderEffect(pos, this->_voidEffect);
		if (this->_spiritInstallTimer)
			this->_renderInstallEffect(this->_spiritEffect);
		else if (this->_matterInstallTimer)
			this->_renderInstallEffect(this->_matterEffect);
		else if (this->_voidInstallTimer)
			this->_renderInstallEffect(this->_voidEffect);
	}

	void Character::_reduceGuard(unsigned int amount, unsigned regenTime, bool canCrush)
	{
		if (this->_guardCooldown)
			return;
		if (this->_guardBarTmp >= amount) {
			this->_guardBarTmp -= amount;
			return;
		}
		amount -= this->_guardBarTmp;
		this->_guardBarTmp = 0;
		if (regenTime)
			this->_guardRegenCd = regenTime;
		if (amount < this->_guardBar)
			this->_guardBar -= amount;
		else if (canCrush) {
			this->_guardBar = this->_maxGuardBar;
			this->_guardCooldown = this->_maxGuardCooldown;
			game->soundMgr.play(BASICSOUND_GUARD_BREAK);
		} else
			this->_guardBar = 0;
	}
}
