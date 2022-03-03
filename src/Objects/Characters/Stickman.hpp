//
// Created by Gegel85 on 01/03/2022.
//

#ifndef BATTLE_STICKMAN_HPP
#define BATTLE_STICKMAN_HPP


#include "../Character.hpp"
#include <random>

namespace Battle
{
	class Stickman : public Character {
	private:
#pragma pack(push, 1)
		struct Data {
			unsigned _addedOFlags;
			unsigned _addedDFlags;
			bool _flagsGenerated;
			unsigned _moveLength;
			unsigned _chargeTime;
		};
#pragma pack(pop)

		// Non Game State
		mutable FrameData _fakeFrameData;
		std::uniform_int_distribution<int> _dist{0, 25};

		// Game State
		OffensiveFlags _addedOFlags = {0};
		DefensiveFlags _addedDFlags = {0};
		bool _flagsGenerated = false;
		unsigned _moveLength = 0;
		unsigned _chargeTime = 0;

		void _decreaseMoveTime();

	protected:
		void _tickMove() override;
		void _applyMoveAttributes() override;
		void _onMoveEnd(const FrameData &lastData) override;
		void _applyNewAnimFlags() override;
		bool _canStartMove(unsigned int action, const FrameData &data) override;
		void _forceStartMove(unsigned int action) override;

	public:
		Stickman() = default;
		Stickman(const std::string &frameData, const std::string &subobjFrameData, const std::pair<std::vector<Color>, std::vector<Color>> &palette, std::shared_ptr<IInput> input);
		const FrameData *getCurrentFrameData() const override;
		unsigned int getClassId() const override;
		void update() override;

		unsigned int getBufferSize() const override;

		void copyToBuffer(void *data) const override;

		void restoreFromBuffer(void *data) override;
	};
}


#endif //BATTLE_STICKMAN_HPP
