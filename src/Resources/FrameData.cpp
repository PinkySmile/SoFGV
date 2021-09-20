//
// Created by andgel on 18/09/2021
//

#include <fstream>
#include "FrameData.hpp"
#include "Game.hpp"

namespace Battle
{
	std::map<unsigned int, std::vector<std::vector<FrameData>>> Battle::FrameData::loadFile(const std::string &path)
	{
		std::ifstream stream{path};
		nlohmann::json json;

		if (stream.fail())
			// TODO: Create proper exceptions
			throw std::invalid_argument(path + ": " + strerror(errno));
		stream >> json;
		return loadFileJson(json);
	}

	std::map<unsigned, std::vector<std::vector<FrameData>>> FrameData::loadFileJson(const nlohmann::json &json)
	{
		std::map<unsigned int, std::vector<std::vector<FrameData>>> data;

		if (!json.is_array())
			// TODO: Create proper exceptions
			throw std::invalid_argument("Invalid json");
		for (auto &val : json) {
			if (!val.is_object())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!val.contains("action"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!val.contains("framedata"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");

			auto &action = val["action"];
			auto &framedata = val["framedata"];

			if (!action.is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!framedata.is_array())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (framedata.empty())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");

			unsigned actionId = action;

			for (auto &block : framedata) {
				if (!block.is_array())
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				data[actionId].emplace_back();
				for (auto &frame : block)
					data[actionId].back().emplace_back(frame);
			}
		}
		return data;
	}

	FrameData::FrameData(const nlohmann::json &data)
	{
		if (!data.is_object())
			// TODO: Create proper exceptions
			throw std::invalid_argument("Invalid json");

		if (!data.contains("sprite"))
			// TODO: Create proper exceptions
			throw std::invalid_argument("Invalid json");
		if (!data["sprite"].is_string())
			// TODO: Create proper exceptions
			throw std::invalid_argument("Invalid json");
		this->spritePath = data["sprite"];

		if (data.contains("offset")) {
			if (!data["offset"].is_object())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["offset"].contains("x"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["offset"].contains("y"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["offset"]["x"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["offset"]["y"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->offset.x = data["offset"]["x"];
			this->offset.y = data["offset"]["y"];
		}
		if (data.contains("size")) {
			if (!data["size"].is_object())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["size"].contains("x"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["size"].contains("y"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["size"]["x"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["size"]["y"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->size.x = data["size"]["x"];
			this->size.y = data["size"]["y"];
		}
		if (data.contains("texture_bounds")) {
			if (!data["texture_bounds"].is_object())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["texture_bounds"].contains("left"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["texture_bounds"].contains("top"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["texture_bounds"].contains("width"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["texture_bounds"].contains("height"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["texture_bounds"]["left"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["texture_bounds"]["top"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["texture_bounds"]["width"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["texture_bounds"]["height"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->textureBounds.pos.x = data["texture_bounds"]["left"];
			this->textureBounds.pos.y = data["texture_bounds"]["top"];
			this->textureBounds.size.x = data["texture_bounds"]["width"];
			this->textureBounds.size.y = data["texture_bounds"]["height"];
		}
		if (data.contains("rotation")) {
			if (!data["rotation"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->rotation = std::fmod(std::fmod(data["rotation"].get<float>(), 360) + 360, 360) * M_PI / 180;
		}
		if (data.contains("hurt_boxes")) {
			if (!data["hurt_boxes"].is_array())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			for (auto &box : data["hurt_boxes"]) {
				if (!box.is_object())
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box.contains("left"))
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box.contains("top"))
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box.contains("width"))
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box.contains("height"))
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box["left"].is_number())
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box["top"].is_number())
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box["width"].is_number())
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box["height"].is_number())
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
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
			if (!data["hit_boxes"].is_array())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			for (auto &box : data["hit_boxes"]) {
				if (!box.is_object())
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box.contains("left"))
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box.contains("top"))
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box.contains("width"))
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box.contains("height"))
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box["left"].is_number())
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box["top"].is_number())
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box["width"].is_number())
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
				if (!box["height"].is_number())
					// TODO: Create proper exceptions
					throw std::invalid_argument("Invalid json");
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
			if (!data["marker"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->specialMarker = data["marker"];
		}
		if (data.contains("defense_flag")) {
			if (!data["defense_flag"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->dFlag.flags = data["defense_flag"];
		} else
			this->dFlag.flags = 0;
		if (data.contains("offense_flag")) {
			if (!data["offense_flag"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->oFlag.flags = data["offense_flag"];
		} else
			this->oFlag.flags = 0;
		if (data.contains("collision_box")) {
			if (!data["collision_box"].is_object())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["collision_box"].contains("left"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["collision_box"].contains("top"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["collision_box"].contains("width"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["collision_box"].contains("height"))
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["collision_box"]["left"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["collision_box"]["top"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["collision_box"]["width"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			if (!data["collision_box"]["height"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->collisionBox = new Box{{
				data["collision_box"]["left"],
				data["collision_box"]["top"]
			}, {
				data["collision_box"]["width"],
				data["collision_box"]["height"]
			}};
		}
		if (data.contains("block_stun")) {
			if (!data["block_stun"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->blockStun = data["block_stun"];
		}
		if (data.contains("hit_stun")) {
			if (!data["hit_stun"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->hitStun = data["hit_stun"];
		}
		if (data.contains("prorate")) {
			if (!data["prorate"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->prorate = data["prorate"];
		}
		if (data.contains("void_limit")) {
			if (!data["void_limit"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->voidLimit = data["void_limit"];
		}
		if (data.contains("spirit_limit")) {
			if (!data["spirit_limit"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->spiritLimit = data["spirit_limit"];
		}
		if (data.contains("matter_limit")) {
			if (!data["matter_limit"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->matterLimit = data["matter_limit"];
		}
		if (data.contains("push_back")) {
			if (!data["push_back"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->pushBack = data["push_back"];
		}
		if (data.contains("push_block")) {
			if (!data["push_block"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->pushBlock = data["push_block"];
		}
		if (data.contains("duration")) {
			if (!data["duration"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->duration = data["duration"];
		}
		if (data.contains("subobject")) {
			if (!data["subobject"].is_number())
				// TODO: Create proper exceptions
				throw std::invalid_argument("Invalid json");
			this->subObjectSpawn = data["subobject"];
		}

		Vector2u textureSize;

		this->textureHandle = game.textureMgr.load(this->spritePath, &textureSize);
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
		game.textureMgr.remove(this->textureHandle);
		delete this->collisionBox;
	}

	FrameData::FrameData(const FrameData &other)
	{
		*this = other;
	}

	FrameData &FrameData::operator=(const FrameData &other)
	{
		this->spritePath = other.spritePath;
		this->textureHandle = other.textureHandle;
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
		if (other.collisionBox)
			this->collisionBox = new Box{
				other.collisionBox->pos,
				other.collisionBox->size
			};
		else
			this->collisionBox = nullptr;
		this->blockStun = other.blockStun;
		this->hitStun = other.hitStun;
		this->prorate = other.prorate;
		this->voidLimit = other.voidLimit;
		this->spiritLimit = other.spiritLimit;
		this->matterLimit = other.matterLimit;
		this->pushBack = other.pushBack;
		this->pushBlock = other.pushBlock;
		this->subObjectSpawn = other.subObjectSpawn;
		game.textureMgr.addRef(this->textureHandle);
		return *this;
	}

	void FrameData::reloadTexture()
	{
		game.textureMgr.remove(this->textureHandle);
		this->textureHandle = game.textureMgr.load(this->spritePath);
	}

	nlohmann::json FrameData::toJson() const
	{
		nlohmann::json result = nlohmann::json::object();

		result["sprite"] = this->spritePath;
		if (this->offset.x || this->offset.y)
			result["offset"] = {
				{"x", this->offset.x},
				{"y", this->offset.y}
			};
		if (this->size != game.textureMgr.getTextureSize(this->textureHandle))
			result["size"] = {
				{"x", this->size.x},
				{"y", this->size.y}
			};
		if (this->textureBounds.pos != Vector2i{0, 0} || this->textureBounds.size != game.textureMgr.getTextureSize(this->textureHandle))
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
		return result;
	}

	Box::operator sf::IntRect() const noexcept
	{
		return {
			this->pos.x, this->pos.y,
			static_cast<int>(this->size.x), static_cast<int>(this->size.y)
		};
	}
}
