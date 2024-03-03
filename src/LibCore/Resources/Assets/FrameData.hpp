//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_FRAMEDATA_HPP
#define SOFGV_FRAMEDATA_HPP


#include <map>
#include <optional>
#include <string>
#include <vector>
#include "nlohmann/json.hpp"
#include <SFML/Graphics/Rect.hpp>
#include "Data/Vector.hpp"
#include "Data/Color.hpp"

namespace SpiralOfFate
{
	struct Box {
		Vector2i pos;
		Vector2u size;
		operator sf::IntRect() const noexcept;
	};

	union DefensiveFlags {
		unsigned flags;
		struct {
			bool invulnerable : 1;
			bool invulnerableArmor : 1;
			bool superarmor : 1;
			bool grabInvulnerable : 1;
			bool voidBlock : 1;
			bool spiritBlock : 1;
			bool matterBlock : 1;
			bool neutralBlock : 1;
			bool airborne : 1;
			bool canBlock : 1;
			bool highBlock : 1;
			bool lowBlock : 1;
			bool karaCancel : 1;
			bool resetRotation : 1;
			bool counterHit : 1;
			bool flash : 1;
			bool crouch : 1;
			bool projectileInvul : 1;
			bool projectile : 1;
			bool landCancel : 1;
			bool dashCancel : 1;
			bool resetSpeed : 1;
			bool neutralInvul : 1;
			bool matterInvul : 1;
			bool spiritInvul : 1;
			bool voidInvul : 1;
			bool neutralArmor : 1;
			bool matterArmor : 1;
			bool spiritArmor : 1;
			bool voidArmor : 1;
		};
	};

	union OffensiveFlags {
		unsigned long long flags;
		struct {
			bool grab : 1;
			bool airUnblockable : 1;
			bool unblockable : 1;
			bool voidElement : 1;
			bool spiritElement : 1;
			bool matterElement : 1;
			bool lowHit : 1;
			bool highHit : 1;
			bool autoHitPos : 1;
			bool canCounterHit : 1;
			bool hitSwitch : 1;
			bool cancelable : 1;
			bool jab : 1;
			bool resetHits : 1;
			bool opposingPush : 1;
			bool restand : 1;
			bool super : 1;
			bool ultimate : 1;
			bool jumpCancelable : 1;
			bool transformCancelable : 1;
			bool unTransformCancelable : 1;
			bool dashCancelable : 1;
			bool backDashCancelable : 1;
			bool unusedFlag1 : 1;
			bool unusedFlag2 : 1;
			bool unusedFlag3 : 1;
			bool turnAround : 1;
			bool forceTurnAround : 1;
			bool nextBlockOnHit : 1;
			bool nextBlockOnBlock : 1;
			bool hardKnockDown : 1;
			bool groundSlam : 1;
			bool groundSlamCH : 1;
			bool wallSplat : 1;
			bool wallSplatCH : 1;
			bool phantomHit : 1;
		};
	};
	static_assert(sizeof(DefensiveFlags) == sizeof(unsigned), "Too many defensive flags");
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
			Vector2f snap;
			Box textureBounds;
			Box collisionBox;
			bool hasCollisionBox;
			bool hasPriority;
			bool hasGravity;
			bool hasSnap;
			Box boxes[0];
		};
		static_assert(sizeof(Box) == 16, "Box has wrong size");
		static_assert(sizeof(Data) == 736, "Data has wrong size");
#pragma pack(pop)

	public:
		std::pair<std::vector<Color>, std::vector<Color>> __palette;
		std::string __folder;

		std::string spritePath;
		std::string soundPath;
		std::string hitSoundPath;
		unsigned textureHandle = 0;
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
		std::optional<Vector2f> snap;

		FrameData() = default;
		~FrameData();
		FrameData(const FrameData &other);
		FrameData(const nlohmann::json &json, const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette = {{}, {}});
		FrameData &operator=(const FrameData &other);
		void reloadTexture(const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette = {{}, {}});
		void reloadSound();
		void setSlave(bool slave = true);
		nlohmann::json toJson() const;
		unsigned int getBufferSize() const;
		void copyToBuffer(void *data) const;
		void restoreFromBuffer(void *data);

		static size_t printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset);
		static size_t printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize);

		static std::map<unsigned, std::vector<std::vector<FrameData>>> loadFile(const std::string &path, const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette = {{}, {}});
		static std::map<unsigned, std::vector<std::vector<FrameData>>> loadFileJson(const nlohmann::json &path, const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette = {{}, {}});
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
