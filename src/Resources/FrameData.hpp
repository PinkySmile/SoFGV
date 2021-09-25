//
// Created by andgel on 18/09/2021
//

#ifndef BATTLE_FRAMEDATA_HPP
#define BATTLE_FRAMEDATA_HPP


#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <SFML/Graphics/Rect.hpp>
#include "../Data/Vector.hpp"

namespace Battle
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
			bool dashSpeed : 1;
			bool resetRotation : 1;
			bool counterHit : 1;
			bool flash : 1;
			bool crouch : 1;
		};
	};

	union OffensiveFlags {
		unsigned flags;
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
			bool resetSpeed : 1;
			bool restand : 1;
			bool super : 1;
			bool ultimate : 1;
			bool jumpCancelable : 1;
			bool transformCancelable : 1;
			bool unTransformCancelable : 1;
			bool dashCancelable : 1;
			bool backDashCancelable : 1;
			bool voidMana : 1;
			bool spiritMana : 1;
			bool matterMana : 1;
		};
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
		unsigned duration = 1;
		unsigned specialMarker = 0;
		DefensiveFlags dFlag = {0};
		OffensiveFlags oFlag = {0};
		Box *collisionBox = nullptr;
		unsigned blockStun = 0;
		unsigned hitStun = 0;
		float prorate = 0;
		unsigned voidLimit = 0;
		unsigned spiritLimit = 0;
		unsigned matterLimit = 0;
		unsigned pushBack = 0;
		unsigned pushBlock = 0;
		unsigned subObjectSpawn = 0;
		unsigned manaGain = 0;
		unsigned manaCost = 0;
		unsigned hitStop = 0;
		unsigned damage = 0;
		Vector2i speed = {0, 0};
		Vector2i hitSpeed = {0, 0};
		Vector2i counterHitSpeed = {0, 0};

		FrameData() = default;
		~FrameData();
		FrameData(const FrameData &other);
		FrameData(const nlohmann::json &json);
		FrameData &operator=(const FrameData &other);
		void reloadTexture();
		nlohmann::json toJson() const;
		static std::map<unsigned, std::vector<std::vector<FrameData>>> loadFile(const std::string &path);
		static std::map<unsigned, std::vector<std::vector<FrameData>>> loadFileJson(const nlohmann::json &path);
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


#endif //BATTLE_FRAMEDATA_HPP
