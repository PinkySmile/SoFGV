//
// Created by PinkySmile on 18/02/2022.
//

#ifndef SOFGV_PRACTICEBATTLEMANAGER_HPP
#define SOFGV_PRACTICEBATTLEMANAGER_HPP


#include "BattleManager.hpp"

namespace SpiralOfFate
{
	class PracticeBattleManager : public BattleManager {
	private:
		struct GapElem {
			int timer;
			int gap;

			GapElem() = default;
			GapElem(int timer, int gap): timer(timer), gap(gap) {};
		};
		struct BlockingState {
			GapElem fa{240, 0};
			std::list<GapElem> gaps;
			unsigned gapTimer = 0;
			unsigned faTimer = 0;
			bool atk = false;
			bool blk = false;
			bool hasBlocked = false;
		};

		BlockingState _left;
		BlockingState _right;
		bool _showBoxes = false;
		float _time = 0;
		bool _step = false;
		bool _next = false;
		unsigned char *_savedState = nullptr;

		static bool _isBlocking(const Character &me);
		static bool _isAttacking(const Character &me);
		static bool _canMashNextFrame(const Character &me);
		static void _renderGapState(const BlockingState &state, Vector2f pos);
		static void _updateGapState(BlockingState &state, const Character &me, const Character &op);

		void _updateFrameStuff();
		void _displayFrameStuff() const;

	protected:
		bool _updateLoop() override;

	public:
		bool replay = false;

		PracticeBattleManager(const StageParams &stage, const CharacterParams &leftCharacter, const CharacterParams &rightCharacter);
		~PracticeBattleManager();
		void render() override;
		bool update() override;
		void consumeEvent(const sf::Event &event) override;

		friend class ReplayInGame;
		friend class PracticeInGame;
	};
}


#endif //SOFGV_PRACTICEBATTLEMANAGER_HPP
