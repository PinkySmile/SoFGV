//
// Created by andgel on 18/09/2021
//

#include <fstream>
#include "FrameData.hpp"
#include "Game.hpp"

namespace Battle
{
	std::map<unsigned int, std::vector<FrameData>> Battle::FrameData::loadFile(const std::string &path)
	{
		std::ifstream stream{path};
		nlohmann::json json;

		if (stream.fail())
			// TODO: Create proper exceptions
			throw std::invalid_argument(path + ": " + strerror(errno));
		stream >> json;
		return loadFileJson(json);
	}

	std::map<unsigned, std::vector<FrameData>> FrameData::loadFileJson(const nlohmann::json &json)
	{
		std::map<unsigned int, std::vector<FrameData>> data;

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

			for (auto &frame : framedata)
				data[actionId].emplace_back(frame);
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
			this->rotation = data["rotation"].get<float>() * M_PI / 180;
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

		Vector2u textureSize;

		game.textureMgr.load(this->spritePath, &textureSize);
		if (!this->textureBounds.size.x)
			this->textureBounds.size.x = textureSize.x;
		if (!this->textureBounds.size.y)
			this->textureBounds.size.y = textureSize.y;
		if (!this->size.x)
			this->textureBounds.size.x = textureSize.x;
		if (!this->size.y)
			this->textureBounds.size.y = textureSize.y;
	}
}
