//
// Created by PinkySmile on 01/03/2022.
//

#ifndef SOFGV_STICKMAN_HPP
#define SOFGV_STICKMAN_HPP


#include <random>
#include "Objects/Character.hpp"

namespace SpiralOfFate
{
	class Stickman : public Character {
	private:
		constexpr static const std::array<std::pair<int, int>, 11> timers = {
			/*  0 */ std::pair<int, int>{120, 900}, //guard bar ignored
			/*  1 */ std::pair<int, int>{120, 900}, //wb = crush
			/*  2 */ std::pair<int, int>{300, 900}, //+ damage
			/*  3 */ std::pair<int, int>{300, 900}, //- damage
			/*  4 */ std::pair<int, int>{90,  900}, //+ hitstun
			/*  5 */ std::pair<int, int>{90,  900}, //- hitstun
			/*  6 */ std::pair<int, int>{240, 900}, //- mana cost
			/*  7 */ std::pair<int, int>{240, 900}, //+ mana cost
			/*  8 */ std::pair<int, int>{120, 900}, //+ proration
			/*  9 */ std::pair<int, int>{360, 900}, //+ speed
			/* 10 */ std::pair<int, int>{360, 900}, //- speed
			/* 11 */ //{300, 900}, //+ gros
		};

#pragma pack(push, 1)
		struct Data {
			unsigned char _buff = 0;
			unsigned _time = 0;
			unsigned _oldAction = 0;
			unsigned _buffTimer = 0;
		};
#pragma pack(pop)

		// Non Game State
		mutable FrameData _fakeFrameData;
		std::uniform_int_distribution<unsigned> _rand{0, timers.size() - 1};
		std::uniform_int_distribution<unsigned> _dist{0, 99};

		// Game State
		unsigned char _buff = 0;
		unsigned _time = 0;
		unsigned _oldAction = 0;
		unsigned _buffTimer = 0;

	protected:
		void _applyMoveAttributes() override;
		void _onMoveEnd(const FrameData &lastData) override;
		void _applyNewAnimFlags() override;
		bool _canStartMove(unsigned int action, const FrameData &data) override;
		void _forceStartMove(unsigned int action) override;
		void _mutateHitFramedata(FrameData &framedata) const override;
		void _allyBuffEffect(FrameData &framedata) const;
		void _enemyBuffEffect(FrameData &framedata) const;
		std::pair<unsigned int, std::shared_ptr<IObject>> _spawnSubobject(unsigned int id, bool needRegister) override;

	public:
		Stickman() = default;
		Stickman(unsigned index, const std::string &frameData, const std::string &subobjFrameData, const std::pair<std::vector<Color>, std::vector<Color>> &palette, std::shared_ptr<IInput> input);
		const FrameData *getCurrentFrameData() const override;
		unsigned int getClassId() const override;
		void update() override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *pVoid, void *pVoid1) const override;
		bool matchEndUpdate() override;
		void onMatchEnd() override;
	};
}


#endif //SOFGV_STICKMAN_HPP
