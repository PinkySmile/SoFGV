//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_FRAMEDATA_HPP
#define SOFGV_FRAMEDATA_HPP


#include <unordered_map>
#include <optional>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"
#include <SFML/Graphics/Rect.hpp>
#include "Data/Vector.hpp"
#include "Color.hpp"
#include "Screen.hpp"

namespace SpiralOfFate
{
	struct Box {
		Vector2i pos;
		Vector2u size;

		operator IntRect() const noexcept;
		bool operator!=(const Box &other) const;
	};

	union DefensiveFlags {
		unsigned long long flags;
		struct {
			/*  0 */ bool invulnerable : 1;
			/*  1 */ bool invulnerableArmor : 1;
			/*  2 */ bool superarmor : 1;
			/*  3 */ bool grabInvulnerable : 1;
			/*  4 */ bool voidBlock : 1;
			/*  5 */ bool spiritBlock : 1;
			/*  6 */ bool matterBlock : 1;
			/*  7 */ bool neutralBlock : 1;
			/*  8 */ bool airborne : 1;
			/*  9 */ bool canBlock : 1;
			/* 10 */ bool highBlock : 1;
			/* 11 */ bool lowBlock : 1;
			/* 12 */ bool karaCancel : 1;
			/* 13 */ bool resetRotation : 1;
			/* 14 */ bool counterHit : 1;
			/* 15 */ bool flash : 1;
			/* 16 */ bool crouch : 1;
			/* 17 */ bool projectileInvul : 1;
			/* 18 */ bool projectile : 1;
			/* 19 */ bool landCancel : 1;
			/* 20 */ bool dashCancel : 1;
			/* 21 */ bool resetSpeed : 1;
			/* 22 */ bool neutralInvul : 1;
			/* 23 */ bool matterInvul : 1;
			/* 24 */ bool spiritInvul : 1;
			/* 25 */ bool voidInvul : 1;
			/* 26 */ bool neutralArmor : 1;
			/* 27 */ bool matterArmor : 1;
			/* 28 */ bool spiritArmor : 1;
			/* 29 */ bool voidArmor : 1;
			/* 30 */ bool unusedFlag31 : 1;
			/* 31 */ bool unusedFlag32 : 1;
			/* 32 */ bool unusedFlag33 : 1;
			/* 33 */ bool unusedFlag34 : 1;
			/* 34 */ bool unusedFlag35 : 1;
			/* 35 */ bool unusedFlag36 : 1;
			/* 36 */ bool unusedFlag37 : 1;
			/* 37 */ bool unusedFlag38 : 1;
			/* 38 */ bool unusedFlag39 : 1;
			/* 39 */ bool unusedFlag40 : 1;
			/* 40 */ bool unusedFlag41 : 1;
			/* 41 */ bool unusedFlag42 : 1;
			/* 42 */ bool unusedFlag43 : 1;
			/* 43 */ bool unusedFlag44 : 1;
			/* 44 */ bool unusedFlag45 : 1;
			/* 45 */ bool unusedFlag46 : 1;
			/* 46 */ bool unusedFlag47 : 1;
			/* 47 */ bool unusedFlag48 : 1;
			/* 48 */ bool unusedFlag49 : 1;
			/* 49 */ bool unusedFlag50 : 1;
			/* 50 */ bool unusedFlag51 : 1;
			/* 51 */ bool unusedFlag52 : 1;
			/* 52 */ bool unusedFlag53 : 1;
			/* 53 */ bool unusedFlag54 : 1;
			/* 54 */ bool unusedFlag55 : 1;
			/* 55 */ bool unusedFlag56 : 1;
			/* 56 */ bool unusedFlag57 : 1;
			/* 57 */ bool unusedFlag58 : 1;
			/* 58 */ bool unusedFlag59 : 1;
			/* 59 */ bool unusedFlag60 : 1;
			/* 60 */ bool unusedFlag61 : 1;
			/* 61 */ bool unusedFlag62 : 1;
			/* 62 */ bool unusedFlag63 : 1;
			/* 63 */ bool unusedFlag64 : 1;
		};

		bool operator!=(const DefensiveFlags &other) const;
	};

	union OffensiveFlags {
		unsigned long long flags;
		struct {
			/*  0 */ bool grab : 1;
			/*  1 */ bool airUnblockable : 1;
			/*  2 */ bool unblockable : 1;
			/*  3 */ bool voidElement : 1;
			/*  4 */ bool spiritElement : 1;
			/*  5 */ bool matterElement : 1;
			/*  6 */ bool lowHit : 1;
			/*  7 */ bool highHit : 1;
			/*  8 */ bool autoHitPos : 1;
			/*  9 */ bool canCounterHit : 1;
			/* 10 */ bool hitSwitch : 1;
			/* 11 */ bool cancelable : 1;
			/* 12 */ bool jab : 1;
			/* 13 */ bool resetHits : 1;
			/* 14 */ bool opposingPush : 1;
			/* 15 */ bool restand : 1;
			/* 16 */ bool super : 1;
			/* 17 */ bool ultimate : 1;
			/* 18 */ bool jumpCancelable : 1;
			/* 19 */ bool transformCancelable : 1;
			/* 20 */ bool unTransformCancelable : 1;
			/* 21 */ bool dashCancelable : 1;
			/* 22 */ bool backDashCancelable : 1;
			/* 23 */ bool unusedFlag24 : 1;
			/* 24 */ bool unusedFlag25 : 1;
			/* 25 */ bool unusedFlag26 : 1;
			/* 26 */ bool turnAround : 1;
			/* 27 */ bool forceTurnAround : 1;
			/* 28 */ bool nextBlockOnHit : 1;   // TODO:
			/* 29 */ bool nextBlockOnBlock : 1; // TODO:
			/* 30 */ bool hardKnockDown : 1;
			/* 31 */ bool groundSlam : 1;
			/* 32 */ bool groundSlamCH : 1;
			/* 33 */ bool wallSplat : 1;
			/* 34 */ bool wallSplatCH : 1;
			/* 35 */ bool phantomHit : 1;
			/* 36 */ bool unusedFlag37 : 1;
			/* 37 */ bool unusedFlag38 : 1;
			/* 38 */ bool unusedFlag39 : 1;
			/* 39 */ bool unusedFlag40 : 1;
			/* 40 */ bool unusedFlag41 : 1;
			/* 41 */ bool unusedFlag42 : 1;
			/* 42 */ bool unusedFlag43 : 1;
			/* 43 */ bool unusedFlag44 : 1;
			/* 44 */ bool unusedFlag45 : 1;
			/* 45 */ bool unusedFlag46 : 1;
			/* 46 */ bool unusedFlag47 : 1;
			/* 47 */ bool unusedFlag48 : 1;
			/* 48 */ bool unusedFlag49 : 1;
			/* 49 */ bool unusedFlag50 : 1;
			/* 50 */ bool unusedFlag51 : 1;
			/* 51 */ bool unusedFlag52 : 1;
			/* 52 */ bool unusedFlag53 : 1;
			/* 53 */ bool unusedFlag54 : 1;
			/* 54 */ bool unusedFlag55 : 1;
			/* 55 */ bool unusedFlag56 : 1;
			/* 56 */ bool unusedFlag57 : 1;
			/* 57 */ bool unusedFlag58 : 1;
			/* 58 */ bool unusedFlag59 : 1;
			/* 59 */ bool unusedFlag60 : 1;
			/* 60 */ bool unusedFlag61 : 1;
			/* 61 */ bool unusedFlag62 : 1;
			/* 62 */ bool unusedFlag63 : 1;
			/* 63 */ bool unusedFlag64 : 1;
		};

		bool operator!=(const OffensiveFlags &other) const;
	};
	static_assert(sizeof(DefensiveFlags) == sizeof(unsigned long long), "Too many defensive flags");
	static_assert(sizeof(OffensiveFlags) == sizeof(unsigned long long), "Too many offensive flags");

	enum LimitType {
		LIMIT_NEUTRAL,
		LIMIT_VOID,
		LIMIT_MATTER,
		LIMIT_SPIRIT,
	};

	class FrameData {
	private:
		bool _slave = false;

#pragma pack(push, 1)
		struct Data {
			char texturePath[512];
			unsigned wrongBlockStun;
			unsigned blockStun;
			unsigned hitStun;
			unsigned untech;
			unsigned guardDmg;
			unsigned duration;
			unsigned specialMarker;
			unsigned neutralLimit;
			unsigned voidLimit;
			unsigned spiritLimit;
			unsigned matterLimit;
			unsigned manaGain;
			unsigned manaCost;
			unsigned hitPlayerHitStop;
			unsigned hitOpponentHitStop;
			unsigned blockPlayerHitStop;
			unsigned blockOpponentHitStop;
			unsigned damage;
			unsigned chipDamage;
			unsigned priority;
			unsigned hurtBoxesCount;
			unsigned hitBoxesCount;
			unsigned particleGenerator;
			unsigned fadeTime;
			DefensiveFlags dFlag;
			OffensiveFlags oFlag;
			int pushBack;
			int pushBlock;
			int subObjectSpawn;
			float prorate;
			float minProrate;
			float rotation;
			Vector2f scale;
			Vector2i offset;
			Vector2i speed;
			Vector2i hitSpeed;
			Vector2i counterHitSpeed;
			Vector2f gravity;
			Vector2f snapPos;
			float snapRot;
			Box textureBounds;
			Box collisionBox;
			bool hasCollisionBox;
			bool hasPriority;
			bool hasGravity;
			bool hasSnap;
			Box boxes[0];
		};
		static_assert(sizeof(Box) == 16, "Box has wrong size");
		static_assert(sizeof(Data) == 744, "Data has wrong size");
#pragma pack(pop)

	public:
		std::filesystem::path __palette;
		std::filesystem::path __folder;
		bool __requireReload = false;
		const std::array<Color, 256> *__paletteData = nullptr;

		std::string spritePath;
		std::string soundPath;
		std::string hitSoundPath;
		unsigned textureHandle = 0;
		unsigned textureHandleEffects = 0;
		unsigned soundHandle = 0;
		unsigned hitSoundHandle = 0;
		Vector2i offset = {0, 0};
		Vector2f scale = {1, 1};
		Box textureBounds = {{0, 0}, {0, 0}};
		float rotation = 0;
		std::vector<Box> hurtBoxes;
		std::vector<Box> hitBoxes;
		unsigned duration = 1;
		unsigned specialMarker = 0;
		DefensiveFlags dFlag = {0};
		OffensiveFlags oFlag = {0};
		// FIXME: Why isn't this just a std::optional?
		Box *collisionBox = nullptr;
		unsigned wrongBlockStun = 0;
		unsigned blockStun = 0;
		unsigned hitStun = 0;
		unsigned untech = 0;
		unsigned guardDmg = 0;
		float prorate = 0;
		float minProrate = 0;
		unsigned neutralLimit = 0;
		unsigned voidLimit = 0;
		unsigned matterLimit = 0;
		unsigned spiritLimit = 0;
		int pushBack = 0;
		int pushBlock = 0;
		int subObjectSpawn = 0;
		unsigned manaGain = 0;
		unsigned manaCost = 0;
		unsigned hitPlayerHitStop = 0;
		unsigned hitOpponentHitStop = 0;
		unsigned blockPlayerHitStop = 0;
		unsigned blockOpponentHitStop = 0;
		unsigned damage = 0;
		unsigned chipDamage = 0;
		unsigned particleGenerator = 0;
		unsigned fadeTime = 0;
		std::optional<unsigned> priority;
		Vector2f speed = {0, 0};
		Vector2f hitSpeed = {0, 0};
		Vector2f counterHitSpeed = {0, 0};
		std::optional<Vector2f> gravity;
		std::optional<std::pair<Vector2f, float>> snap;

		FrameData() = default;
		~FrameData();
		FrameData(const FrameData &other);
		FrameData(const nlohmann::json &json, const std::filesystem::path &folder, const std::filesystem::path &palette = {});
		FrameData &operator=(const FrameData &other);
		void checkReloadTexture();
		void reloadTexture();
		void reloadSound();
		void setSlave(bool slave = true);
		nlohmann::json toJson() const;
		unsigned int getBufferSize() const;
		void copyToBuffer(void *data) const;
		void restoreFromBuffer(void *data);

		static size_t printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset);
		static size_t printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize);

		static std::unordered_map<unsigned, std::vector<std::vector<FrameData>>> loadFile(const std::filesystem::path &path, const std::filesystem::path &folder, const std::filesystem::path &palette = {});
		static std::unordered_map<unsigned, std::vector<std::vector<FrameData>>> loadFileJson(const nlohmann::json &path, const std::filesystem::path &folder, const std::filesystem::path &palette = {});
	};

	inline void to_json(nlohmann::json &j, const FrameData &data) {
		j = data.toJson();
	}

	inline void to_json(nlohmann::json &j, const Box &box) {
		j = {
			{"left", box.pos.x},
			{"top", box.pos.y},
			{"width", box.size.x},
			{"height", box.size.y},
		};
	}
}


#endif //SOFGV_FRAMEDATA_HPP
