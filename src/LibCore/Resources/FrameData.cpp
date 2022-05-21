//
// Created by andgel on 18/09/2021
//

#include <fstream>
#include "FrameData.hpp"
#include "Game.hpp"
#include "../Logger.hpp"

namespace SpiralOfFate
{
	std::map<unsigned int, std::vector<std::vector<FrameData>>> SpiralOfFate::FrameData::loadFile(const std::string &path, const std::pair<std::vector<Color>, std::vector<Color>> &palette)
	{
		std::ifstream stream{path};
		nlohmann::json json;

		game->logger.debug("Loading framedata file " + path);
		my_assert2(!stream.fail(), path + ": " + strerror(errno));
		stream >> json;
		return loadFileJson(json, palette);
	}

	std::map<unsigned, std::vector<std::vector<FrameData>>> FrameData::loadFileJson(const nlohmann::json &json, const std::pair<std::vector<Color>, std::vector<Color>> &palette)
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

			for (auto &block : framedata) {
				my_assert2(block.is_array(), "Invalid json");
				my_assert2(!block.empty(), "Invalid json");
				data[actionId].emplace_back();
				for (auto &frame : block)
					data[actionId].back().emplace_back(frame, palette);
			}
		}
		return data;
	}

	FrameData::FrameData(const nlohmann::json &data, const std::pair<std::vector<Color>, std::vector<Color>> &palette)
	{
		my_assert2(data.is_object(), "Invalid json");
		my_assert2(data.contains("sprite"), "Invalid json");
		my_assert2(data["sprite"].is_string(), "Invalid json");
		this->spritePath = data["sprite"];

		if (data.contains("sound")) {
			my_assert2(data["sound"].is_string(), "Invalid json");
			this->soundPath = data["sound"];
		}
		if (data.contains("hitSound")) {
			my_assert2(data["hitSound"].is_string(), "Invalid json");
			this->hitSoundPath = data["hitSound"];
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
		if (data.contains("prorate")) {
			my_assert2(data["prorate"].is_number(), "Invalid json");
			this->prorate = data["prorate"];
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
		if (data.contains("hit_stop")) {
			my_assert2(data["hit_stop"].is_number(), "Invalid json");
			this->hitStop = data["hit_stop"];
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

		this->_palette = palette;
		this->textureHandle = game->textureMgr.load(this->spritePath, palette, &textureSize);
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
		this->_palette = other._palette;
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
		this->prorate = other.prorate;
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
		this->hitStop = other.hitStop;
		this->hitSpeed = other.hitSpeed;
		this->speed = other.speed;
		this->counterHitSpeed = other.counterHitSpeed;
		this->gravity = other.gravity;
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
		this->_palette = other._palette;
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
		this->prorate = other.prorate;
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
		this->hitStop = other.hitStop;
		this->hitSpeed = other.hitSpeed;
		this->speed = other.speed;
		this->counterHitSpeed = other.counterHitSpeed;
		this->gravity = other.gravity;
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
		return *this;
	}

	void FrameData::reloadTexture()
	{
		my_assert(!this->_slave);
		game->textureMgr.remove(this->textureHandle);
		this->textureHandle = game->textureMgr.load(this->spritePath, this->_palette);
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
			result["hitSound"] = this->hitSoundPath;
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
		if (this->prorate != 0)
			result["prorate"] = this->prorate;
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
		if (this->hitStop)
			result["hit_stop"] = this->hitStop;
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
		if (!slave && this->_slave) {
			game->textureMgr.remove(this->textureHandle);
			game->soundMgr.remove(this->soundHandle);
			game->soundMgr.remove(this->hitSoundHandle);
			delete this->collisionBox;
			this->collisionBox = nullptr;
		}
		this->_slave = slave;
	}

	Box::operator sf::IntRect() const noexcept
	{
		return {
			this->pos.x, this->pos.y,
			static_cast<int>(this->size.x), static_cast<int>(this->size.y)
		};
	}
}
