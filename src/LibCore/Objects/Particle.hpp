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
			Vector2u mirror;
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

			InitData(const nlohmann::json &data, const std::vector<Color> &colors);
		};
		typedef std::tuple<unsigned char, unsigned, unsigned> Source;

	protected:
#pragma pack(push, 1)
		struct Data {
			unsigned _aliveTimer;
			unsigned _fadeTime;
			unsigned _maxFadeTime;
			unsigned _updateTimer;
			unsigned _currentColors;
			float _scale;
			Vector2f _speed;
			Vector2f _position;
			Vector2<bool> _mirror;
		};
#pragma pack(pop)
		static_assert(sizeof(Data) == 42, "Data has wrong size");

		// Non-Game State
		Source _source;
		const Character &_owner;
		const InitData &_data;
		Sprite _sprite;

		// Game State
		unsigned _aliveTimer;
		unsigned _fadeTime;
		unsigned _maxFadeTime;
		unsigned _updateTimer;
		unsigned _currentColors = 0;
		float _scale;
		Vector2<bool> _mirror;
		Vector2f _speed;
		Vector2f _position;

	public:
		static constexpr unsigned char CLASS_ID = 11;

		Particle(Source source, const InitData &initData, const Character &owner, unsigned sprite, Vector2f position, bool compute = true);

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
		const Source &getSource() const;
		const Character &getOwner() const;
		int getLayer() const override;
	};
}


#endif //SOFGV_PARTICLE_HPP
