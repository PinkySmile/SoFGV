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
			bool predicted;
			std::bitset<INPUT_NUMBER - 1> keyStates;
			std::array<int, INPUT_NUMBER - 1> keyDuration;

			InputData() = default;
			InputData(IInput &input, InputData *old);
		};

		struct RollbackData {
			InputData left;
			InputData right;
			size_t dataSize;
			void *data = nullptr;

			RollbackData();
			RollbackData(RollbackData &);
			RollbackData(IInput &left, IInput &right, RollbackData *old);
			~RollbackData();
		};

		std::list<RollbackData> _savedData;
		std::shared_ptr<IInput> _realInputLeft;
		std::shared_ptr<IInput> _realInputRight;
		std::shared_ptr<RollbackInput> inputLeft = std::make_shared<RollbackInput>();
		std::shared_ptr<RollbackInput> inputRight = std::make_shared<RollbackInput>();

		bool _simulateFrame(const RollbackData &data);
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
		void debugRollback();
		void consumeEvent(const sf::Event &event);
		size_t getBufferSize() const;
		size_t getMaxBufferSize() const;
	};
}


#endif //SOFGV_ROLLBACKMACHINE_HPP
