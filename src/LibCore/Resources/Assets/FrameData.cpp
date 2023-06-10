//
// Created by PinkySmile on 18/09/2021.
//

#include <fstream>
#include "FrameData.hpp"
#include "Resources/Game.hpp"
#include "Logger.hpp"

namespace SpiralOfFate
{
	std::map<unsigned int, std::vector<std::vector<FrameData>>> SpiralOfFate::FrameData::loadFile(const std::string &path, const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette)
	{
		std::ifstream stream{path};
		nlohmann::json json;

		game->logger.debug("Loading framedata file " + path);
		my_assert2(!stream.fail(), path + ": " + strerror(errno));
		stream >> json;
		return loadFileJson(json, folder, palette);
	}

	std::map<unsigned, std::vector<std::vector<FrameData>>> FrameData::loadFileJson(const nlohmann::json &json, const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette)
	{
		std::map<unsigned int, std::vector<std::vector<FrameData>>> data;

		game->logger.debug("Loading json");
		my_assert2(json.is_array(), "Invalid json");
		for (auto &val : json) {
			my_assert2(val.is_object(), "Invalid json");
			my_assert2(val.contains("action"), "Invalid json");
			my_assert2(val.contains("framedata"), "Invalid json");

			auto &action = val["action"];
			auto &framedata = val["framedata"];

			my_assert2(action.is_number(), "Invalid json");
			my_assert2(framedata.is_array(), "Invalid json");
			my_assert2(!framedata.empty(), "Invalid json");

			unsigned actionId = action;

			data[actionId].reserve(framedata.size());
			for (auto &block : framedata) {
				my_assert2(block.is_array(), "Invalid json");
				my_assert2(!block.empty(), "Invalid json");
				data[actionId].emplace_back();
				data[actionId].back().reserve(block.size());
				for (auto &frame : block)
					data[actionId].back().emplace_back(frame, folder, palette);
			}
		}
		return data;
	}

	FrameData::FrameData(const nlohmann::json &data, const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette)
	{
		my_assert2(data.is_object(), "Invalid json");
		my_assert2(data.contains("sprite"), "Invalid json");
		my_assert2(data["sprite"].is_string(), "Invalid json");
		this->spritePath = data["sprite"];

		if (data.contains("sound")) {
			my_assert2(data["sound"].is_string(), "Invalid json");
			this->soundPath = data["sound"];
		}
		if (data.contains("hit_sound")) {
			my_assert2(data["hit_sound"].is_string(), "Invalid json");
			this->hitSoundPath = data["hit_sound"];
		}

		if (data.contains("offset")) {
			my_assert2(data["offset"].is_object(), "Invalid json");
			my_assert2(data["offset"].contains("x"), "Invalid json");
			my_assert2(data["offset"].contains("y"), "Invalid json");
			my_assert2(data["offset"]["x"].is_number(), "Invalid json");
			my_assert2(data["offset"]["y"].is_number(), "Invalid json");
			this->offset.x = data["offset"]["x"];
			this->offset.y = data["offset"]["y"];
		}
		if (data.contains("size")) {
			my_assert2(data["size"].is_object(), "Invalid json");
			my_assert2(data["size"].contains("x"), "Invalid json");
			my_assert2(data["size"].contains("y"), "Invalid json");
			my_assert2(data["size"]["x"].is_number(), "Invalid json");
			my_assert2(data["size"]["y"].is_number(), "Invalid json");
			this->size.x = data["size"]["x"];
			this->size.y = data["size"]["y"];
		}
		if (data.contains("gravity")) {
			my_assert2(data["gravity"].is_object(), "Invalid json");
			my_assert2(data["gravity"].contains("x"), "Invalid json");
			my_assert2(data["gravity"].contains("y"), "Invalid json");
			my_assert2(data["gravity"]["x"].is_number(), "Invalid json");
			my_assert2(data["gravity"]["y"].is_number(), "Invalid json");
			this->gravity = Vector2f(data["gravity"]["x"], data["gravity"]["y"]);
		}
		if (data.contains("snap")) {
			my_assert2(data["snap"].is_object(), "Invalid json");
			my_assert2(data["snap"].contains("x"), "Invalid json");
			my_assert2(data["snap"].contains("y"), "Invalid json");
			my_assert2(data["snap"]["x"].is_number(), "Invalid json");
			my_assert2(data["snap"]["y"].is_number(), "Invalid json");
			this->snap = Vector2f(data["snap"]["x"], data["snap"]["y"]);
		}
		if (data.contains("texture_bounds")) {
			my_assert2(data["texture_bounds"].is_object(), "Invalid json");
			my_assert2(data["texture_bounds"].contains("left"), "Invalid json");
			my_assert2(data["texture_bounds"].contains("top"), "Invalid json");
			my_assert2(data["texture_bounds"].contains("width"), "Invalid json");
			my_assert2(data["texture_bounds"].contains("height"), "Invalid json");
			my_assert2(data["texture_bounds"]["left"].is_number(), "Invalid json");
			my_assert2(data["texture_bounds"]["top"].is_number(), "Invalid json");
			my_assert2(data["texture_bounds"]["width"].is_number(), "Invalid json");
			my_assert2(data["texture_bounds"]["height"].is_number(), "Invalid json");
			this->textureBounds.pos.x = data["texture_bounds"]["left"];
			this->textureBounds.pos.y = data["texture_bounds"]["top"];
			this->textureBounds.size.x = data["texture_bounds"]["width"];
			this->textureBounds.size.y = data["texture_bounds"]["height"];
		}
		if (data.contains("rotation")) {
			my_assert2(data["rotation"].is_number(), "Invalid json");
			this->rotation = std::fmod(std::fmod(data["rotation"].get<float>(), 360) + 360, 360) * M_PI / 180;
		}
		if (data.contains("hurt_boxes")) {
			my_assert2(data["hurt_boxes"].is_array(), "Invalid json");
			for (auto &box : data["hurt_boxes"]) {
				my_assert2(box.is_object(), "Invalid json");
				my_assert2(box.contains("left"), "Invalid json");
				my_assert2(box.contains("top"), "Invalid json");
				my_assert2(box.contains("width"), "Invalid json");
				my_assert2(box.contains("height"), "Invalid json");
				my_assert2(box["left"].is_number(), "Invalid json");
				my_assert2(box["top"].is_number(), "Invalid json");
				my_assert2(box["width"].is_number(), "Invalid json");
				my_assert2(box["height"].is_number(), "Invalid json");
				this->hurtBoxes.push_back(Box{{
					box["left"],
					box["top"]
				}, {
					box["width"],
					box["height"]
				}});
			}
		}
		if (data.contains("hit_boxes")) {
			my_assert2(data["hit_boxes"].is_array(), "Invalid json");
			for (auto &box : data["hit_boxes"]) {
				my_assert2(box.is_object(), "Invalid json");
				my_assert2(box.contains("left"), "Invalid json");
				my_assert2(box.contains("top"), "Invalid json");
				my_assert2(box.contains("width"), "Invalid json");
				my_assert2(box.contains("height"), "Invalid json");
				my_assert2(box["left"].is_number(), "Invalid json");
				my_assert2(box["top"].is_number(), "Invalid json");
				my_assert2(box["width"].is_number(), "Invalid json");
				my_assert2(box["height"].is_number(), "Invalid json");
				this->hitBoxes.push_back(Box{{
					box["left"],
					box["top"]
				}, {
					box["width"],
					box["height"]
				}});
			}
		}
		if (data.contains("marker")) {
			my_assert2(data["marker"].is_number(), "Invalid json");
			this->specialMarker = data["marker"];
		}
		if (data.contains("defense_flag")) {
			my_assert2(data["defense_flag"].is_number(), "Invalid json");
			this->dFlag.flags = data["defense_flag"];
		} else
			this->dFlag.flags = 0;
		if (data.contains("offense_flag")) {
			my_assert2(data["offense_flag"].is_number(), "Invalid json");
			this->oFlag.flags = data["offense_flag"];
		} else
			this->oFlag.flags = 0;
		if (data.contains("collision_box")) {
			my_assert2(data["collision_box"].is_object(), "Invalid json");
			my_assert2(data["collision_box"].contains("left"), "Invalid json");
			my_assert2(data["collision_box"].contains("top"), "Invalid json");
			my_assert2(data["collision_box"].contains("width"), "Invalid json");
			my_assert2(data["collision_box"].contains("height"), "Invalid json");
			my_assert2(data["collision_box"]["left"].is_number(), "Invalid json");
			my_assert2(data["collision_box"]["top"].is_number(), "Invalid json");
			my_assert2(data["collision_box"]["width"].is_number(), "Invalid json");
			my_assert2(data["collision_box"]["height"].is_number(), "Invalid json");
			this->collisionBox = new Box{{
				data["collision_box"]["left"],
				data["collision_box"]["top"]
			}, {
				data["collision_box"]["width"],
				data["collision_box"]["height"]
			}};
		}
		if (data.contains("block_stun")) {
			my_assert2(data["block_stun"].is_number(), "Invalid json");
			this->blockStun = data["block_stun"];
		}
		if (data.contains("chip_damage")) {
			my_assert2(data["chip_damage"].is_number(), "Invalid json");
			this->chipDamage = data["chip_damage"];
		}
		if (data.contains("priority")) {
			my_assert2(data["priority"].is_number(), "Invalid json");
			this->priority = data["priority"];
		}
		if (data.contains("hit_stun")) {
			my_assert2(data["hit_stun"].is_number(), "Invalid json");
			this->hitStun = data["hit_stun"];
		}
		if (data.contains("untech")) {
			my_assert2(data["untech"].is_number(), "Invalid json");
			this->untech = data["untech"];
		}
		if (data.contains("guard_damage")) {
			my_assert2(data["guard_damage"].is_number(), "Invalid json");
			this->guardDmg = data["guard_damage"];
		}
		if (data.contains("prorate")) {
			my_assert2(data["prorate"].is_number(), "Invalid json");
			this->prorate = data["prorate"];
		}
		if (data.contains("min_prorate")) {
			my_assert2(data["min_prorate"].is_number(), "Invalid json");
			this->minProrate = data["min_prorate"];
		}
		if (data.contains("neutral_limit")) {
			my_assert2(data["neutral_limit"].is_number(), "Invalid json");
			this->neutralLimit = data["neutral_limit"];
		}
		if (data.contains("void_limit")) {
			my_assert2(data["void_limit"].is_number(), "Invalid json");
			this->voidLimit = data["void_limit"];
		}
		if (data.contains("spirit_limit")) {
			my_assert2(data["spirit_limit"].is_number(), "Invalid json");
			this->spiritLimit = data["spirit_limit"];
		}
		if (data.contains("matter_limit")) {
			my_assert2(data["matter_limit"].is_number(), "Invalid json");
			this->matterLimit = data["matter_limit"];
		}
		if (data.contains("push_back")) {
			my_assert2(data["push_back"].is_number(), "Invalid json");
			this->pushBack = data["push_back"];
		}
		if (data.contains("push_block")) {
			my_assert2(data["push_block"].is_number(), "Invalid json");
			this->pushBlock = data["push_block"];
		}
		if (data.contains("duration")) {
			my_assert2(data["duration"].is_number(), "Invalid json");
			this->duration = data["duration"];
		}
		if (data.contains("subobject")) {
			my_assert2(data["subobject"].is_number(), "Invalid json");
			this->subObjectSpawn = data["subobject"];
		}
		if (data.contains("mana_gain")) {
			my_assert2(data["mana_gain"].is_number(), "Invalid json");
			this->manaGain = data["mana_gain"];
		}
		if (data.contains("mana_cost")) {
			my_assert2(data["mana_cost"].is_number(), "Invalid json");
			this->manaCost = data["mana_cost"];
		}
		if (data.contains("hit_player_hit_stop")) {
			my_assert2(data["hit_player_hit_stop"].is_number(), "Invalid json");
			this->hitPlayerHitStop = data["hit_player_hit_stop"];
		}
		if (data.contains("hit_opponent_hit_stop")) {
			my_assert2(data["hit_opponent_hit_stop"].is_number(), "Invalid json");
			this->hitOpponentHitStop = data["hit_opponent_hit_stop"];
		}
		if (data.contains("block_player_hit_stop")) {
			my_assert2(data["block_player_hit_stop"].is_number(), "Invalid json");
			this->blockPlayerHitStop = data["block_player_hit_stop"];
		}
		if (data.contains("block_opponent_hit_stop")) {
			my_assert2(data["block_opponent_hit_stop"].is_number(), "Invalid json");
			this->blockOpponentHitStop = data["block_opponent_hit_stop"];
		}
		if (data.contains("damage")) {
			my_assert2(data["damage"].is_number(), "Invalid json");
			this->damage = data["damage"];
		}
		if (data.contains("speed")) {
			my_assert2(data["speed"].is_object(), "Invalid json");
			my_assert2(data["speed"].contains("x"), "Invalid json");
			my_assert2(data["speed"].contains("y"), "Invalid json");
			my_assert2(data["speed"]["x"].is_number(), "Invalid json");
			my_assert2(data["speed"]["y"].is_number(), "Invalid json");
			this->speed.x = data["speed"]["x"];
			this->speed.y = data["speed"]["y"];
		}
		if (data.contains("hit_speed")) {
			my_assert2(data["hit_speed"].is_object(), "Invalid json");
			my_assert2(data["hit_speed"].contains("x"), "Invalid json");
			my_assert2(data["hit_speed"].contains("y"), "Invalid json");
			my_assert2(data["hit_speed"]["x"].is_number(), "Invalid json");
			my_assert2(data["hit_speed"]["y"].is_number(), "Invalid json");
			this->hitSpeed.x = data["hit_speed"]["x"];
			this->hitSpeed.y = data["hit_speed"]["y"];
		}
		if (data.contains("counter_hit_speed")) {
			my_assert2(data["counter_hit_speed"].is_object(), "Invalid json");
			my_assert2(data["counter_hit_speed"].contains("x"), "Invalid json");
			my_assert2(data["counter_hit_speed"].contains("y"), "Invalid json");
			my_assert2(data["counter_hit_speed"]["x"].is_number(), "Invalid json");
			my_assert2(data["counter_hit_speed"]["y"].is_number(), "Invalid json");
			this->counterHitSpeed.x = data["counter_hit_speed"]["x"];
			this->counterHitSpeed.y = data["counter_hit_speed"]["y"];
		}

		Vector2u textureSize;

		this->textureHandle = game->textureMgr.load(folder + "/" + this->spritePath, palette, &textureSize);
		if (!this->soundPath.empty()) {
			if (this->soundPath[0] != 'a') {
				this->soundHandle = std::stoul(this->soundPath);
				game->soundMgr.addRef(this->soundHandle);
			} else
				this->soundHandle = game->soundMgr.load(this->soundPath);
		}
		if (!this->hitSoundPath.empty()){
			if (this->hitSoundPath[0] >= '0' && this->hitSoundPath[0] <= '9') {
				this->hitSoundHandle = std::stoul(this->hitSoundPath);
				game->soundMgr.addRef(this->hitSoundHandle);
			} else
				this->hitSoundHandle = game->soundMgr.load(this->hitSoundPath);
		}
		if (!this->textureBounds.size.x)
			this->textureBounds.size.x = textureSize.x;
		if (!this->textureBounds.size.y)
			this->textureBounds.size.y = textureSize.y;
		if (!this->size.x)
			this->size.x = this->textureBounds.size.x;
		if (!this->size.y)
			this->size.y = this->textureBounds.size.y;
	}

	FrameData::~FrameData()
	{
		if (!this->_slave) {
			game->textureMgr.remove(this->textureHandle);
			game->soundMgr.remove(this->soundHandle);
			game->soundMgr.remove(this->hitSoundHandle);
			delete this->collisionBox;
		}
	}

	FrameData::FrameData(const FrameData &other)
	{
		this->chipDamage = other.chipDamage;
		this->priority = other.priority;
		this->spritePath = other.spritePath;
		this->textureHandle = other.textureHandle;
		this->soundPath = other.soundPath;
		this->soundHandle = other.soundHandle;
		this->hitSoundPath = other.hitSoundPath;
		this->hitSoundHandle = other.hitSoundHandle;
		this->offset = other.offset;
		this->size = other.size;
		this->textureBounds = other.textureBounds;
		this->rotation = other.rotation;
		this->hurtBoxes = other.hurtBoxes;
		this->hitBoxes = other.hitBoxes;
		this->duration = other.duration;
		this->specialMarker = other.specialMarker;
		this->dFlag = other.dFlag;
		this->oFlag = other.oFlag;
		this->blockStun = other.blockStun;
		this->hitStun = other.hitStun;
		this->untech = other.untech;
		this->guardDmg = other.guardDmg;
		this->prorate = other.prorate;
		this->minProrate = other.minProrate;
		this->neutralLimit = other.neutralLimit;
		this->voidLimit = other.voidLimit;
		this->spiritLimit = other.spiritLimit;
		this->matterLimit = other.matterLimit;
		this->pushBack = other.pushBack;
		this->pushBlock = other.pushBlock;
		this->subObjectSpawn = other.subObjectSpawn;
		this->manaGain = other.manaGain;
		this->manaCost = other.manaCost;
		this->damage = other.damage;
		this->hitPlayerHitStop = other.hitPlayerHitStop;
		this->hitOpponentHitStop = other.hitOpponentHitStop;
		this->blockPlayerHitStop = other.blockPlayerHitStop;
		this->blockOpponentHitStop = other.blockOpponentHitStop;
		this->hitSpeed = other.hitSpeed;
		this->speed = other.speed;
		this->counterHitSpeed = other.counterHitSpeed;
		this->gravity = other.gravity;
		this->snap = other.snap;
		if (!this->_slave) {
			game->textureMgr.addRef(this->textureHandle);
			game->soundMgr.addRef(this->soundHandle);
			game->soundMgr.addRef(this->hitSoundHandle);
			delete this->collisionBox;
			this->collisionBox = nullptr;
			if (other.collisionBox) {
				this->collisionBox = new Box{
					other.collisionBox->pos,
					other.collisionBox->size
				};
			}
		} else
			this->collisionBox = other.collisionBox;
	}

	FrameData &FrameData::operator=(const FrameData &other)
	{
		if (!this->_slave) {
			game->textureMgr.remove(this->textureHandle);
			game->soundMgr.remove(this->soundHandle);
			game->soundMgr.remove(this->hitSoundHandle);
		}
		this->chipDamage = other.chipDamage;
		this->priority = other.priority;
		this->spritePath = other.spritePath;
		this->textureHandle = other.textureHandle;
		this->soundPath = other.soundPath;
		this->soundHandle = other.soundHandle;
		this->hitSoundPath = other.hitSoundPath;
		this->hitSoundHandle = other.hitSoundHandle;
		this->offset = other.offset;
		this->size = other.size;
		this->textureBounds = other.textureBounds;
		this->rotation = other.rotation;
		this->hurtBoxes = other.hurtBoxes;
		this->hitBoxes = other.hitBoxes;
		this->duration = other.duration;
		this->specialMarker = other.specialMarker;
		this->dFlag = other.dFlag;
		this->oFlag = other.oFlag;
		this->blockStun = other.blockStun;
		this->hitStun = other.hitStun;
		this->untech = other.untech;
		this->guardDmg = other.guardDmg;
		this->prorate = other.prorate;
		this->minProrate = other.minProrate;
		this->neutralLimit = other.neutralLimit;
		this->voidLimit = other.voidLimit;
		this->spiritLimit = other.spiritLimit;
		this->matterLimit = other.matterLimit;
		this->pushBack = other.pushBack;
		this->pushBlock = other.pushBlock;
		this->subObjectSpawn = other.subObjectSpawn;
		this->manaGain = other.manaGain;
		this->manaCost = other.manaCost;
		this->damage = other.damage;
		this->hitPlayerHitStop = other.hitPlayerHitStop;
		this->hitOpponentHitStop = other.hitOpponentHitStop;
		this->blockPlayerHitStop = other.blockPlayerHitStop;
		this->blockOpponentHitStop = other.blockOpponentHitStop;
		this->hitSpeed = other.hitSpeed;
		this->speed = other.speed;
		this->counterHitSpeed = other.counterHitSpeed;
		this->gravity = other.gravity;
		this->snap = other.snap;
		if (!this->_slave) {
			game->textureMgr.addRef(this->textureHandle);
			game->soundMgr.addRef(this->soundHandle);
			game->soundMgr.addRef(this->hitSoundHandle);
			delete this->collisionBox;
			this->collisionBox = nullptr;
			if (other.collisionBox)
				this->collisionBox = new Box{*other.collisionBox};
		} else
			this->collisionBox = other.collisionBox;
		return *this;
	}

	void FrameData::reloadTexture(const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette)
	{
		my_assert(!this->_slave);
		game->textureMgr.remove(this->textureHandle);
		this->textureHandle = game->textureMgr.load(folder + "/" + this->spritePath, palette);
	}

	void FrameData::reloadSound()
	{
		my_assert(!this->_slave);
		game->soundMgr.remove(this->soundHandle);
		game->soundMgr.remove(this->hitSoundHandle);
		this->soundHandle = 0;
		this->hitSoundHandle = 0;
		if (!this->soundPath.empty()) {
			if (this->soundPath[0] != 'a') {
				this->soundHandle = std::stoul(this->soundPath);
				game->soundMgr.addRef(this->soundHandle);
			} else
				this->soundHandle = game->soundMgr.load(this->soundPath);
		}
		if (!this->hitSoundPath.empty()){
			if (this->hitSoundPath[0] != 'a') {
				this->hitSoundHandle = std::stoul(this->hitSoundPath);
				game->soundMgr.addRef(this->hitSoundHandle);
			} else
				this->hitSoundHandle = game->soundMgr.load(this->hitSoundPath);
		}
	}

	nlohmann::json FrameData::toJson() const
	{
		nlohmann::json result = nlohmann::json::object();

		result["sprite"] = this->spritePath;
		if (!this->soundPath.empty())
			result["sound"] = this->soundPath;
		if (!this->hitSoundPath.empty())
			result["hit_sound"] = this->hitSoundPath;
		if (this->offset.x || this->offset.y)
			result["offset"] = {
				{"x", this->offset.x},
				{"y", this->offset.y}
			};
		if (this->speed.x || this->speed.y)
			result["speed"] = {
				{"x", this->speed.x},
				{"y", this->speed.y}
			};
		if (this->gravity)
			result["gravity"] = {
				{"x", this->gravity->x},
				{"y", this->gravity->y}
			};
		if (this->snap)
			result["snap"] = {
				{"x", this->snap->x},
				{"y", this->snap->y}
			};
		if (this->size != game->textureMgr.getTextureSize(this->textureHandle))
			result["size"] = {
				{"x", this->size.x},
				{"y", this->size.y}
			};
		if (this->textureBounds.pos != Vector2i{0, 0} || this->textureBounds.size != game->textureMgr.getTextureSize(this->textureHandle))
			result["texture_bounds"] = {
				{"left", this->textureBounds.pos.x},
				{"top", this->textureBounds.pos.y},
				{"width", this->textureBounds.size.x},
				{"height", this->textureBounds.size.y}
			};
		if (this->rotation != 0)
			result["rotation"] = this->rotation * 180 / M_PI;
		if (!this->hurtBoxes.empty())
			result["hurt_boxes"] = this->hurtBoxes;
		if (!this->hitBoxes.empty())
			result["hit_boxes"] = this->hitBoxes;
		if (this->specialMarker)
			result["marker"] = this->specialMarker;
		if (this->damage)
			result["damage"] = this->damage;
		if (this->chipDamage)
			result["chip_damage"] = this->chipDamage;
		if (this->priority)
			result["priority"] = *this->priority;
		if (this->dFlag.flags)
			result["defense_flag"] = this->dFlag.flags;
		if (this->oFlag.flags)
			result["offense_flag"] = this->oFlag.flags;
		if (this->collisionBox)
			result["collision_box"] = *this->collisionBox;
		if (this->blockStun)
			result["block_stun"] = this->blockStun;
		if (this->hitStun)
			result["hit_stun"] = this->hitStun;
		if (this->untech)
			result["untech"] = this->untech;
		if (this->guardDmg)
			result["guard_damage"] = this->guardDmg;
		if (this->prorate != 0)
			result["prorate"] = this->prorate;
		if (this->minProrate != 0)
			result["min_prorate"] = this->minProrate;
		if (this->neutralLimit)
			result["neutral_limit"] = this->neutralLimit;
		if (this->voidLimit)
			result["void_limit"] = this->voidLimit;
		if (this->spiritLimit)
			result["spirit_limit"] = this->spiritLimit;
		if (this->matterLimit)
			result["matter_limit"] = this->matterLimit;
		if (this->pushBack)
			result["push_back"] = this->pushBack;
		if (this->pushBlock)
			result["push_block"] = this->pushBlock;
		if (this->duration > 1)
			result["duration"] = this->duration;
		if (this->subObjectSpawn)
			result["subobject"] = this->subObjectSpawn;
		if (this->manaGain)
			result["mana_gain"] = this->manaGain;
		if (this->manaCost)
			result["mana_cost"] = this->manaCost;
		if (this->hitPlayerHitStop)
			result["hit_player_hit_stop"] = this->hitPlayerHitStop;
		if (this->hitOpponentHitStop)
			result["hit_opponent_hit_stop"] = this->hitOpponentHitStop;
		if (this->blockPlayerHitStop)
			result["block_player_hit_stop"] = this->blockPlayerHitStop;
		if (this->blockOpponentHitStop)
			result["block_opponent_hit_stop"] = this->blockOpponentHitStop;
		if (this->hitSpeed.x || this->hitSpeed.y)
			result["hit_speed"] = {
				{"x", this->hitSpeed.x},
				{"y", this->hitSpeed.y}
			};
		if (this->counterHitSpeed.x || this->counterHitSpeed.y)
			result["counter_hit_speed"] = {
				{"x", this->counterHitSpeed.x},
				{"y", this->counterHitSpeed.y}
			};
		return result;
	}

	void FrameData::setSlave(bool slave)
	{
		if (slave == this->_slave)
			return;
		if (slave) {
			game->textureMgr.remove(this->textureHandle);
			game->soundMgr.remove(this->soundHandle);
			game->soundMgr.remove(this->hitSoundHandle);
			delete this->collisionBox;
			this->collisionBox = nullptr;
		} else {
			game->textureMgr.addRef(this->textureHandle);
			game->soundMgr.addRef(this->soundHandle);
			game->soundMgr.addRef(this->hitSoundHandle);
			if (this->collisionBox)
				this->collisionBox = new Box{*this->collisionBox};
		}
		this->_slave = slave;
	}

	unsigned int FrameData::getBufferSize() const
	{
		return sizeof(Data) + sizeof(Box) * (this->hurtBoxes.size() + this->hitBoxes.size());
	}

	void FrameData::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>(data);
		unsigned i = 0;

		memset(dat, 0, sizeof(*dat));
		game->logger.verbose("Saving FrameData (Data size: " + std::to_string(this->getBufferSize()) + ") @" + std::to_string((uintptr_t)dat));
		dat->textureHandle = this->textureHandle;
		dat->soundHandle = this->soundHandle;
		dat->hitSoundHandle = this->hitSoundHandle;
		dat->blockStun = this->blockStun;
		dat->hitStun = this->hitStun;
		dat->untech = this->untech;
		dat->guardDmg = this->guardDmg;
		dat->duration = this->duration;
		dat->specialMarker = this->specialMarker;
		dat->neutralLimit = this->neutralLimit;
		dat->voidLimit = this->voidLimit;
		dat->spiritLimit = this->spiritLimit;
		dat->matterLimit = this->matterLimit;
		dat->manaGain = this->manaGain;
		dat->manaCost = this->manaCost;
		dat->hitPlayerHitStop = this->hitPlayerHitStop;
		dat->hitOpponentHitStop = this->hitOpponentHitStop;
		dat->blockPlayerHitStop = this->blockPlayerHitStop;
		dat->blockOpponentHitStop = this->blockOpponentHitStop;
		dat->damage = this->damage;
		dat->chipDamage = this->chipDamage;
		dat->dFlag = this->dFlag;
		dat->oFlag = this->oFlag;
		dat->pushBack = this->pushBack;
		dat->pushBlock = this->pushBlock;
		dat->subObjectSpawn = this->subObjectSpawn;
		dat->prorate = this->prorate;
		dat->minProrate = this->minProrate;
		dat->rotation = this->rotation;
		dat->size = this->size;
		dat->offset = this->offset;
		dat->speed = this->speed;
		dat->hitSpeed = this->hitSpeed;
		dat->counterHitSpeed = this->counterHitSpeed;
		dat->textureBounds = this->textureBounds;
		dat->hasCollisionBox = this->collisionBox != nullptr;
		dat->hasPriority = this->priority.has_value();
		dat->hasGravity = this->gravity.has_value();
		dat->hasSnap = this->snap.has_value();
		if (this->collisionBox)
			dat->collisionBox = *this->collisionBox;
		if (this->gravity)
			dat->gravity = *this->gravity;
		if (this->snap)
			dat->snap = *this->snap;
		if (this->priority)
			dat->priority = *this->priority;
		dat->hurtBoxesCount = this->hurtBoxes.size();
		dat->hitBoxesCount = this->hitBoxes.size();
		for (auto &box : this->hurtBoxes)
			dat->boxes[i++] = box;
		for (auto &box : this->hitBoxes)
			dat->boxes[i++] = box;
	}

	void FrameData::restoreFromBuffer(void *data)
	{
		auto dat = reinterpret_cast<Data *>(data);
		unsigned i = 0;

		this->textureHandle = dat->textureHandle;
		this->soundHandle = dat->soundHandle;
		this->hitSoundHandle = dat->hitSoundHandle;
		this->blockStun = dat->blockStun;
		this->hitStun = dat->hitStun;
		this->untech = dat->untech;
		this->guardDmg = dat->guardDmg;
		this->duration = dat->duration;
		this->specialMarker = dat->specialMarker;
		this->neutralLimit = dat->neutralLimit;
		this->voidLimit = dat->voidLimit;
		this->spiritLimit = dat->spiritLimit;
		this->matterLimit = dat->matterLimit;
		this->manaGain = dat->manaGain;
		this->manaCost = dat->manaCost;
		this->hitPlayerHitStop = dat->hitPlayerHitStop;
		this->hitOpponentHitStop = dat->hitOpponentHitStop;
		this->blockPlayerHitStop = dat->blockPlayerHitStop;
		this->blockOpponentHitStop = dat->blockOpponentHitStop;
		this->damage = dat->damage;
		this->chipDamage = dat->chipDamage;
		this->dFlag = dat->dFlag;
		this->oFlag = dat->oFlag;
		this->pushBack = dat->pushBack;
		this->pushBlock = dat->pushBlock;
		this->subObjectSpawn = dat->subObjectSpawn;
		this->prorate = dat->prorate;
		this->minProrate = dat->minProrate;
		this->rotation = dat->rotation;
		this->size = dat->size;
		this->offset = dat->offset;
		this->speed = dat->speed;
		this->hitSpeed = dat->hitSpeed;
		this->counterHitSpeed = dat->counterHitSpeed;
		this->textureBounds = dat->textureBounds;
		if (dat->hasCollisionBox)
			this->collisionBox = new Box(dat->collisionBox);
		else
			this->collisionBox = nullptr;
		if (dat->hasPriority)
			this->gravity = dat->gravity;
		else
			this->gravity.reset();
		if (dat->hasGravity)
			this->snap = dat->snap;
		else
			this->snap.reset();
		if (dat->hasSnap)
			this->priority = dat->priority;
		else
			this->priority.reset();
		this->hurtBoxes.clear();
		this->hitBoxes.clear();
		this->hurtBoxes.reserve(dat->hurtBoxesCount);
		this->hitBoxes.reserve(dat->hitBoxesCount);
		while (i < dat->hurtBoxesCount)
			this->hurtBoxes.emplace_back(dat->boxes[i++]);
		while (i < dat->hurtBoxesCount + dat->hitBoxesCount)
			this->hitBoxes.emplace_back(dat->boxes[i++]);
		game->logger.verbose("Restored FrameData @" + std::to_string((uintptr_t)dat));
	}

	size_t FrameData::printDifference(const char *msgStart, void *data1, void *data2)
	{
		auto dat1 = reinterpret_cast<Data *>(data1);
		auto dat2 = reinterpret_cast<Data *>(data2);
		unsigned i = 0;

		if (dat1->textureHandle != dat2->textureHandle)
			game->logger.fatal(std::string(msgStart) + "FrameData::textureHandle: " + std::to_string(dat1->textureHandle) + " vs " + std::to_string(dat2->textureHandle));
		if (dat1->soundHandle != dat2->soundHandle)
			game->logger.fatal(std::string(msgStart) + "FrameData::soundHandle: " + std::to_string(dat1->soundHandle) + " vs " + std::to_string(dat2->soundHandle));
		if (dat1->hitSoundHandle != dat2->hitSoundHandle)
			game->logger.fatal(std::string(msgStart) + "FrameData::hitSoundHandle: " + std::to_string(dat1->hitSoundHandle) + " vs " + std::to_string(dat2->hitSoundHandle));
		if (dat1->blockStun != dat2->blockStun)
			game->logger.fatal(std::string(msgStart) + "FrameData::blockStun: " + std::to_string(dat1->blockStun) + " vs " + std::to_string(dat2->blockStun));
		if (dat1->hitStun != dat2->hitStun)
			game->logger.fatal(std::string(msgStart) + "FrameData::hitStun: " + std::to_string(dat1->hitStun) + " vs " + std::to_string(dat2->hitStun));
		if (dat1->untech != dat2->untech)
			game->logger.fatal(std::string(msgStart) + "FrameData::untech: " + std::to_string(dat1->untech) + " vs " + std::to_string(dat2->untech));
		if (dat1->guardDmg != dat2->guardDmg)
			game->logger.fatal(std::string(msgStart) + "FrameData::guardDmg: " + std::to_string(dat1->guardDmg) + " vs " + std::to_string(dat2->guardDmg));
		if (dat1->duration != dat2->duration)
			game->logger.fatal(std::string(msgStart) + "FrameData::duration: " + std::to_string(dat1->duration) + " vs " + std::to_string(dat2->duration));
		if (dat1->specialMarker != dat2->specialMarker)
			game->logger.fatal(std::string(msgStart) + "FrameData::specialMarker: " + std::to_string(dat1->specialMarker) + " vs " + std::to_string(dat2->specialMarker));
		if (dat1->neutralLimit != dat2->neutralLimit)
			game->logger.fatal(std::string(msgStart) + "FrameData::neutralLimit: " + std::to_string(dat1->neutralLimit) + " vs " + std::to_string(dat2->neutralLimit));
		if (dat1->voidLimit != dat2->voidLimit)
			game->logger.fatal(std::string(msgStart) + "FrameData::voidLimit: " + std::to_string(dat1->voidLimit) + " vs " + std::to_string(dat2->voidLimit));
		if (dat1->spiritLimit != dat2->spiritLimit)
			game->logger.fatal(std::string(msgStart) + "FrameData::spiritLimit: " + std::to_string(dat1->spiritLimit) + " vs " + std::to_string(dat2->spiritLimit));
		if (dat1->matterLimit != dat2->matterLimit)
			game->logger.fatal(std::string(msgStart) + "FrameData::matterLimit: " + std::to_string(dat1->matterLimit) + " vs " + std::to_string(dat2->matterLimit));
		if (dat1->manaGain != dat2->manaGain)
			game->logger.fatal(std::string(msgStart) + "FrameData::manaGain: " + std::to_string(dat1->manaGain) + " vs " + std::to_string(dat2->manaGain));
		if (dat1->manaCost != dat2->manaCost)
			game->logger.fatal(std::string(msgStart) + "FrameData::manaCost: " + std::to_string(dat1->manaCost) + " vs " + std::to_string(dat2->manaCost));
		if (dat1->hitPlayerHitStop != dat2->hitPlayerHitStop)
			game->logger.fatal(std::string(msgStart) + "FrameData::hitPlayerHitStop: " + std::to_string(dat1->hitPlayerHitStop) + " vs " + std::to_string(dat2->hitPlayerHitStop));
		if (dat1->hitOpponentHitStop != dat2->hitOpponentHitStop)
			game->logger.fatal(std::string(msgStart) + "FrameData::hitOpponentHitStop: " + std::to_string(dat1->hitOpponentHitStop) + " vs " + std::to_string(dat2->hitOpponentHitStop));
		if (dat1->blockPlayerHitStop != dat2->blockPlayerHitStop)
			game->logger.fatal(std::string(msgStart) + "FrameData::blockPlayerHitStop: " + std::to_string(dat1->blockPlayerHitStop) + " vs " + std::to_string(dat2->blockPlayerHitStop));
		if (dat1->blockOpponentHitStop != dat2->blockOpponentHitStop)
			game->logger.fatal(std::string(msgStart) + "FrameData::blockOpponentHitStop: " + std::to_string(dat1->blockOpponentHitStop) + " vs " + std::to_string(dat2->blockOpponentHitStop));
		if (dat1->damage != dat2->damage)
			game->logger.fatal(std::string(msgStart) + "FrameData::damage: " + std::to_string(dat1->damage) + " vs " + std::to_string(dat2->damage));
		if (dat1->chipDamage != dat2->chipDamage)
			game->logger.fatal(std::string(msgStart) + "FrameData::chipDamage: " + std::to_string(dat1->chipDamage) + " vs " + std::to_string(dat2->chipDamage));
		if (dat1->hasPriority && dat2->hasPriority && dat1->priority != dat2->priority)
			game->logger.fatal(std::string(msgStart) + "FrameData::priority: " + std::to_string(dat1->priority) + " vs " + std::to_string(dat2->priority));
		if (dat1->hurtBoxesCount != dat2->hurtBoxesCount)
			game->logger.fatal(std::string(msgStart) + "FrameData::hurtBoxesCount: " + std::to_string(dat1->hurtBoxesCount) + " vs " + std::to_string(dat2->hurtBoxesCount));
		if (dat1->hitBoxesCount != dat2->hitBoxesCount)
			game->logger.fatal(std::string(msgStart) + "FrameData::hitBoxesCount: " + std::to_string(dat1->hitBoxesCount) + " vs " + std::to_string(dat2->hitBoxesCount));
		if (dat1->dFlag.flags != dat2->dFlag.flags)
			game->logger.fatal(std::string(msgStart) + "FrameData::dFlag: " + std::to_string(dat1->dFlag.flags) + " vs " + std::to_string(dat2->dFlag.flags));
		if (dat1->oFlag.flags != dat2->oFlag.flags)
			game->logger.fatal(std::string(msgStart) + "FrameData::oFlag: " + std::to_string(dat1->oFlag.flags) + " vs " + std::to_string(dat2->oFlag.flags));
		if (dat1->pushBack != dat2->pushBack)
			game->logger.fatal(std::string(msgStart) + "FrameData::pushBack: " + std::to_string(dat1->pushBack) + " vs " + std::to_string(dat2->pushBack));
		if (dat1->pushBlock != dat2->pushBlock)
			game->logger.fatal(std::string(msgStart) + "FrameData::pushBlock: " + std::to_string(dat1->pushBlock) + " vs " + std::to_string(dat2->pushBlock));
		if (dat1->subObjectSpawn != dat2->subObjectSpawn)
			game->logger.fatal(std::string(msgStart) + "FrameData::subObjectSpawn: " + std::to_string(dat1->subObjectSpawn) + " vs " + std::to_string(dat2->subObjectSpawn));
		if (dat1->prorate != dat2->prorate)
			game->logger.fatal(std::string(msgStart) + "FrameData::prorate: " + std::to_string(dat1->prorate) + " vs " + std::to_string(dat2->prorate));
		if (dat1->minProrate != dat2->minProrate)
			game->logger.fatal(std::string(msgStart) + "FrameData::minProrate: " + std::to_string(dat1->minProrate) + " vs " + std::to_string(dat2->minProrate));
		if (dat1->rotation != dat2->rotation)
			game->logger.fatal(std::string(msgStart) + "FrameData::rotation: " + std::to_string(dat1->rotation) + " vs " + std::to_string(dat2->rotation));
		if (dat1->hasCollisionBox != dat2->hasCollisionBox)
			game->logger.fatal(std::string(msgStart) + "FrameData::hasCollisionBox: " + std::to_string(dat1->hasCollisionBox) + " vs " + std::to_string(dat2->hasCollisionBox));
		if (dat1->hasPriority != dat2->hasPriority)
			game->logger.fatal(std::string(msgStart) + "FrameData::hasPriority: " + std::to_string(dat1->hasPriority) + " vs " + std::to_string(dat2->hasPriority));
		if (dat1->hasGravity != dat2->hasGravity)
			game->logger.fatal(std::string(msgStart) + "FrameData::hasGravity: " + std::to_string(dat1->hasGravity) + " vs " + std::to_string(dat2->hasGravity));
		if (dat1->hasSnap != dat2->hasSnap)
			game->logger.fatal(std::string(msgStart) + "FrameData::hasSnap: " + std::to_string(dat1->hasSnap) + " vs " + std::to_string(dat2->hasSnap));
		if (dat1->textureBounds.pos != dat2->textureBounds.pos)
			game->logger.fatal(std::string(msgStart) + "FrameData::textureBounds::pos: (" + std::to_string(dat1->textureBounds.pos.x) + ", " + std::to_string(dat1->textureBounds.pos.y) + ") vs (" + std::to_string(dat2->textureBounds.pos.x) + ", " + std::to_string(dat2->textureBounds.pos.y) + ")");
		if (dat1->textureBounds.size != dat2->textureBounds.size)
			game->logger.fatal(std::string(msgStart) + "FrameData::textureBounds::size: (" + std::to_string(dat1->textureBounds.size.x) + ", " + std::to_string(dat1->textureBounds.size.y) + ") vs (" + std::to_string(dat2->textureBounds.size.x) + ", " + std::to_string(dat2->textureBounds.size.y) + ")");
		if (dat1->hasCollisionBox && dat2->hasCollisionBox && dat1->collisionBox.pos != dat2->collisionBox.pos)
			game->logger.fatal(std::string(msgStart) + "FrameData::collisionBox::pos: (" + std::to_string(dat1->collisionBox.pos.x) + ", " + std::to_string(dat1->collisionBox.pos.y) + ") vs (" + std::to_string(dat2->collisionBox.pos.x) + ", " + std::to_string(dat2->collisionBox.pos.y) + ")");
		if (dat1->hasCollisionBox && dat2->hasCollisionBox && dat1->collisionBox.size != dat2->collisionBox.size)
			game->logger.fatal(std::string(msgStart) + "FrameData::collisionBox::size: (" + std::to_string(dat1->collisionBox.size.x) + ", " + std::to_string(dat1->collisionBox.size.y) + ") vs (" + std::to_string(dat2->collisionBox.size.x) + ", " + std::to_string(dat2->collisionBox.size.y) + ")");
		if (dat1->size != dat2->size)
			game->logger.fatal(std::string(msgStart) + "FrameData::size: (" + std::to_string(dat1->size.x) + ", " + std::to_string(dat1->size.y) + ") vs (" + std::to_string(dat2->size.x) + ", " + std::to_string(dat2->size.y) + ")");
		if (dat1->offset != dat2->offset)
			game->logger.fatal(std::string(msgStart) + "FrameData::offset: (" + std::to_string(dat1->offset.x) + ", " + std::to_string(dat1->offset.y) + ") vs (" + std::to_string(dat2->offset.x) + ", " + std::to_string(dat2->offset.y) + ")");
		if (dat1->speed != dat2->speed)
			game->logger.fatal(std::string(msgStart) + "FrameData::speed: (" + std::to_string(dat1->speed.x) + ", " + std::to_string(dat1->speed.y) + ") vs (" + std::to_string(dat2->speed.x) + ", " + std::to_string(dat2->speed.y) + ")");
		if (dat1->hitSpeed != dat2->hitSpeed)
			game->logger.fatal(std::string(msgStart) + "FrameData::hitSpeed: (" + std::to_string(dat1->hitSpeed.x) + ", " + std::to_string(dat1->hitSpeed.y) + ") vs (" + std::to_string(dat2->hitSpeed.x) + ", " + std::to_string(dat2->hitSpeed.y) + ")");
		if (dat1->counterHitSpeed != dat2->counterHitSpeed)
			game->logger.fatal(std::string(msgStart) + "FrameData::counterHitSpeed: (" + std::to_string(dat1->counterHitSpeed.x) + ", " + std::to_string(dat1->counterHitSpeed.y) + ") vs (" + std::to_string(dat2->counterHitSpeed.x) + ", " + std::to_string(dat2->counterHitSpeed.y) + ")");
		if (dat1->hasGravity && dat2->hasGravity && dat1->gravity != dat2->gravity)
			game->logger.fatal(std::string(msgStart) + "FrameData::gravity: (" + std::to_string(dat1->gravity.x) + ", " + std::to_string(dat1->gravity.y) + ") vs (" + std::to_string(dat2->gravity.x) + ", " + std::to_string(dat2->gravity.y) + ")");
		if (dat1->hasSnap && dat2->hasSnap && dat1->snap != dat2->snap)
			game->logger.fatal(std::string(msgStart) + "FrameData::snap: (" + std::to_string(dat1->snap.x) + ", " + std::to_string(dat1->snap.y) + ") vs (" + std::to_string(dat2->snap.x) + ", " + std::to_string(dat2->snap.y) + ")");

		if (dat1->hurtBoxesCount != dat2->hurtBoxesCount || dat1->hitBoxesCount != dat2->hitBoxesCount)
			return 0;

		while (i < dat1->hurtBoxesCount) {
			if (dat1->boxes[i].pos != dat2->boxes[i].pos)
				game->logger.fatal(std::string(msgStart) + "FrameData::hurtBoxes[" + std::to_string(i) + "]::pos: (" + std::to_string(dat1->boxes[i].pos.x) + ", " + std::to_string(dat1->boxes[i].pos.y) + ") vs (" + std::to_string(dat2->boxes[i].pos.x) + ", " + std::to_string(dat2->boxes[i].pos.y) + ")");
			if (dat1->boxes[i].size != dat2->boxes[i].size)
				game->logger.fatal(std::string(msgStart) + "FrameData::hurtBoxes[" + std::to_string(i) + "]::size: (" + std::to_string(dat1->boxes[i].size.x) + ", " + std::to_string(dat1->boxes[i].size.y) + ") vs (" + std::to_string(dat2->boxes[i].size.x) + ", " + std::to_string(dat2->boxes[i].size.y) + ")");
			i++;
		}
		while (i < dat1->hurtBoxesCount + dat1->hitBoxesCount) {
			if (dat1->boxes[i].pos != dat2->boxes[i].pos)
				game->logger.fatal(std::string(msgStart) + "FrameData::hitBoxes[" + std::to_string(i - dat1->hurtBoxesCount) + "]::pos: (" + std::to_string(dat1->boxes[i].pos.x) + ", " + std::to_string(dat1->boxes[i].pos.y) + ") vs (" + std::to_string(dat2->boxes[i].pos.x) + ", " + std::to_string(dat2->boxes[i].pos.y) + ")");
			if (dat1->boxes[i].size != dat2->boxes[i].size)
				game->logger.fatal(std::string(msgStart) + "FrameData::hitBoxes[" + std::to_string(i - dat1->hurtBoxesCount) + "]::size: (" + std::to_string(dat1->boxes[i].size.x) + ", " + std::to_string(dat1->boxes[i].size.y) + ") vs (" + std::to_string(dat2->boxes[i].size.x) + ", " + std::to_string(dat2->boxes[i].size.y) + ")");
			i++;
		}
		return sizeof(Data) + sizeof(Box) * (dat1->hurtBoxesCount + dat1->hitBoxesCount);
	}

	Box::operator sf::IntRect() const noexcept
	{
		return {
			this->pos.x, this->pos.y,
			static_cast<int>(this->size.x), static_cast<int>(this->size.y)
		};
	}
}
