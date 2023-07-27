//
// Created by PinkySmile on 26/02/23.
//

#ifndef SOFGV_VICTORIASTAR_HPP
#define SOFGV_VICTORIASTAR_HPP


#include "Butterfly.hpp"
#include "Objects/Character.hpp"
#include "Flower.hpp"

#define NB_BUTTERFLIES 8
#define BUTTERFLIES_START_ID 1000
#define SHADOW_PER_STACK 3
#define MAX_STACKS 15
#define START_STACKS (5 * SHADOW_PER_STACK)
#define RC_COST 3

namespace SpiralOfFate
{
	class Shadow;

	class VictoriaStar : public Character {
	private:
#pragma pack(push, 1)
		struct Data {
			bool _hitShadow;
			unsigned _stacks;
			unsigned _flower;
			unsigned _nbShadows;
			unsigned _objects[0];
		};
		static_assert(sizeof(Data) == 13, "Data has wrong size");
#pragma pack(pop)

		// Game state
		bool _hitShadow = false;
		unsigned _stacks = START_STACKS;
		std::vector<std::pair<unsigned, std::shared_ptr<Shadow>>> _shadows;
		std::optional<std::pair<unsigned, std::shared_ptr<Flower>>> _flower;

		// Non-game state
		const Object *_target = nullptr;
		Sprite _hudBack;
		Sprite _hudFull;
		Sprite _hudPart;
		std::vector<std::vector<FrameData>> _shadowActions;
		std::array<std::pair<unsigned, Butterfly *>, NB_BUTTERFLIES> _happyBufferFlies;
		std::array<std::pair<unsigned, Butterfly *>, NB_BUTTERFLIES> _weirdBufferFlies;

		Vector2f _getButterflyAttackPos(unsigned id);

	protected:
		bool _startMove(unsigned int action) override;
		std::pair<unsigned int, std::shared_ptr<IObject>> _spawnSubObject(BattleManager &manager, unsigned int id, bool needRegister) override;
		bool _canCancel(unsigned int action) override;
		void _forceStartMove(unsigned int action) override;
		void _blockMove(Object *other, const FrameData &data) override;
		void _applyNewAnimFlags() override;
		bool _canStartMove(unsigned int action, const FrameData &data) override;
		void _tickMove() override;

	public:
		VictoriaStar() = default;
		VictoriaStar(
			unsigned index,
			const std::string &folder,
			const std::pair<std::vector<Color>, std::vector<Color>> &palette,
			std::shared_ptr<IInput> input,
			const std::string &opName
		);
		~VictoriaStar() override;

		unsigned int getClassId() const override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		void getHit(IObject &other, const FrameData *data) override;
		bool hits(const IObject &other) const override;
		void hit(IObject &other, const FrameData *data) override;
		void init(BattleManager &manager, const InitData &data) override;
		void resolveSubObjects(const BattleManager &manager) override;
		size_t printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const override;
		size_t printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const override;
		void postUpdate() override;
		void drawSpecialHUD(sf::RenderTarget &texture) override;

		friend class Flower;
		friend class Butterfly;
		friend class VoidShadow;
	};
}


#endif //SOFGV_VICTORIASTAR_HPP
