//
// Created by PinkySmile on 15/08/2022.
//

#include <iostream>
#include "RollbackMachine.hpp"
#include "Resources/Game.hpp"

#define DIFF_TIME_NB_AVG 10
#define MAX_SETBACK 1000LL

std::vector<char> __frame;

namespace SpiralOfFate
{
	RollbackMachine::RollbackData::RollbackData(std::pair<IInput *, IInput *> inputs, std::pair<std::bitset<INPUT_NUMBER - 1> *, std::bitset<INPUT_NUMBER - 1> *> old)
	{
		this->regenInputs(inputs, old);
	}

	RollbackMachine::RollbackData::RollbackData(RollbackMachine::RollbackData &other) :
		clock(other.clock),
		left(other.left),
		right(other.right),
		dataSize(other.dataSize)
	{
		this->data = new char[this->dataSize];
		memcpy(this->data, other.data, this->dataSize);
	}

	RollbackMachine::RollbackData::~RollbackData()
	{
		delete[] this->data;
	}

	void RollbackMachine::RollbackData::regenInputs(std::pair<IInput *, IInput *> inputs, std::pair<std::bitset<INPUT_NUMBER - 1> *, std::bitset<INPUT_NUMBER - 1> *> old)
	{
		this->left.regenInputs(*inputs.first, old.first);
		this->right.regenInputs(*inputs.second, old.second);
	}

	void RollbackMachine::RollbackData::save(RollbackInput &l, RollbackInput &r)
	{
		this->left.save(l);
		this->right.save(r);
		this->dataSize = game->battleMgr->getBufferSize();
		delete[] this->data;
		this->data = new char[this->dataSize];
		game->battleMgr->copyToBuffer(this->data);
	}

	void RollbackMachine::InputData::regenInputs(IInput &input, std::bitset<INPUT_NUMBER - 1> *old)
	{
		if (input.hasInputs()) {
			input.update();
			for (int i = 0; i < INPUT_NUMBER - 1; ++i)
				this->keyStates[i] = input.isPressed(static_cast<InputEnum>(i));
			this->predicted = false;
		} else {
#if MAX_ROLLBACK == 0
			(void)old;
			throw AssertionFailedException("input.hasInput()", "");
#else
			my_assert(old);
			this->keyStates = *old;
			this->predicted = true;
#endif
		}
	}

	void RollbackMachine::InputData::save(RollbackInput &input)
	{
		this->keyDuration = input._keyDuration;
	}

	RollbackMachine::RollbackMachine(Character *left, Character *right) :
		_realInputLeft(left->_input),
		_realInputRight(right->_input)
	{
		auto onDesync = game->connection->onDesync;

		left->_input = this->inputLeft;
		right->_input = this->inputRight;
		if (!game->connection)
			return;
		game->connection->onInputReceived = [this](Connection::Remote &, unsigned frameId) {
			this->_onInputReceived(frameId);
		};
		game->connection->onTimeSync = [this](Connection::Remote &, unsigned, long long diff) {
			if (this->_opDiffTimes.size() >= DIFF_TIME_NB_AVG)
				this->_opDiffTimes.pop_front();
			this->_opDiffTimes.push_back(diff);
			this->_totalOpAvgDiffTimes += diff;
			this->_totalOpAvgDiffTimes /= (long long)this->_opDiffTimes.size();
			this->_opDiffTimesAverage.push_back(this->_totalOpAvgDiffTimes);
		};
		game->connection->onDesync = [onDesync](Connection::Remote &remote, unsigned frameId, unsigned cpuSum, unsigned recvSum) {
			if (onDesync)
				onDesync(remote, frameId, cpuSum, recvSum);
			std::filesystem::create_directory("frames");

			std::ofstream stream{"frames/frames-" + std::to_string(BattleManager::getFrame(__frame.data())) + ".frame"};

			stream.write(__frame.data(), __frame.size());
		};
		for (unsigned m = game->connection->getCurrentDelay(), i = 0; i < m; i++)
			game->connection->timeSync(0, i);
	}

	RollbackMachine::~RollbackMachine()
	{
		if (!game->connection)
			return;
		game->connection->onInputReceived = nullptr;
		game->connection->onTimeSync = nullptr;
	}

	void RollbackMachine::_onInputReceived(unsigned int frame)
	{
		if (this->_savedData.empty()) {
			this->_advanceInputs.emplace_back();
			return;
		}
		frame += game->connection->getCurrentDelay();

		auto lowFrame = BattleManager::getFrame(this->_savedData.front().data);
		auto highFrame = BattleManager::getFrame(this->_savedData.back().data);

		if (frame < lowFrame)
			return game->logger.error(std::to_string(frame) + " < " + std::to_string(lowFrame));
		if (frame > highFrame) {
			this->_advanceInputs.emplace_back();
			return;
		}

		auto it = this->_savedData.begin();

		while (frame != BattleManager::getFrame(it->data))
			it++;

		auto time = it->clock.getElapsedTime().asMicroseconds();

		this->_diffTimes.push_back(time);
		this->_totalAvgDiffTimes += time;
		this->_totalAvgDiffTimes /= (long long)this->_diffTimes.size();
		this->_diffTimesAverage.push_back(this->_totalAvgDiffTimes);
		if (this->_diffTimes.size() >= DIFF_TIME_NB_AVG)
			this->_diffTimes.pop_front();
		game->connection->timeSync(time, frame);
	}

	RollbackMachine::UpdateStatus RollbackMachine::update(bool useP1Inputs, bool useP2Inputs)
	{
		bool hasInitTimer = false;

		//TODO: Use useP1Inputs, useP2Inputs and check the fake pause
		(void)useP1Inputs;
		(void)useP2Inputs;
	loopStart:
#if MAX_ROLLBACK == 0
		if (!this->_realInputLeft->hasInputs() || !this->_realInputRight->hasInputs()) {
#else
		if (this->_savedData.size() == MAX_ROLLBACK && (
			(!this->_realInputLeft->hasInputs() && this->_savedData.front().left.predicted) ||
			(!this->_realInputRight->hasInputs() && this->_savedData.front().right.predicted)
		)) {
#endif
			game->logger.verbose("Skipping 1 frame!");
			return UPDATESTATUS_NO_INPUTS;
		}
		if (!hasInitTimer)
			this->_frameTimer += 1000000 / 60;
		hasInitTimer = true;

		long long timeResultOp = 0;
		long long timeResult = 0;
		long long nbs = 0;

		while (!this->_opDiffTimesAverage.empty() && !this->_diffTimesAverage.empty()) {
			timeResultOp += this->_opDiffTimesAverage.front();
			timeResult += this->_diffTimesAverage.front();
			this->_opDiffTimesAverage.pop_front();
			this->_diffTimesAverage.pop_front();
			nbs++;
		}
		if (nbs)
			this->_lastAvgTimes = {timeResultOp / nbs, timeResult / nbs};
		this->_frameTimer += std::min(std::max((this->_lastAvgTimes.first - this->_lastAvgTimes.second) / 4, -MAX_SETBACK), MAX_SETBACK);
		if (this->_frameTimer < 1000000 / 60)
			return UPDATESTATUS_NO_INPUTS;

#if MAX_ROLLBACK == 0
		this->_savedData.emplace_back(
			std::pair<IInput *, IInput *>{&*this->_realInputLeft, &*this->_realInputRight},
			std::pair<std::bitset<INPUT_NUMBER - 1> *, std::bitset<INPUT_NUMBER - 1> *>{&this->inputLeft->_keyStates, &this->inputRight->_keyStates}
		);

		bool result = this->_simulateFrame(this->_savedData.back(), false);

		this->_savedData.pop_back();
#else
		bool result = this->_checkPredictedInputs();

		if (result) {
			if (!this->_advanceInputs.empty()) {
				auto frame = (this->_savedData.empty() ? 0 : BattleManager::getFrame(this->_savedData.back().data) + 1);

				if (frame >= game->connection->getCurrentDelay()) {
					auto time = this->_advanceInputs.front().getElapsedTime().asMicroseconds();

					my_assert(this->_savedData.empty() || (!this->_savedData.back().left.predicted && !this->_savedData.back().right.predicted));
					this->_advanceInputs.pop_front();
					this->_diffTimes.push_back(time);
					this->_totalAvgDiffTimes += time;
					this->_totalAvgDiffTimes /= (long long)this->_diffTimes.size();
					this->_diffTimesAverage.push_back(this->_totalAvgDiffTimes);
					if (this->_diffTimes.size() >= DIFF_TIME_NB_AVG)
						this->_diffTimes.pop_front();
					game->connection->timeSync(-time, frame);
				}
			}
			this->_savedData.emplace_back(
				std::pair<IInput *, IInput *>{&*this->_realInputLeft, &*this->_realInputRight},
				std::pair<std::bitset<INPUT_NUMBER - 1> *, std::bitset<INPUT_NUMBER - 1> *>{&this->inputLeft->_keyStates, &this->inputRight->_keyStates}
			);
			result = this->_simulateFrame(this->_savedData.back(), true);
		}
		while (this->_savedData.size() > 1 && !this->_savedData.front().left.predicted && !this->_savedData.front().right.predicted) {
			auto &dat = this->_savedData.front();
			auto frameId = BattleManager::getFrame(dat.data);
			//std::ofstream stream{"frames/frame-" + std::to_string(frameId) + ".frame"};

			//stream.write((char *)dat.data, dat.dataSize);
			if (!game->connection || frameId % 60 != 0) {
				this->_savedData.pop_front();
				continue;
			}
			game->connection->reportChecksum(_computeCheckSum((short *)dat.data, dat.dataSize / sizeof(short)), frameId);
			__frame.resize(dat.dataSize);
			memcpy(__frame.data(), dat.data, dat.dataSize);
			this->_savedData.pop_front();
		}
#endif
		this->_frameTimer -= 1000000 / 60;
		if (this->_frameTimer >= 1000000 / 60)
			goto loopStart;
		return result ? UPDATESTATUS_OK : UPDATESTATUS_GAME_ENDED;
	}

	RollbackMachine::UpdateStatus RollbackMachine::syncTestUpdate(bool useP1Inputs, bool useP2Inputs)
	{
		//TODO: Use useP1Inputs, useP2Inputs and check the fake pause
		(void)useP1Inputs;
		(void)useP2Inputs;

		unsigned int dataSizeBefore = game->battleMgr->getBufferSize();
		unsigned int dataSizeAfter;
		unsigned int dataSizeAfter2;
		char *dataBefore = new char[dataSizeBefore];
		char *dataAfter;
		char *dataAfter2;
		int checksum1;
		int checksum2;
		auto lDur = this->inputLeft->_keyDuration;
		auto rDur = this->inputRight->_keyDuration;

		game->battleMgr->copyToBuffer(dataBefore);
		this->_realInputLeft->update();
		for (int i = 0; i < INPUT_NUMBER - 1; ++i)
			this->inputLeft->_keyStates[i] = this->_realInputLeft->isPressed(static_cast<InputEnum>(i));
		this->_realInputRight->update();
		for (int i = 0; i < INPUT_NUMBER - 1; ++i)
			this->inputRight->_keyStates[i] = this->_realInputRight->isPressed(static_cast<InputEnum>(i));

		auto result1 = game->battleMgr->update();
		auto lDur2 = this->inputLeft->_keyDuration;
		auto rDur2 = this->inputRight->_keyDuration;

		dataSizeAfter = game->battleMgr->getBufferSize();
		dataAfter = new char[dataSizeAfter];
		memset(dataAfter, 0xCD, dataSizeAfter);
		game->battleMgr->copyToBuffer(dataAfter);
		checksum1 = _computeCheckSum((short *)dataAfter, dataSizeAfter / sizeof(short));

		game->battleMgr->restoreFromBuffer(dataBefore);
		this->inputLeft->_keyDuration = lDur;
		this->inputRight->_keyDuration = rDur;

		auto result2 = game->battleMgr->update();

		dataSizeAfter2 = game->battleMgr->getBufferSize();
		dataAfter2 = new char[dataSizeAfter2];
		memset(dataAfter2, 0xDC, dataSizeAfter2);
		game->battleMgr->copyToBuffer(dataAfter2);
		checksum2 = _computeCheckSum((short *)dataAfter2, dataSizeAfter2 / sizeof(short));

		if (checksum1 != checksum2) {
			game->logger.fatal("RollbackMachine::debugRollback: Checksum mismatch");
			game->logger.fatal("Old checksum: " + std::to_string(checksum1) + " vs new checksum: " + std::to_string(checksum2));
			if (dataSizeAfter != dataSizeAfter2)
				game->logger.fatal("Old data size: " + std::to_string(dataSizeAfter) + " vs new data size: " + std::to_string(dataSizeAfter2));
			else
				for (size_t i = 0; i < dataSizeAfter; i++)
					if (((char *)dataAfter)[i] != ((char *)dataAfter2)[i])
						game->logger.fatal("Old data at index " + std::to_string(i) + ": " + std::to_string(((char *)dataAfter)[i]) + " vs new data at index " + std::to_string(i) + ": " + std::to_string(((char *)dataAfter2)[i]));
			game->battleMgr->logDifference(dataAfter, dataAfter2);
			delete[] dataBefore;
			delete[] dataAfter;
			delete[] dataAfter2;
			throw AssertionFailedException(
				"checksum1 == checksum2",
				std::to_string(checksum1) + " != " + std::to_string(checksum2)
			);
		}
		my_assert(result1 == result2);
		my_assert(lDur2 == this->inputLeft->_keyDuration);
		my_assert(rDur2 == this->inputRight->_keyDuration);
		delete[] dataBefore;
		delete[] dataAfter;
		delete[] dataAfter2;
		return result1 ? UPDATESTATUS_OK : UPDATESTATUS_GAME_ENDED;
	}

	bool RollbackMachine::_checkPredictedInputs()
	{
		if (this->_savedData.empty())
			return true;

		auto it = this->_savedData.begin();
		auto result = 0;
		bool different = false;
		RollbackData *old = nullptr;

		while (true) {
			auto &input = *it;

			// If neither inputs are predicted, no need to do anything
			if (!input.left.predicted && !input.right.predicted)
				goto endLoop;

			// Here, at least one input is predicted, let's see if we were wrong
			if (input.left.predicted) {
				if (!this->_realInputLeft->hasInputs())
					return true;

				auto o = input.left.keyStates;

				input.left.regenInputs(*this->_realInputLeft, nullptr);
				different = o != input.left.keyStates;
			}
			if (input.right.predicted) {
				if (!this->_realInputRight->hasInputs())
					return true;

				auto o = input.right.keyStates;

				input.right.regenInputs(*this->_realInputRight, nullptr);
				different |= o != input.right.keyStates;
			}
			if (different)
				break;
			// We weren't! Good job!
		endLoop:
			it++;
			// All inputs have been processed, bail out
			if (it == this->_savedData.end())
				return true;
		}
		if (it == this->_savedData.end())
			return true;

		bool save = false;

		game->battleMgr->restoreFromBuffer(it->data);
		this->inputLeft->_keyDuration = it->left.keyDuration;
		this->inputRight->_keyDuration = it->right.keyDuration;
		while (it != this->_savedData.end()) {
			if (it->left.predicted)
				it->left.regenInputs(*this->_realInputLeft, old ? &old->left.keyStates : nullptr);
			if (it->right.predicted)
				it->right.regenInputs(*this->_realInputRight, old ? &old->right.keyStates : nullptr);
			result++;
			if (!this->_simulateFrame(*it, save)) {
				game->logger.debug("Rolled back " + std::to_string(result) + " frames (hit the end prematurely)");
				return false;
			}
			save = true;
			old = &*it;
			it++;
		}
		game->logger.debug("Rolled back " + std::to_string(result) + " frames");
		return true;
	}

	/*
	 * Simple checksum function from wikipedia:
	 *   http://en.wikipedia.org/wiki/Fletcher%27s_checksum
	 */
	int RollbackMachine::_computeCheckSum(short *data, size_t len)
	{
		int sum1 = 0xffff, sum2 = 0xffff;

		while (len) {
			size_t tlen = len > 360 ? 360 : len;
			len -= tlen;
			do {
				sum1 += *data++;
				sum2 += sum1;
			} while (--tlen);
			sum1 = (sum1 & 0xffff) + (sum1 >> 16);
			sum2 = (sum2 & 0xffff) + (sum2 >> 16);
		}

		/* Second reduction step to reduce sums to 16 bits */
		sum1 = (sum1 & 0xffff) + (sum1 >> 16);
		sum2 = (sum2 & 0xffff) + (sum2 >> 16);
		return sum2 << 16 | sum1;
	}

	void RollbackMachine::consumeEvent(const sf::Event &event)
	{
		this->_realInputLeft->consumeEvent(event);
		this->_realInputRight->consumeEvent(event);
	}

	size_t RollbackMachine::getBufferSize() const
	{
		if (this->_savedData.empty())
			return 0;
		return this->_savedData.size() - (!this->_savedData.front().left.predicted && !this->_savedData.front().right.predicted);
	}

	size_t RollbackMachine::getMaxBufferSize() const
	{
		return MAX_ROLLBACK;
	}

	bool RollbackMachine::_simulateFrame(RollbackData &data, bool saveState)
	{
		if (saveState)
			data.save(*this->inputLeft, *this->inputRight);
		this->inputLeft->_keyStates = data.left.keyStates;
		this->inputRight->_keyStates = data.right.keyStates;
		return game->battleMgr->update();
	}

	const std::pair<long long int, long long int> &RollbackMachine::getLastAvgTimes() const
	{
		return this->_lastAvgTimes;
	}

	std::pair<long long int, long long int> RollbackMachine::getLastTimes() const
	{
		std::pair<long long int, long long int> times{0, 0};

		if (!this->_opDiffTimes.empty())
			times.first = this->_opDiffTimes.front();
		if (!this->_diffTimes.empty())
			times.second = this->_diffTimes.front();
		return times;
	}
}
