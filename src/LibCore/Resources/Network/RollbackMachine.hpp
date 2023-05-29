//
// Created by PinkySmile on 15/08/2022.
//

#ifndef SOFGV_ROLLBACKMACHINE_HPP
#define SOFGV_ROLLBACKMACHINE_HPP


#include <list>
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
			InputData left;
			InputData right;
			size_t dataSize = 0;
			void *data = nullptr;

			RollbackData() = default;
			RollbackData(std::pair<IInput *, IInput *> inputs, std::pair<std::bitset<INPUT_NUMBER - 1> *, std::bitset<INPUT_NUMBER - 1> *> old);
			RollbackData(RollbackData &);
			~RollbackData();

			void regenInputs(std::pair<IInput *, IInput *> inputs, std::pair<std::bitset<INPUT_NUMBER - 1> *, std::bitset<INPUT_NUMBER - 1> *> old);
			void save(RollbackInput &left, RollbackInput &right);
		};

		std::list<RollbackData> _savedData;
		std::shared_ptr<IInput> _realInputLeft;
		std::shared_ptr<IInput> _realInputRight;
		std::shared_ptr<RollbackInput> inputLeft = std::make_shared<RollbackInput>();
		std::shared_ptr<RollbackInput> inputRight = std::make_shared<RollbackInput>();

		bool _simulateFrame(RollbackData &data, bool saveState);
		bool _checkPredictedInputs();
		static int _computeCheckSum(short *data, size_t size);

	public:
		enum UpdateStatus {
			UPDATESTATUS_GAME_ENDED,
			UPDATESTATUS_OK,
			UPDATESTATUS_NO_INPUTS,
		};

		RollbackMachine(Character *left, Character *right);
		UpdateStatus update(bool useP1Inputs, bool useP2Inputs);
		UpdateStatus syncTestUpdate(bool useP1Inputs, bool useP2Inputs);
		void consumeEvent(const sf::Event &event);
		size_t getBufferSize() const;
		size_t getMaxBufferSize() const;
	};
}


#endif //SOFGV_ROLLBACKMACHINE_HPP
