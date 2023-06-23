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
		enum BuffType {
			BUFFTYPE_GUARD_BAR_IGNORED, //guard bar ignored
			BUFFTYPE_INSTA_CRUSH,       //wb = crush
			BUFFTYPE_PLUS_DAMAGE,       //+ damage
			BUFFTYPE_MINUS_DAMAGE,      //- damage
			BUFFTYPE_PLUS_HITSTUN,      //+ hitstun
			BUFFTYPE_MINUS_HITSTUN,     //- hitstun
			BUFFTYPE_MINUS_MANA_COST,   //- mana cost
			BUFFTYPE_PLUS_MANA_COST,    //+ mana cost
			BUFFTYPE_PLUS_PRORATION,    //+ proration
			BUFFTYPE_PLUS_SPEED,        //+ speed
			BUFFTYPE_MINUS_SPEED,       //- speed
		};
		constexpr static const std::array<const char *, 11> buffName = {
			"GUARD_BAR_IGNORED",
			"INSTA_CRUSH",
			"PLUS_DAMAGE",
			"MINUS_DAMAGE",
			"PLUS_HITSTUN",
			"MINUS_HITSTUN",
			"MINUS_MANA_COST",
			"PLUS_MANA_COST",
			"PLUS_PRORATION",
			"PLUS_SPEED",
			"MINUS_SPEED"
		};
		constexpr static const std::array<int, 11> timers = {
			/*  0 */ 2 * 120, //guard bar ignored
			/*  1 */ 2 * 120, //wb = crush
			/*  2 */ 2 * 300, //+ damage
			/*  3 */ 2 * 300, //- damage
			/*  4 */ 2 * 90,  //+ hitstun
			/*  5 */ 2 * 90,  //- hitstun
			/*  6 */ 2 * 240, //- mana cost
			/*  7 */ 2 * 240, //+ mana cost
			/*  8 */ 2 * 120, //+ proration
			/*  9 */ 2 * 360, //+ speed
			/* 10 */ 2 * 360, //- speed
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

		// Game State
		unsigned char _buff = 0;
		unsigned _time = 0;
		unsigned _oldAction = 0;
		unsigned _buffTimer = 0;

	protected:
		void _applyMoveAttributes() override;
		void _onMoveEnd(const FrameData &lastData) override;
		void _forceStartMove(unsigned int action) override;
		void _mutateHitFramedata(FrameData &framedata) const override;
		void _allyBuffEffect(FrameData &framedata) const;
		void _enemyBuffEffect(FrameData &framedata) const;
		bool _canStartMove(unsigned int action, const FrameData &data) override;
		void _computeFrameDataCache() override;

	public:
		Stickman() = default;
		Stickman(unsigned index, const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette, std::shared_ptr<IInput> input);
		unsigned int getClassId() const override;
		void update() override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *pVoid, void *pVoid1, unsigned startOffset) const override;
		bool matchEndUpdate() override;
		void onMatchEnd() override;
		void render() const override;
	};
}


#endif //SOFGV_STICKMAN_HPP
