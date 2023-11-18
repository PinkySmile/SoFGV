//
// Created by PinkySmile on 16/11/23.
//

#ifndef SOFGV_PARTICLE_HPP
#define SOFGV_PARTICLE_HPP


#include "IObject.hpp"
#include "Sprite.hpp"

namespace SpiralOfFate
{
	class Character;

	class Particle : public IObject {
	public:
		struct InitData {
			std::vector<std::array<Color, 4>> colors;
			std::pair<unsigned, unsigned> updateInterval;
			Box textureBounds;
			std::pair<unsigned, unsigned> mirror;
			std::pair<int, int> rotation;
			std::pair<Vector2f, Vector2f> acceleration;
			std::pair<Vector2f, Vector2f> velocity_limit;
			std::pair<unsigned, unsigned> lifeSpan;
			std::pair<unsigned, unsigned> fadeTime;
			std::pair<float, float> scale;
			float spawnChance;
			int layer;
			bool useRandomColor;
			bool disabledOnHit;

			InitData(nlohmann::json &data, const std::vector<Color> &colors);
		};

	protected:
		Sprite _sprite;
		const InitData &_data;
		const Character &_owner;
		unsigned _aliveTimer;
		unsigned _fadeTime;
		unsigned _maxFadeTime;
		unsigned _updateTimer;
		unsigned _currentColors = 0;
		float _scale;
		Vector2f _speed;
		Vector2f _position;

	public:
		Particle(const InitData &initData, const Character &owner, unsigned sprite, Vector2f position);

		void render() const override;
		void update() override;
		bool isDead() const override;
		void kill() override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *data1, void *data2, unsigned int startOffset) const override;
		size_t printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const override;
		unsigned int getClassId() const override;
		int getLayer() const override;
	};
}


#endif //SOFGV_PARTICLE_HPP
