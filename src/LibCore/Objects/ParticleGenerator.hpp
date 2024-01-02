//
// Created by PinkySmile on 16/11/23.
//

#ifndef SOFGV_PARTICLEGENERATOR_HPP
#define SOFGV_PARTICLEGENERATOR_HPP


#include "IObject.hpp"
#include "Particle.hpp"

namespace SpiralOfFate
{
	class Character;

	class ParticleGenerator : public IObject {
	public:
		struct InitData {
			unsigned sprite;
			std::vector<Color> colors;
			Box spawnBoundary;
			Vector2i clipCenter;
			bool stickToTarget;
			bool forceInStage;
			bool disabledOnHit;
			std::pair<unsigned, unsigned> lifeSpan;
			std::pair<float, float> spawnInterval;
			std::vector<Particle::InitData> particles;

			InitData(const nlohmann::json &data, const std::string &folder);
			~InitData();
		};
		typedef std::tuple<unsigned char, unsigned> Source;

	protected:
		struct Data {
			Vector2f _position;
			unsigned _aliveTimer;
			float _maxSpawnRate;
			float _nextSpawnCost;
			float _spawnCredit;
		};
		static_assert(sizeof(Data) == 24, "Data has wrong size");

		// Non-Game State
		const InitData &_data;
		Source _source;
		const Character &_owner;
		const Character &_target;

		// Game State
		Vector2f _position;
		unsigned _aliveTimer;
		float _maxSpawnRate = 0;
		float _nextSpawnCost;
		float _spawnCredit = 0;

		void _computePosition();

	public:
		ParticleGenerator(Source source, const InitData &initData, const Character &owner, const Character &target, bool compute = true);
		ParticleGenerator(const ParticleGenerator &generator) = delete;
		ParticleGenerator &operator=(ParticleGenerator &generator) = delete;

		void render() const override;
		void update() override;
		bool isDead() const override;
		void kill() override;
		void spawnParticle();
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *data1, void *data2, unsigned int startOffset) const override;
		size_t printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const override;
		unsigned int getClassId() const override;
		const Source &getSource() const;
		const Character &getOwner() const;
		const Character &getTarget() const;
		int getLayer() const override;

		friend class BattleManager;
	};
}


#endif //SOFGV_PARTICLEGENERATOR_HPP
