//
// Created by Gegel85 on 15/08/22
//

#include "Cloud.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	Cloud::Cloud(nlohmann::json &json)
	{
		this->_moves = FrameData::loadFile(json["framedata"]);
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
			this->_dir * static_cast<float>(data.size.x) / data.textureBounds.size.x,
			static_cast<float>(data.size.y) / data.textureBounds.size.y
		};

		result.y *= -1;
		result += Vector2f{
			!this->_direction * data.size.x + data.size.x / -2.f,
			-static_cast<float>(data.size.y)
		};
		result += Vector2f{
			data.textureBounds.size.x * scale.x / 2,
			data.textureBounds.size.y * scale.y / 2
		};
		result.x += data.size.x;
		this->_airDrag = {1, 1};
		if (result.x < -100)
			this->_refresh();
	}

	void Cloud::_reload()
	{
		std::uniform_int_distribution<size_t> val{0, (this->_moves.size() * 32) - 32};
		std::uniform_int_distribution<size_t> val2{0, 1};
		std::uniform_real_distribution<float> val3{this->_minSpeed, this->_maxSpeed};
		auto rand = val(game->battleRandom);

		this->_action = rand / 32;
		this->_direction = val2(game->battleRandom);
		this->_dir = this->_direction ? 1 : -1;
		this->_speed.x = -val3(game->battleRandom);
	}

	void Cloud::_refresh()
	{
		std::uniform_int_distribution<size_t> val1{this->_minY, this->_maxY};
		auto &data = *this->getCurrentFrameData();
		auto result = Vector2f{data.offset.x * this->_dir, static_cast<float>(data.offset.y)};
		auto scale = Vector2f{
			this->_dir * static_cast<float>(data.size.x) / data.textureBounds.size.x,
			static_cast<float>(data.size.y) / data.textureBounds.size.y
		};

		result.y *= -1;
		result += Vector2f{
			!this->_direction * data.size.x + data.size.x / -2.f,
			-static_cast<float>(data.size.y)
		};
		result += Vector2f{
			data.textureBounds.size.x * scale.x / 2,
			data.textureBounds.size.y * scale.y / 2
		};

		this->_reload();
		this->_position.x = 1200 - result.x + data.size.x;
		this->_position.y = val1(game->battleRandom);
	}
}