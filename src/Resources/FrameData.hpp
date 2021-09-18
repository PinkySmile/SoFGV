//
// Created by andgel on 18/09/2021
//

#ifndef BATTLE_FRAMEDATA_HPP
#define BATTLE_FRAMEDATA_HPP


#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "../Data/Vector.hpp"

namespace Battle
{
	struct Box {
		Vector2i pos;
		Vector2u size;
	};

	struct DefensiveFlags {
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
	};

	struct OffensiveFlags {
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
		bool charaCancel : 1;
	};

	class FrameData {
	public:
		std::string spritePath;
		unsigned textureHandle = 0;
		Vector2i offset = {0, 0};
		Vector2u size = {0, 0};
		Box textureBounds = {{0, 0}, {0, 0}};
		float rotation = 0;
		std::vector<Box> hurtBoxes;
		std::vector<Box> hitBoxes;
		unsigned duration;
		unsigned specialMarker = 0;

		DefensiveFlags dFlag;
		OffensiveFlags oFlag;
		Box *collisionBox;
		unsigned baseBlockStun;
		unsigned baseHitStun;
		float prorate;
		unsigned voidLimit;
		unsigned spiritLimit;
		unsigned matterLimit;
		unsigned opponentPushBack;
		unsigned pushBack;

		FrameData() = default;
		FrameData(const nlohmann::json &json);
		static std::map<unsigned, std::vector<FrameData>> loadFile(const std::string &path);
		static std::map<unsigned, std::vector<FrameData>> loadFileJson(const nlohmann::json &path);
	};
}


#endif //BATTLE_FRAMEDATA_HPP
