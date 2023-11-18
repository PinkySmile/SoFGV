//
// Created by PinkySmile on 16/11/23.
//

#include "ParticleGenerator.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	ParticleGenerator::ParticleGenerator(const ParticleGenerator::InitData &initData, const Character &owner) :
		_data(initData),
		_owner(owner)
	{
		this->_aliveTimer = random_distrib(game->battleRandom, initData.lifeSpan.first, initData.lifeSpan.second);
		this->_nextSpawnCost = random_distrib(game->battleRandom, initData.spawnInterval.first, initData.spawnInterval.second);
		this->_computePosition();
		for (auto &p : initData.particles)
			this->_maxSpawnRate += p.spawnChance;
	}

	void ParticleGenerator::_computePosition()
	{
		this->_position = this->_owner.getPosition();
		this->_position.x += this->_data.clipCenter.x * (this->_owner.getDirection() ? 1 : -1);
		this->_position.y += this->_data.clipCenter.y;
		if (this->_data.forceInStage) {
			if (this->_position.x < STAGE_X_MIN)
				this->_position.x = STAGE_X_MIN;
			else if (this->_position.x > STAGE_X_MAX)
				this->_position.x = STAGE_X_MAX;
			if (this->_position.y < STAGE_Y_MIN)
				this->_position.y = STAGE_Y_MIN;
			else if (this->_position.y > STAGE_Y_MAX)
				this->_position.y = STAGE_Y_MAX;
		}
	}

	void ParticleGenerator::render() const
	{
		if (this->showAttributes) {
			game->screen->borderColor(1, sf::Color::Black);
			game->screen->displayElement({
				static_cast<int>(this->_position.x + this->_data.spawnBoundary.pos.x),
				-static_cast<int>(this->_position.y + this->_data.spawnBoundary.pos.y),
				static_cast<int>(this->_data.spawnBoundary.size.x),
				static_cast<int>(this->_data.spawnBoundary.size.y)
			}, sf::Color{0, 0, 0, 0});
			game->screen->borderColor();
		}
		if (this->showBoxes) {
			sf::VertexArray arr{sf::Quads, 4};
			sf::VertexArray arr2{sf::LineStrip, 5};
			Rectangle box = {
				{this->_position.x - 4.5f, -this->_position.y - 4.5f},
				{this->_position.x + 4.5f, -this->_position.y - 4.5f},
				{this->_position.x + 4.5f, -this->_position.y + 4.5f},
				{this->_position.x - 4.5f, -this->_position.y + 4.5f},
			};

			for (int i = 0; i < 4; i++) {
				arr[i].color = sf::Color::Black;
				arr[i].color.a *= 0x30 / 255.f;
				arr[i].position = (&box.pt1)[i];
			}
			game->screen->draw(arr);

			for (unsigned i = 0; i < 5; i++) {
				arr2[i].color = sf::Color::Black;
				arr2[i].position = (&box.pt1)[i % 4];
			}
			game->screen->draw(arr2);
		}
	}

	void ParticleGenerator::update()
	{
		if (this->_aliveTimer <= 1 || (this->_data.disabledOnHit && this->_owner.isHit()))
			return this->kill();
		else
			this->_aliveTimer--;
		if (this->_data.stickToTarget)
			this->_computePosition();
		++this->_spawnCredit;
		while (this->_spawnCredit >= this->_nextSpawnCost) {
			this->_spawnCredit -= this->_nextSpawnCost;
			this->spawnParticle();
			this->_nextSpawnCost = random_distrib(game->battleRandom, this->_data.spawnInterval.first, this->_data.spawnInterval.second);
		}
	}

	void ParticleGenerator::spawnParticle()
	{
		if (this->_maxSpawnRate == 0)
			return;

		float val = random_distrib(game->battleRandom, 0.f, this->_maxSpawnRate);

		for (auto &p : this->_data.particles) {
			if (val <= p.spawnChance) {
				Vector2f pos{
					this->_position.x + this->_data.spawnBoundary.pos.x + random_distrib(game->battleRandom, 0.f, this->_data.spawnBoundary.size.x),
					this->_position.y + this->_data.spawnBoundary.pos.y - random_distrib(game->battleRandom, 0.f, this->_data.spawnBoundary.size.y)
				};

				game->battleMgr->registerObject<Particle>(true, p, this->_owner, this->_data.sprite, pos);
				return;
			}
			val -= p.spawnChance;
		}
		my_assert(false);
	}

	bool ParticleGenerator::isDead() const
	{
		return this->_aliveTimer == 0;
	}

	void ParticleGenerator::kill()
	{
		this->_aliveTimer = 0;
	}

	int ParticleGenerator::getLayer() const
	{
		return -500;
	}

	// TODO:
	unsigned int ParticleGenerator::getBufferSize() const
	{
		return 0;
	}

	// TODO:
	void ParticleGenerator::copyToBuffer(void *data) const
	{

	}

	// TODO:
	void ParticleGenerator::restoreFromBuffer(void *data)
	{

	}

	// TODO:
	size_t ParticleGenerator::printDifference(const char *msgStart, void *data1, void *data2, unsigned int startOffset) const
	{
		return 0;
	}

	// TODO:
	size_t ParticleGenerator::printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const
	{
		return 0;
	}

	// TODO:
	unsigned int ParticleGenerator::getClassId() const
	{
		return 0;
	}

	ParticleGenerator::InitData::InitData(nlohmann::json &data, const std::string &folder)
	{
		// TODO: Add proper error checking
		this->sprite = game->textureMgr.load(folder + "/" + data["sprite"].get<std::string>());
		this->spawnBoundary.pos.x = data["spawn_boundary"]["left"];
		this->spawnBoundary.pos.y = data["spawn_boundary"]["top"];
		this->spawnBoundary.size.x = data["spawn_boundary"]["width"];
		this->spawnBoundary.size.y = data["spawn_boundary"]["height"];
		this->clipCenter.x = data["clip_center"]["x"];
		this->clipCenter.y = data["clip_center"]["y"];
		this->stickToTarget = data["stick_to_target"];
		this->forceInStage = data["force_in_stage"];
		this->disabledOnHit = data["disabled_on_hit"];
		this->lifeSpan.first = data["life_span"][0];
		this->lifeSpan.second = data["life_span"][1];
		this->spawnInterval.first = data["spawn_interval"][0];
		this->spawnInterval.second = data["spawn_interval"][1];
		for (auto &color : data["colors"])
			this->colors.emplace_back(
				color["r"], color["g"], color["b"],
				static_cast<unsigned char>(color.contains("a") ? color["a"].get<unsigned char>() : 255)
			);
		this->particles.reserve(data["particles"].size());
		for (auto &particle : data["particles"])
			this->particles.emplace_back(particle, this->colors);
	}

	ParticleGenerator::InitData::~InitData()
	{
		game->textureMgr.remove(this->sprite);
	}
}