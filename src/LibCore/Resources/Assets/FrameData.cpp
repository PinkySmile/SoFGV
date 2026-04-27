//
// Created by PinkySmile on 18/09/2021.
//

#include <fstream>
#include "Utils.hpp"
#include "FrameData.hpp"
#include "Resources/Game.hpp"
#include "Logger.hpp"
#include "Objects/CheckUtils.hpp"

namespace SpiralOfFate
{
	std::unordered_map<unsigned int, std::vector<std::vector<FrameData>>> FrameData::loadFile(const std::filesystem::path &path, const std::filesystem::path &folder, const std::filesystem::path &palette)
	{
		game->logger.debug("Loading framedata file " + path.string());
		return loadFileJson(nlohmann::json::parse(game->fileMgr.readFull(path)), folder, palette);
	}

	std::unordered_map<unsigned, std::vector<std::vector<FrameData>>> FrameData::loadFileJson(const nlohmann::json &json, const std::filesystem::path &folder, const std::filesystem::path &palette)
	{
		std::unordered_map<unsigned int, std::vector<std::vector<FrameData>>> data;

		game->logger.debug("Loading json");
		assert_msg(json.is_array(), "Invalid json");
		for (auto &val : json) {
			assert_msg(val.is_object(), "Invalid json");
			assert_msg(val.contains("action"), "Invalid json");
			assert_msg(val.contains("framedata"), "Invalid json");

			auto &action = val["action"];
			auto &framedata = val["framedata"];

			assert_msg(action.is_number(), "Invalid json");
			assert_msg(framedata.is_array(), "Invalid json");
			assert_msg(!framedata.empty(), "Invalid json");

			unsigned actionId = action;

			data[actionId].reserve(framedata.size());
			for (auto &block : framedata) {
				assert_msg(block.is_array(), "Invalid json");
				assert_msg(!block.empty(), "Invalid json");
				data[actionId].emplace_back();
				data[actionId].back().reserve(block.size());
				for (auto &frame : block)
					data[actionId].back().emplace_back(frame, folder, palette);
			}
		}
		return data;
	}

	FrameData::FrameData(const nlohmann::json &data, const std::filesystem::path &folder, const std::filesystem::path &palette)
	{
		Vector2u textureSize{0, 0};

		this->__folder = folder;
		this->__palette = palette;
		assert_msg(data.is_object(), "Invalid json");
		assert_msg(data.contains("sprite"), "Invalid json");
		assert_msg(data["sprite"].is_string(), "Invalid json");
		this->spritePath = data["sprite"];
		this->textureHandle = game->textureMgr.load(folder / this->spritePath, palette, &textureSize);
		this->textureHandleEffects = game->textureMgr.load(folder / "effects" / this->spritePath, palette, &textureSize);
		if (data.contains("sound")) {
			assert_msg(data["sound"].is_string(), "Invalid json");
			this->soundPath = data["sound"];
		}
		if (data.contains("hit_sound")) {
			assert_msg(data["hit_sound"].is_string(), "Invalid json");
			this->hitSoundPath = data["hit_sound"];
		}

		if (data.contains("offset")) {
			assert_msg(data["offset"].is_object(), "Invalid json");
			assert_msg(data["offset"].contains("x"), "Invalid json");
			assert_msg(data["offset"].contains("y"), "Invalid json");
			assert_msg(data["offset"]["x"].is_number(), "Invalid json");
			assert_msg(data["offset"]["y"].is_number(), "Invalid json");
			this->offset.x = data["offset"]["x"];
			this->offset.y = data["offset"]["y"];
		}
		if (data.contains("texture_bounds")) {
			assert_msg(data["texture_bounds"].is_object(), "Invalid json");
			assert_msg(data["texture_bounds"].contains("left"), "Invalid json");
			assert_msg(data["texture_bounds"].contains("top"), "Invalid json");
			assert_msg(data["texture_bounds"].contains("width"), "Invalid json");
			assert_msg(data["texture_bounds"].contains("height"), "Invalid json");
			assert_msg(data["texture_bounds"]["left"].is_number(), "Invalid json");
			assert_msg(data["texture_bounds"]["top"].is_number(), "Invalid json");
			assert_msg(data["texture_bounds"]["width"].is_number(), "Invalid json");
			assert_msg(data["texture_bounds"]["height"].is_number(), "Invalid json");
			this->textureBounds.pos.x = data["texture_bounds"]["left"];
			this->textureBounds.pos.y = data["texture_bounds"]["top"];
			this->textureBounds.size.x = data["texture_bounds"]["width"];
			this->textureBounds.size.y = data["texture_bounds"]["height"];
		}
		if (!this->textureBounds.size.x)
			this->textureBounds.size.x = textureSize.x;
		if (!this->textureBounds.size.y)
			this->textureBounds.size.y = textureSize.y;
		if (data.contains("scale")) {
			assert_msg(data["scale"].is_object(), "Invalid json");
			assert_msg(data["scale"].contains("x"), "Invalid json");
			assert_msg(data["scale"].contains("y"), "Invalid json");
			assert_msg(data["scale"]["x"].is_number(), "Invalid json");
			assert_msg(data["scale"]["y"].is_number(), "Invalid json");
			this->scale.x = data["scale"]["x"];
			this->scale.y = data["scale"]["y"];
		} else if (data.contains("size")) {
			assert_msg(data["size"].is_object(), "Invalid json");
			assert_msg(data["size"].contains("x"), "Invalid json");
			assert_msg(data["size"].contains("y"), "Invalid json");
			assert_msg(data["size"]["x"].is_number(), "Invalid json");
			assert_msg(data["size"]["y"].is_number(), "Invalid json");
			this->scale.x = data["size"]["x"].get<float>() / this->textureBounds.size.x;
			this->scale.y = data["size"]["y"].get<float>() / this->textureBounds.size.y;
		}
		if (data.contains("gravity")) {
			assert_msg(data["gravity"].is_object(), "Invalid json");
			assert_msg(data["gravity"].contains("x"), "Invalid json");
			assert_msg(data["gravity"].contains("y"), "Invalid json");
			assert_msg(data["gravity"]["x"].is_number(), "Invalid json");
			assert_msg(data["gravity"]["y"].is_number(), "Invalid json");
			this->gravity = Vector2f(data["gravity"]["x"], data["gravity"]["y"]);
		}
		if (data.contains("snap")) {
			assert_msg(data["snap"].is_object(), "Invalid json");
			assert_msg(data["snap"].contains("x"), "Invalid json");
			assert_msg(data["snap"].contains("y"), "Invalid json");
			assert_msg(data["snap"]["x"].is_number(), "Invalid json");
			assert_msg(data["snap"]["y"].is_number(), "Invalid json");
			assert_msg(!data["snap"].contains("r") || data["snap"]["r"].is_number(), "Invalid json");
			this->snap = std::pair(Vector2f(data["snap"]["x"], data["snap"]["y"]), data["snap"].contains("r") ? data["snap"]["r"].get<float>() : 0);
		}
		if (data.contains("rotation")) {
			assert_msg(data["rotation"].is_number(), "Invalid json");
			this->rotation = std::fmod(std::fmod(data["rotation"].get<float>(), 360) + 360, 360) * M_PI / 180;
		}
		if (data.contains("hurt_boxes")) {
			assert_msg(data["hurt_boxes"].is_array(), "Invalid json");
			for (auto &box : data["hurt_boxes"]) {
				assert_msg(box.is_object(), "Invalid json");
				assert_msg(box.contains("left"), "Invalid json");
				assert_msg(box.contains("top"), "Invalid json");
				assert_msg(box.contains("width"), "Invalid json");
				assert_msg(box.contains("height"), "Invalid json");
				assert_msg(box["left"].is_number(), "Invalid json");
				assert_msg(box["top"].is_number(), "Invalid json");
				assert_msg(box["width"].is_number(), "Invalid json");
				assert_msg(box["height"].is_number(), "Invalid json");
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
			assert_msg(data["hit_boxes"].is_array(), "Invalid json");
			for (auto &box : data["hit_boxes"]) {
				assert_msg(box.is_object(), "Invalid json");
				assert_msg(box.contains("left"), "Invalid json");
				assert_msg(box.contains("top"), "Invalid json");
				assert_msg(box.contains("width"), "Invalid json");
				assert_msg(box.contains("height"), "Invalid json");
				assert_msg(box["left"].is_number(), "Invalid json");
				assert_msg(box["top"].is_number(), "Invalid json");
				assert_msg(box["width"].is_number(), "Invalid json");
				assert_msg(box["height"].is_number(), "Invalid json");
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
			assert_msg(data["marker"].is_number(), "Invalid json");
			this->specialMarker = data["marker"];
		}
		if (data.contains("defense_flag")) {
			assert_msg(data["defense_flag"].is_number(), "Invalid json");
			this->dFlag.flags = data["defense_flag"];
		} else
			this->dFlag.flags = 0;
		if (data.contains("offense_flag")) {
			assert_msg(data["offense_flag"].is_number(), "Invalid json");
			this->oFlag.flags = data["offense_flag"];
		} else
			this->oFlag.flags = 0;
		if (data.contains("collision_box")) {
			assert_msg(data["collision_box"].is_object(), "Invalid json");
			assert_msg(data["collision_box"].contains("left"), "Invalid json");
			assert_msg(data["collision_box"].contains("top"), "Invalid json");
			assert_msg(data["collision_box"].contains("width"), "Invalid json");
			assert_msg(data["collision_box"].contains("height"), "Invalid json");
			assert_msg(data["collision_box"]["left"].is_number(), "Invalid json");
			assert_msg(data["collision_box"]["top"].is_number(), "Invalid json");
			assert_msg(data["collision_box"]["width"].is_number(), "Invalid json");
			assert_msg(data["collision_box"]["height"].is_number(), "Invalid json");
			this->collisionBox = new Box{{
				data["collision_box"]["left"],
				data["collision_box"]["top"]
			}, {
				data["collision_box"]["width"],
				data["collision_box"]["height"]
			}};
		}
		if (data.contains("particle_generator")) {
			assert_msg(data["particle_generator"].is_number(), "Invalid json");
			this->particleGenerator = data["particle_generator"];
		}
		if (data.contains("fade_time")) {
			assert_msg(data["fade_time"].is_number(), "Invalid json");
			this->fadeTime = data["fade_time"];
		}
		if (data.contains("block_stun")) {
			assert_msg(data["block_stun"].is_number(), "Invalid json");
			this->blockStun = data["block_stun"];
		}
		if (data.contains("wrong_block_stun")) {
			assert_msg(data["wrong_block_stun"].is_number(), "Invalid json");
			this->wrongBlockStun = data["wrong_block_stun"];
		} else
			this->wrongBlockStun = this->blockStun + 8;
		if (data.contains("chip_damage")) {
			assert_msg(data["chip_damage"].is_number(), "Invalid json");
			this->chipDamage = data["chip_damage"];
		}
		if (data.contains("priority")) {
			assert_msg(data["priority"].is_number(), "Invalid json");
			this->priority = data["priority"];
		}
		if (data.contains("hit_stun")) {
			assert_msg(data["hit_stun"].is_number(), "Invalid json");
			this->hitStun = data["hit_stun"];
		}
		if (data.contains("untech")) {
			assert_msg(data["untech"].is_number(), "Invalid json");
			this->untech = data["untech"];
		}
		if (data.contains("guard_damage")) {
			assert_msg(data["guard_damage"].is_number(), "Invalid json");
			this->guardDmg = data["guard_damage"];
		}
		if (data.contains("prorate")) {
			assert_msg(data["prorate"].is_number(), "Invalid json");
			this->prorate = data["prorate"];
		}
		if (data.contains("min_prorate")) {
			assert_msg(data["min_prorate"].is_number(), "Invalid json");
			this->minProrate = data["min_prorate"];
		}
		if (data.contains("neutral_limit")) {
			assert_msg(data["neutral_limit"].is_number(), "Invalid json");
			this->neutralLimit = data["neutral_limit"];
		}
		if (data.contains("void_limit")) {
			assert_msg(data["void_limit"].is_number(), "Invalid json");
			this->voidLimit = data["void_limit"];
		}
		if (data.contains("spirit_limit")) {
			assert_msg(data["spirit_limit"].is_number(), "Invalid json");
			this->spiritLimit = data["spirit_limit"];
		}
		if (data.contains("matter_limit")) {
			assert_msg(data["matter_limit"].is_number(), "Invalid json");
			this->matterLimit = data["matter_limit"];
		}
		if (data.contains("push_back")) {
			assert_msg(data["push_back"].is_number(), "Invalid json");
			this->pushBack = data["push_back"];
		}
		if (data.contains("push_block")) {
			assert_msg(data["push_block"].is_number(), "Invalid json");
			this->pushBlock = data["push_block"];
		}
		if (data.contains("duration")) {
			assert_msg(data["duration"].is_number(), "Invalid json");
			this->duration = data["duration"];
		}
		if (data.contains("subobject")) {
			assert_msg(data["subobject"].is_number(), "Invalid json");
			this->subObjectSpawn = data["subobject"];
		}
		if (data.contains("mana_gain")) {
			assert_msg(data["mana_gain"].is_number(), "Invalid json");
			this->manaGain = data["mana_gain"];
		}
		if (data.contains("mana_cost")) {
			assert_msg(data["mana_cost"].is_number(), "Invalid json");
			this->manaCost = data["mana_cost"];
		}
		if (data.contains("hit_player_hit_stop")) {
			assert_msg(data["hit_player_hit_stop"].is_number(), "Invalid json");
			this->hitPlayerHitStop = data["hit_player_hit_stop"];
		}
		if (data.contains("hit_opponent_hit_stop")) {
			assert_msg(data["hit_opponent_hit_stop"].is_number(), "Invalid json");
			this->hitOpponentHitStop = data["hit_opponent_hit_stop"];
		}
		if (data.contains("block_player_hit_stop")) {
			assert_msg(data["block_player_hit_stop"].is_number(), "Invalid json");
			this->blockPlayerHitStop = data["block_player_hit_stop"];
		}
		if (data.contains("block_opponent_hit_stop")) {
			assert_msg(data["block_opponent_hit_stop"].is_number(), "Invalid json");
			this->blockOpponentHitStop = data["block_opponent_hit_stop"];
		}
		if (data.contains("damage")) {
			assert_msg(data["damage"].is_number(), "Invalid json");
			this->damage = data["damage"];
		}
		if (data.contains("speed")) {
			assert_msg(data["speed"].is_object(), "Invalid json");
			assert_msg(data["speed"].contains("x"), "Invalid json");
			assert_msg(data["speed"].contains("y"), "Invalid json");
			assert_msg(data["speed"]["x"].is_number(), "Invalid json");
			assert_msg(data["speed"]["y"].is_number(), "Invalid json");
			this->speed.x = data["speed"]["x"];
			this->speed.y = data["speed"]["y"];
		}
		if (data.contains("hit_speed")) {
			assert_msg(data["hit_speed"].is_object(), "Invalid json");
			assert_msg(data["hit_speed"].contains("x"), "Invalid json");
			assert_msg(data["hit_speed"].contains("y"), "Invalid json");
			assert_msg(data["hit_speed"]["x"].is_number(), "Invalid json");
			assert_msg(data["hit_speed"]["y"].is_number(), "Invalid json");
			this->hitSpeed.x = data["hit_speed"]["x"];
			this->hitSpeed.y = data["hit_speed"]["y"];
		}
		if (data.contains("counter_hit_speed")) {
			assert_msg(data["counter_hit_speed"].is_object(), "Invalid json");
			assert_msg(data["counter_hit_speed"].contains("x"), "Invalid json");
			assert_msg(data["counter_hit_speed"].contains("y"), "Invalid json");
			assert_msg(data["counter_hit_speed"]["x"].is_number(), "Invalid json");
			assert_msg(data["counter_hit_speed"]["y"].is_number(), "Invalid json");
			this->counterHitSpeed.x = data["counter_hit_speed"]["x"];
			this->counterHitSpeed.y = data["counter_hit_speed"]["y"];
		}
		if (!this->soundPath.empty()) {
			if (std::ranges::all_of(this->soundPath, [](char c){ return std::isdigit(c) == 1; })) {
				this->soundHandle = std::stoul(this->soundPath);
				game->soundMgr.addRef(this->soundHandle);
			} else
				this->soundHandle = game->soundMgr.load("assets/sfxs/se/" + this->soundPath);
		}
		if (!this->hitSoundPath.empty()){
			if (std::ranges::all_of(this->hitSoundPath, [](char c){ return std::isdigit(c) == 1; })) {
				this->hitSoundHandle = std::stoul(this->hitSoundPath);
				game->soundMgr.addRef(this->hitSoundHandle);
			} else
				this->hitSoundHandle = game->soundMgr.load("assets/sfxs/se/" + this->hitSoundPath);
		}
	}

	FrameData::~FrameData()
	{
		if (!this->_slave) {
			game->textureMgr.remove(this->textureHandle);
			game->textureMgr.remove(this->textureHandleEffects);
			game->soundMgr.remove(this->soundHandle);
			game->soundMgr.remove(this->hitSoundHandle);
			delete this->collisionBox;
		}
	}

	FrameData::FrameData(const FrameData &other)
	{
		this->__folder = other.__folder;
		this->__palette = other.__palette;
		this->__requireReload = other.__requireReload;
		this->__paletteData = other.__paletteData;
		this->particleGenerator = other.particleGenerator;
		this->chipDamage = other.chipDamage;
		this->priority = other.priority;
		this->spritePath = other.spritePath;
		this->textureHandle = other.textureHandle;
		this->textureHandleEffects = other.textureHandleEffects;
		this->soundPath = other.soundPath;
		this->soundHandle = other.soundHandle;
		this->hitSoundPath = other.hitSoundPath;
		this->hitSoundHandle = other.hitSoundHandle;
		this->offset = other.offset;
		this->scale = other.scale;
		this->textureBounds = other.textureBounds;
		this->rotation = other.rotation;
		this->hurtBoxes = other.hurtBoxes;
		this->hitBoxes = other.hitBoxes;
		this->duration = other.duration;
		this->specialMarker = other.specialMarker;
		this->dFlag = other.dFlag;
		this->oFlag = other.oFlag;
		this->blockStun = other.blockStun;
		this->wrongBlockStun = other.wrongBlockStun;
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
			game->textureMgr.addRef(this->textureHandleEffects);
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
		this->__folder = other.__folder;
		this->__palette = other.__palette;
		this->__requireReload = other.__requireReload;
		this->__paletteData = other.__paletteData;
		if (!this->_slave) {
			game->textureMgr.remove(this->textureHandle);
			game->textureMgr.remove(this->textureHandleEffects);
			game->soundMgr.remove(this->soundHandle);
			game->soundMgr.remove(this->hitSoundHandle);
		}
		this->particleGenerator = other.particleGenerator;
		this->chipDamage = other.chipDamage;
		this->priority = other.priority;
		this->spritePath = other.spritePath;
		this->textureHandle = other.textureHandle;
		this->textureHandleEffects = other.textureHandleEffects;
		this->soundPath = other.soundPath;
		this->soundHandle = other.soundHandle;
		this->hitSoundPath = other.hitSoundPath;
		this->hitSoundHandle = other.hitSoundHandle;
		this->offset = other.offset;
		this->scale = other.scale;
		this->textureBounds = other.textureBounds;
		this->rotation = other.rotation;
		this->hurtBoxes = other.hurtBoxes;
		this->hitBoxes = other.hitBoxes;
		this->duration = other.duration;
		this->specialMarker = other.specialMarker;
		this->dFlag = other.dFlag;
		this->oFlag = other.oFlag;
		this->blockStun = other.blockStun;
		this->wrongBlockStun = other.wrongBlockStun;
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
			game->textureMgr.addRef(this->textureHandleEffects);
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

	void FrameData::checkReloadTexture()
	{
		if (this->__requireReload)
			this->reloadTexture();
		this->__requireReload = false;
	}

	void FrameData::reloadTexture()
	{
		assert_exp(!this->_slave);
		game->textureMgr.remove(this->textureHandle);
		game->textureMgr.remove(this->textureHandleEffects);
		if (!this->__paletteData) {
			this->textureHandle = game->textureMgr.load(this->__folder / this->spritePath, this->__palette);
			this->textureHandleEffects = game->textureMgr.load(this->__folder / "effects" / this->spritePath, this->__palette);
		} else {
			this->textureHandle = game->textureMgr.load(this->__folder / this->spritePath, *this->__paletteData);
			this->textureHandleEffects = game->textureMgr.load(this->__folder / "effects" / this->spritePath, *this->__paletteData);
		}
	}

	void FrameData::reloadSound()
	{
		assert_exp(!this->_slave);
		game->soundMgr.remove(this->soundHandle);
		game->soundMgr.remove(this->hitSoundHandle);
		this->soundHandle = 0;
		this->hitSoundHandle = 0;
		if (!this->soundPath.empty()) {
			if (this->soundPath.find('.') == std::string::npos) {
				this->soundHandle = std::stoul(this->soundPath);
				game->soundMgr.addRef(this->soundHandle);
			} else
				this->soundHandle = game->soundMgr.load("assets/sfxs/se/" + this->soundPath);
		}
		if (!this->hitSoundPath.empty()){
			if (this->hitSoundPath.find('.') == std::string::npos) {
				this->hitSoundHandle = std::stoul(this->hitSoundPath);
				game->soundMgr.addRef(this->hitSoundHandle);
			} else
				this->hitSoundHandle = game->soundMgr.load("assets/sfxs/se/" + this->hitSoundPath);
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
		if (this->particleGenerator != 0)
			result["particle_generator"] = this->particleGenerator;
		if (this->offset.x != 0 || this->offset.y != 0)
			result["offset"] = {
				{"x", this->offset.x},
				{"y", this->offset.y}
			};
		if (this->speed.x != 0 || this->speed.y != 0)
			result["speed"] = {
				{"x", this->speed.x},
				{"y", this->speed.y}
			};
		if (this->gravity.has_value())
			result["gravity"] = {
				{"x", this->gravity->x},
				{"y", this->gravity->y}
			};
		if (this->snap.has_value())
			result["snap"] = {
				{"x", this->snap->first.x},
				{"y", this->snap->first.y},
				{"r", this->snap->second}
			};
		if (this->scale != Vector2f{1, 1})
			result["scale"] = {
				{"x", this->scale.x},
				{"y", this->scale.y}
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
		if (this->specialMarker != 0)
			result["marker"] = this->specialMarker;
		if (this->damage != 0)
			result["damage"] = this->damage;
		if (this->fadeTime != 0)
			result["fade_time"] = this->fadeTime;
		if (this->chipDamage != 0)
			result["chip_damage"] = this->chipDamage;
		if (this->priority.has_value())
			result["priority"] = *this->priority;
		if (this->dFlag.flags != 0)
			result["defense_flag"] = this->dFlag.flags;
		if (this->oFlag.flags != 0)
			result["offense_flag"] = this->oFlag.flags;
		if (this->collisionBox != nullptr)
			result["collision_box"] = *this->collisionBox;
		if (this->blockStun != 0)
			result["block_stun"] = this->blockStun;
		if (this->wrongBlockStun != this->blockStun + 8)
			result["wrong_block_stun"] = this->wrongBlockStun;
		if (this->hitStun != 0)
			result["hit_stun"] = this->hitStun;
		if (this->untech != 0)
			result["untech"] = this->untech;
		if (this->guardDmg != 0)
			result["guard_damage"] = this->guardDmg;
		if (this->prorate != 0)
			result["prorate"] = this->prorate;
		if (this->minProrate != 0)
			result["min_prorate"] = this->minProrate;
		if (this->neutralLimit != 0)
			result["neutral_limit"] = this->neutralLimit;
		if (this->voidLimit != 0)
			result["void_limit"] = this->voidLimit;
		if (this->spiritLimit != 0)
			result["spirit_limit"] = this->spiritLimit;
		if (this->matterLimit != 0)
			result["matter_limit"] = this->matterLimit;
		if (this->pushBack != 0)
			result["push_back"] = this->pushBack;
		if (this->pushBlock != 0)
			result["push_block"] = this->pushBlock;
		if (this->duration > 1)
			result["duration"] = this->duration;
		if (this->subObjectSpawn != 0)
			result["subobject"] = this->subObjectSpawn;
		if (this->manaGain != 0)
			result["mana_gain"] = this->manaGain;
		if (this->manaCost != 0)
			result["mana_cost"] = this->manaCost;
		if (this->hitPlayerHitStop != 0)
			result["hit_player_hit_stop"] = this->hitPlayerHitStop;
		if (this->hitOpponentHitStop != 0)
			result["hit_opponent_hit_stop"] = this->hitOpponentHitStop;
		if (this->blockPlayerHitStop != 0)
			result["block_player_hit_stop"] = this->blockPlayerHitStop;
		if (this->blockOpponentHitStop != 0)
			result["block_opponent_hit_stop"] = this->blockOpponentHitStop;
		if (this->hitSpeed.x != 0 || this->hitSpeed.y != 0)
			result["hit_speed"] = {
				{"x", this->hitSpeed.x},
				{"y", this->hitSpeed.y}
			};
		if (this->counterHitSpeed.x != 0 || this->counterHitSpeed.y != 0)
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
			game->textureMgr.remove(this->textureHandleEffects);
			game->soundMgr.remove(this->soundHandle);
			game->soundMgr.remove(this->hitSoundHandle);
			delete this->collisionBox;
			this->collisionBox = nullptr;
		} else {
			game->textureMgr.addRef(this->textureHandle);
			game->textureMgr.addRef(this->textureHandleEffects);
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
		auto dat = static_cast<Data *>(data);
		unsigned i = 0;

		game->logger.verbose("Saving FrameData (Data size: " + std::to_string(this->getBufferSize()) + ") @" + Utils::toHex(reinterpret_cast<uintptr_t>(dat)));
		memset(dat->texturePath, 0, sizeof(dat->texturePath));
		strncpy(dat->texturePath, this->spritePath.c_str(), sizeof(dat->texturePath));
		dat->particleGenerator = this->particleGenerator;
		dat->blockStun = this->blockStun;
		dat->wrongBlockStun = this->wrongBlockStun;
		dat->hitStun = this->hitStun;
		dat->untech = this->untech;
		dat->guardDmg = this->guardDmg;
		dat->duration = this->duration;
		dat->fadeTime = this->fadeTime;
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
		dat->scale = this->scale;
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
		else
			dat->collisionBox = {{0, 0}, {0, 0}};
		if (this->gravity)
			dat->gravity = *this->gravity;
		else
			dat->gravity = {0, 0};
		if (this->snap) {
			dat->snapPos = this->snap->first;
			dat->snapRot = this->snap->second;
		} else {
			dat->snapPos = {0, 0};
			dat->snapRot = 0;
		}
		if (this->priority)
			dat->priority = *this->priority;
		else
			dat->priority = 0;
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

		assert_exp(!this->_slave);
		game->textureMgr.remove(this->textureHandle);
		game->textureMgr.remove(this->textureHandleEffects);
		delete this->collisionBox;

		this->spritePath = std::string(dat->texturePath, strnlen(dat->texturePath, sizeof(dat->texturePath)));
		this->textureHandle = game->textureMgr.load(this->__folder / this->spritePath, __palette);
		this->textureHandleEffects = game->textureMgr.load(this->__folder / "effects" / this->spritePath, __palette);
		this->particleGenerator = dat->particleGenerator;
		this->blockStun = dat->blockStun;
		this->wrongBlockStun = dat->wrongBlockStun;
		this->hitStun = dat->hitStun;
		this->untech = dat->untech;
		this->fadeTime = dat->fadeTime;
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
		this->scale = dat->scale;
		this->offset = dat->offset;
		this->speed = dat->speed;
		this->hitSpeed = dat->hitSpeed;
		this->counterHitSpeed = dat->counterHitSpeed;
		this->textureBounds = dat->textureBounds;
		if (dat->hasCollisionBox)
			this->collisionBox = new Box(dat->collisionBox);
		else
			this->collisionBox = nullptr;
		if (dat->hasGravity)
			this->gravity = dat->gravity;
		else
			this->gravity.reset();
		if (dat->hasSnap)
			this->snap.emplace(dat->snapPos, dat->snapRot);
		else
			this->snap.reset();
		if (dat->hasPriority)
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
		game->logger.verbose("Restored FrameData @" + Utils::toHex((uintptr_t)dat));
	}

	size_t FrameData::printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset)
	{
		auto dat1 = reinterpret_cast<Data *>(data1);
		auto dat2 = reinterpret_cast<Data *>(data2);
		unsigned i = 0;

		game->logger.info("FrameData @" + std::to_string(startOffset));
		if (strncmp(dat1->texturePath, dat2->texturePath, sizeof(dat2->texturePath)))
			game->logger.fatal(std::string(msgStart) + "FrameData::texturePath: " + std::string(dat1->texturePath, strnlen(dat1->texturePath, sizeof(dat1->texturePath))) + " vs " + std::string(dat2->texturePath, strnlen(dat2->texturePath, sizeof(dat2->texturePath))));
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, particleGenerator, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, blockStun, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, wrongBlockStun, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, hitStun, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, untech, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, guardDmg, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, duration, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, fadeTime, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, specialMarker, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, neutralLimit, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, voidLimit, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, spiritLimit, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, matterLimit, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, manaGain, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, manaCost, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, hitPlayerHitStop, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, hitOpponentHitStop, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, blockPlayerHitStop, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, blockOpponentHitStop, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, damage, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, chipDamage, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, pushBack, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, pushBlock, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, subObjectSpawn, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, prorate, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, minProrate, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, rotation, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, dFlag.flags, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, oFlag.flags, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, priority, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, hasCollisionBox, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, hasPriority, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, hasGravity, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, hasSnap, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, hurtBoxesCount, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, hitBoxesCount, std::to_string);

		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, scale, DISP_VEC);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, offset, DISP_VEC);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, speed, DISP_VEC);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, hitSpeed, DISP_VEC);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, gravity, DISP_VEC);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, snapPos, DISP_VEC);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, snapRot, std::to_string);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, counterHitSpeed, DISP_VEC);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, textureBounds.pos, DISP_VEC);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, textureBounds.size, DISP_VEC);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, collisionBox.pos, DISP_VEC);
		OBJECT_CHECK_FIELD("FrameData", "", dat1, dat2, collisionBox.size, DISP_VEC);

		if (dat1->hurtBoxesCount != dat2->hurtBoxesCount || dat1->hitBoxesCount != dat2->hitBoxesCount)
			return 0;

		while (i < dat1->hurtBoxesCount) {
			const Box &box1 = dat1->boxes[i];
			const Box &box2 = dat2->boxes[i];

			OBJECT_CHECK_FIELD_VAL("FrameData", box1.pos,  box2.pos,  "hurtBoxes[" + std::to_string(i) + "]::pos",  DISP_VEC);
			OBJECT_CHECK_FIELD_VAL("FrameData", box1.size, box2.size, "hurtBoxes[" + std::to_string(i) + "]::size", DISP_VEC);
			i++;
		}
		while (i < dat1->hurtBoxesCount + dat1->hitBoxesCount) {
			const Box &box1 = dat1->boxes[i];
			const Box &box2 = dat2->boxes[i];

			OBJECT_CHECK_FIELD_VAL("FrameData", box1.pos,  box2.pos,  "hitBoxes[" + std::to_string(i - dat1->hurtBoxesCount) + "]::pos",  DISP_VEC);
			OBJECT_CHECK_FIELD_VAL("FrameData", box1.size, box2.size, "hitBoxes[" + std::to_string(i - dat1->hurtBoxesCount) + "]::size", DISP_VEC);
			i++;
		}
		return sizeof(Data) + sizeof(Box) * (dat1->hurtBoxesCount + dat1->hitBoxesCount);
	}

	size_t FrameData::printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize)
	{
		auto dat = reinterpret_cast<Data *>(data);
		unsigned i = 0;

		game->logger.info("FrameData @" + std::to_string(startOffset));
		if (startOffset + sizeof(Data) + sizeof(Box) * (dat->hurtBoxesCount + dat->hitBoxesCount) >= dataSize)
			game->logger.warn("Object is " + std::to_string(startOffset + sizeof(Data) - dataSize) + " bytes bigger than input");
		game->logger.info(std::string(msgStart) + "FrameData::texturePath: " + std::string(dat->texturePath, strnlen(dat->texturePath, sizeof(dat->texturePath))));

		DISPLAY_FIELD("FrameData", "", dat, particleGenerator, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, blockStun, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, wrongBlockStun, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, hitStun, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, untech, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, guardDmg, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, duration, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, fadeTime, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, specialMarker, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, neutralLimit, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, voidLimit, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, spiritLimit, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, matterLimit, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, manaGain, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, manaCost, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, hitPlayerHitStop, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, hitOpponentHitStop, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, blockPlayerHitStop, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, blockOpponentHitStop, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, damage, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, chipDamage, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, pushBack, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, pushBlock, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, subObjectSpawn, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, prorate, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, minProrate, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, rotation, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, dFlag.flags, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, oFlag.flags, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, priority, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, hasCollisionBox, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, hasPriority, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, hasGravity, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, hasSnap, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, hurtBoxesCount, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, hitBoxesCount, std::to_string);

		DISPLAY_FIELD("FrameData", "", dat, scale, DISP_VEC);
		DISPLAY_FIELD("FrameData", "", dat, offset, DISP_VEC);
		DISPLAY_FIELD("FrameData", "", dat, speed, DISP_VEC);
		DISPLAY_FIELD("FrameData", "", dat, hitSpeed, DISP_VEC);
		DISPLAY_FIELD("FrameData", "", dat, gravity, DISP_VEC);
		DISPLAY_FIELD("FrameData", "", dat, snapPos, DISP_VEC);
		DISPLAY_FIELD("FrameData", "", dat, snapRot, std::to_string);
		DISPLAY_FIELD("FrameData", "", dat, counterHitSpeed, DISP_VEC);
		DISPLAY_FIELD("FrameData", "", dat, textureBounds.pos, DISP_VEC);
		DISPLAY_FIELD("FrameData", "", dat, textureBounds.size, DISP_VEC);
		DISPLAY_FIELD("FrameData", "", dat, collisionBox.pos, DISP_VEC);
		DISPLAY_FIELD("FrameData", "", dat, collisionBox.size, DISP_VEC);

		while (i < dat->hurtBoxesCount) {
			const Box &box = dat->boxes[i];

			DISPLAY_FIELD_VAL("FrameData", box.pos,  "hurtBoxes[" + std::to_string(i) + "]::pos",  DISP_VEC);
			DISPLAY_FIELD_VAL("FrameData", box.size, "hurtBoxes[" + std::to_string(i) + "]::size", DISP_VEC);
			i++;
		}
		while (i < dat->hurtBoxesCount + dat->hitBoxesCount) {
			const Box &box = dat->boxes[i];

			DISPLAY_FIELD_VAL("FrameData", box.pos,  "hitBoxes[" + std::to_string(i - dat->hurtBoxesCount) + "]::pos",  DISP_VEC);
			DISPLAY_FIELD_VAL("FrameData", box.size, "hitBoxes[" + std::to_string(i - dat->hurtBoxesCount) + "]::size", DISP_VEC);
			i++;
		}
		if (startOffset + sizeof(Data) + sizeof(Box) * (dat->hurtBoxesCount + dat->hitBoxesCount) >= dataSize) {
			game->logger.fatal("Invalid input frame");
			return 0;
		}
		return sizeof(Data) + sizeof(Box) * (dat->hurtBoxesCount + dat->hitBoxesCount);
	}

	Box::operator IntRect() const noexcept
	{
		return {
			this->pos,
			this->size.to<int>()
		};
	}

	bool Box::operator!=(const Box &other) const
	{
		return this->pos != other.pos || this->size != other.size;
	}

	bool DefensiveFlags::operator!=(const DefensiveFlags &other) const
	{
		return this->flags != other.flags;
	}

	bool OffensiveFlags::operator!=(const OffensiveFlags &other) const
	{
		return this->flags != other.flags;
	}

}
