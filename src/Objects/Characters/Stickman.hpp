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
		std::uniform_int_distribution<int> _dist{0, 25};
		OffensiveFlags _addedOFlags = {0};
		DefensiveFlags _addedDFlags = {0};
		mutable FrameData _fakeFrameData;
		bool _flagsGenerated = false;
		unsigned _moveLength = 0;
		unsigned _chargeTime = 0;

		void _decreaseMoveTime();

	protected:
		void _tickMove() override;

	public:
		Stickman() = default;
		Stickman(const std::string &frameData, const std::string &subobjFrameData, const std::pair<std::vector<Color>, std::vector<Color>> &palette, std::shared_ptr<IInput> input);
		const FrameData *getCurrentFrameData() const override;
		unsigned int getClassId() const override;
	};
}


#endif //BATTLE_STICKMAN_HPP
