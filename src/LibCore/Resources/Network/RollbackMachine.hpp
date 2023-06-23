//
// Created by PinkySmile on 15/08/2022.
//

#ifndef SOFGV_ROLLBACKMACHINE_HPP
#define SOFGV_ROLLBACKMACHINE_HPP


#include <list>
#include <deque>
#include <memory>
#include <optional>
#include "Inputs/RollbackInput.hpp"
#include "Objects/Character.hpp"

#define MAX_ROLLBACK 8

namespace SpiralOfFate
{
	class RollbackMachine {
	private:
		struct InputData {
			bool predicted = false;
			std::bitset<INPUT_NUMBER - 1> keyStates;
			std::array<int, INPUT_NUMBER - 1> keyDuration;

			InputData() = default;

			void regenInputs(IInput &input, std::bitset<INPUT_NUMBER - 1> *old);
			void save(RollbackInput &input);
		};

		struct RollbackData {
			sf::Clock clock;
			InputData left;
			InputData right;
			size_t dataSize = 0;
			char *data = nullptr;

			RollbackData() = default;
			RollbackData(std::pair<IInput *, IInput *> inputs, std::pair<std::bitset<INPUT_NUMBER - 1> *, std::bitset<INPUT_NUMBER - 1> *> old);
			RollbackData(RollbackData &);
			~RollbackData();

			void regenInputs(std::pair<IInput *, IInput *> inputs, std::pair<std::bitset<INPUT_NUMBER - 1> *, std::bitset<INPUT_NUMBER - 1> *> old);
			void save(RollbackInput &left, RollbackInput &right);
		};

		long long _frameTimer = 0;
		long long _totalAvgDiffTimes = 0;
		long long _totalOpAvgDiffTimes = 0;
		std::deque<long long> _diffTimes;
		std::deque<long long> _opDiffTimes;
		std::deque<long long> _diffTimesAverage;
		std::deque<long long> _opDiffTimesAverage;
		std::pair<long long, long long> _lastAvgTimes{0, 0};
		std::list<RollbackData> _savedData;
		std::list<sf::Clock> _advanceInputs;
		std::shared_ptr<IInput> _realInputLeft;
		std::shared_ptr<IInput> _realInputRight;
		std::shared_ptr<RollbackInput> inputLeft = std::make_shared<RollbackInput>();
		std::shared_ptr<RollbackInput> inputRight = std::make_shared<RollbackInput>();

		bool _simulateFrame(RollbackData &data, bool saveState);
		bool _checkPredictedInputs();
		void _onInputReceived(unsigned frame);
		static int _computeCheckSum(short *data, size_t size);

	public:
		enum UpdateStatus {
			UPDATESTATUS_GAME_ENDED,
			UPDATESTATUS_OK,
			UPDATESTATUS_NO_INPUTS,
		};

		RollbackMachine(Character *left, Character *right);
		~RollbackMachine();
		UpdateStatus update(bool useP1Inputs, bool useP2Inputs);
		UpdateStatus syncTestUpdate(bool useP1Inputs, bool useP2Inputs);
		void consumeEvent(const sf::Event &event);
		std::pair<long long int, long long int> getLastTimes() const;
		const std::pair<long long int, long long int> &getLastAvgTimes() const;
		size_t getBufferSize() const;
		size_t getMaxBufferSize() const;
	};
}


#endif //SOFGV_ROLLBACKMACHINE_HPP
