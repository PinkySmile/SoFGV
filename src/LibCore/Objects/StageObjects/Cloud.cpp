//
// Created by PinkySmile on 15/08/2022.
//

#include "Cloud.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	Cloud::Cloud(nlohmann::json &json)
	{
		std::string framedata = json["framedata"];

		//TODO
		this->_moves = FrameData::loadFile(framedata, framedata.substr(0, framedata.find_last_of('/')));
		this->_position = {json["x"], json["y"]};
		this->_team = json["team"];
		this->_minSpeed = json["minSpeed"];
		this->_maxSpeed = json["maxSpeed"];
		this->_minY = json["minY"];
		this->_maxY = json["maxY"];
		this->_reload();
	}

	void Cloud::update()
	{
		Object::update();

		auto &data = *this->getCurrentFrameData();
		auto result = Vector2f{data.offset.x * this->_dir, static_cast<float>(data.offset.y)} + this->_position;
		auto scale = Vector2f{
			this->_dir * data.scale.x,
			data.scale.y
		};
		auto size = Vector2f{
			data.textureBounds.size.x * data.scale.x,
			data.textureBounds.size.y * data.scale.y
		};

		result.y *= -1;
		result += Vector2f{
			!this->_direction * size.x + size.x / -2.f,
			-size.y
		};
		result += Vector2f{
			data.textureBounds.size.x * scale.x / 2,
			data.textureBounds.size.y * scale.y / 2
		};
		result.x += size.x;
		this->_airDrag = {1, 1};
		if (result.x < STAGE_X_MIN - 100)
			this->_refresh();
	}

	void Cloud::_reload()
	{
		auto rand = random_distrib(game->battleRandom, 0, (this->_moves.size() * 32) - 31);

		this->_action = rand / 32;
		this->_direction = random_distrib(game->battleRandom, 0, 2);
		this->_dir = this->_direction ? 1 : -1;
		this->_speed.x = -random_distrib(game->battleRandom, this->_minSpeed, this->_maxSpeed);
		this->_rotation = 0;
	}

	void Cloud::_refresh()
	{
		auto &data = *this->getCurrentFrameData();
		auto result = Vector2f{data.offset.x * this->_dir, static_cast<float>(data.offset.y)};
		auto scale = Vector2f{
			this->_dir * data.scale.x,
			data.scale.y
		};
		auto size = Vector2f{
			data.textureBounds.size.x * data.scale.x,
			data.textureBounds.size.y * data.scale.y
		};

		result.y *= -1;
		result += Vector2f{
			!this->_direction * size.x + size.x / -2.f,
			-size.y
		};
		result += Vector2f{
			data.textureBounds.size.x * scale.x / 2,
			data.textureBounds.size.y * scale.y / 2
		};

		this->_reload();
		this->_position.x = STAGE_X_MAX + 200 - result.x + size.x;
		this->_position.y = random_distrib(game->battleRandom, this->_minY, this->_maxY);
	}

	int Cloud::getLayer() const
	{
		return -500;
	}
}