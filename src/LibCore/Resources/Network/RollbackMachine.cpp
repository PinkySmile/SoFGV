//
// Created by PinkySmile on 15/08/2022.
//

#include <filesystem>
#include <iostream>
#include "RollbackMachine.hpp"
#include "Resources/Game.hpp"
#include "Utils.hpp"

#define DIFF_TIME_NB_AVG 10
#define MAX_SETBACK 1000LL
#define MAX_ROLLBACK 15
#ifdef _DEBUG
#define CHECKSUM_CHECK_INTERVAL 1
#else
#define CHECKSUM_CHECK_INTERVAL 60
#endif

static std::list<std::pair<size_t, std::vector<char>>> g_framesList;

namespace SpiralOfFate
{
	RollbackMachine::RollbackData::RollbackData(const std::pair<IInput *, IInput *> &inputs, const std::pair<std::bitset<INPUT_NUMBER - 1> *, std::bitset<INPUT_NUMBER - 1> *> &old)
	{
		this->regenInputs(inputs, old);
	}

	RollbackMachine::RollbackData::RollbackData(const RollbackData &other) :
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

	void RollbackMachine::RollbackData::regenInputs(const std::pair<IInput *, IInput *> &inputs, const std::pair<std::bitset<INPUT_NUMBER - 1> *, std::bitset<INPUT_NUMBER - 1> *> &old)
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

	void RollbackMachine::InputData::regenInputs(IInput &input, const std::bitset<INPUT_NUMBER - 1> *old)
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
			assert_exp(old);
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
			this->_totalOpAvgDiffTimes /= static_cast<long long>(this->_opDiffTimes.size());
			this->_opDiffTimesAverage.push_back(this->_totalOpAvgDiffTimes);
		};
		game->connection->onDesync = [](Connection::Remote &, unsigned frameId, unsigned, unsigned) {
			try {
				std::filesystem::create_directory("desync-frames");
			} catch (std::exception &e) {
				game->logger.error("Cannot save frame " + std::to_string(frameId) + ": " + e.what());
				return;
			}

			auto path = "desync-frames/frames-" + std::to_string(frameId) + ".sofgv_frame";
			std::ofstream stream{path, std::ofstream::binary};

			if (!stream) {
				game->logger.error("Cannot open " + path + " for writing: " + strerror(errno));
				return;
			}
			for (auto &frame : g_framesList)
				if (frame.first == frameId) {
					game->logger.info("Saving frame " + std::to_string(frameId) + ": Size " + std::to_string(frame.second.size()));
					stream.write(frame.second.data(), frame.second.size());
					if (stream)
						return;
					game->logger.error("Cannot write bytes to " + path + ": " + strerror(errno));
					return;
				}
			game->logger.fatal("Frame " + std::to_string(frameId) + " in no longer in the list!");
		};
		for (unsigned m = game->connection->getCurrentDelay(), i = 0; i < m; i++)
			game->connection->timeSync(0, i);
	}

	RollbackMachine::~RollbackMachine()
	{
		g_framesList.clear();
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
			++it;

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

					assert_exp(this->_savedData.empty() || (!this->_savedData.back().left.predicted && !this->_savedData.back().right.predicted));
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

			if (!game->connection || frameId % CHECKSUM_CHECK_INTERVAL != 0) {
				this->_savedData.pop_front();
				continue;
			}
			if (g_framesList.size() > 60)
				g_framesList.pop_front();
			g_framesList.emplace_back();
			g_framesList.back().first = frameId;
			g_framesList.back().second.resize(dat.dataSize);
			memcpy(g_framesList.back().second.data(), dat.data, dat.dataSize);
			game->connection->reportChecksum(_computeCheckSum(reinterpret_cast<short *>(dat.data), dat.dataSize / sizeof(short)), frameId);
			this->_savedData.pop_front();
		}
#endif
		this->_frameTimer -= 1000000 / 60;
		if (this->_frameTimer >= 1000000 / 60)
			goto loopStart;
		return result ? UPDATESTATUS_OK : UPDATESTATUS_GAME_ENDED;
	}


#define BOLD_OFF 2
#define D1_OFF 4
#define D2_OFF 5
#define PAIR_SIZE 7
#define ADDR_SIZE 10

	char digits[] = "0123456789ABCDEF";

	void displayBeforeDiff(size_t index, size_t lastIndex, const char *buffer1, const char *buffer2, char *lineBuffer)
	{
		size_t currentLine = index / 10;
		size_t lastLine = lastIndex / 10;
		char *line1Buffer = lineBuffer + ADDR_SIZE + 2;
		char *line2Buffer = lineBuffer + ADDR_SIZE + 78;

		if (lastLine != currentLine) {
			char *ptr = lineBuffer + ADDR_SIZE - 2;
			size_t i = currentLine;

			*ptr = '0';
			while (i != 0) {
				ptr--;
				*ptr = '0' + i % 10;
				i /= 10;
			}
		}
		for (size_t i = currentLine; i < index; i++) {
			if (i <= lastIndex)
				continue;

			size_t slot = i % 10;
			unsigned char c1 = buffer1[i];
			unsigned char c2 = buffer2[i];

			line1Buffer[slot * PAIR_SIZE + BOLD_OFF] = '0';
			line1Buffer[slot * PAIR_SIZE + D1_OFF] = digits[c1 >> 4];
			line1Buffer[slot * PAIR_SIZE + D2_OFF] = digits[c1 & 0xF];
			line2Buffer[slot * PAIR_SIZE + BOLD_OFF] = '0';
			line2Buffer[slot * PAIR_SIZE + D1_OFF] = digits[c2 >> 4];
			line2Buffer[slot * PAIR_SIZE + D2_OFF] = digits[c2 & 0xF];
		}
	}

	void displayAfterDiff(size_t index, size_t lastIndex, const char *buffer1, const char *buffer2, char *lineBuffer, bool last)
	{
		size_t currentLine = index / 10;
		size_t lastLine = lastIndex / 10;
		char *line1Buffer = lineBuffer + ADDR_SIZE + 2;
		char *line2Buffer = lineBuffer + ADDR_SIZE + 78;

		for (size_t i = lastIndex + 1; i < index && i < lastLine + 10; i++) {
			size_t slot = i % 10;
			unsigned char c1 = buffer1[i];
			unsigned char c2 = buffer2[i];

			line1Buffer[slot * PAIR_SIZE + BOLD_OFF] = '0';
			line1Buffer[slot * PAIR_SIZE + D1_OFF] = digits[c1 >> 4];
			line1Buffer[slot * PAIR_SIZE + D2_OFF] = digits[c1 & 0xF];
			line2Buffer[slot * PAIR_SIZE + BOLD_OFF] = '0';
			line2Buffer[slot * PAIR_SIZE + D1_OFF] = digits[c2 >> 4];
			line2Buffer[slot * PAIR_SIZE + D2_OFF] = digits[c2 & 0xF];
		}
		if (currentLine > lastLine || last)
			game->logger.fatal(lineBuffer);
		if (currentLine > lastLine && currentLine - lastLine > 10)
			game->logger.fatal("| ....... | .. .. .. .. .. .. .. .. .. .. | .. .. .. .. .. .. .. .. .. .. |");
	}

	void displayDiff(size_t index, const char *buffer1, const char *buffer2, char *lineBuffer)
	{
		size_t slot = index % 10;
		char *line1Buffer = lineBuffer + ADDR_SIZE + 2;
		char *line2Buffer = lineBuffer + ADDR_SIZE + 78;
		unsigned char c1 = buffer1[index];
		unsigned char c2 = buffer2[index];

		line1Buffer[slot * PAIR_SIZE + BOLD_OFF] = '1';
		line1Buffer[slot * PAIR_SIZE + D1_OFF] = digits[c1 >> 4];
		line1Buffer[slot * PAIR_SIZE + D2_OFF] = digits[c1 & 0xF];
		line2Buffer[slot * PAIR_SIZE + BOLD_OFF] = '1';
		line2Buffer[slot * PAIR_SIZE + D1_OFF] = digits[c2 >> 4];
		line2Buffer[slot * PAIR_SIZE + D2_OFF] = digits[c2 & 0xF];
	}

	void displayDiffs(const std::vector<size_t> &diffs, const char *buffer1, const char *buffer2, size_t size)
	{
		size_t lastPos = -1;
		char lineBuffer[] = "|         | \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00\033[0m | \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00\033[0m |";

		game->logger.fatal("*---------*-------------------------------*-------------------------------*");
		game->logger.fatal("| Address |            File 1             |             File 2            |");
		game->logger.fatal("*---------*-------------------------------*-------------------------------*");
		for (size_t diff : diffs) {
			displayAfterDiff(diff, lastPos, buffer1, buffer2, lineBuffer, false);
			displayBeforeDiff(diff, lastPos, buffer1, buffer2, lineBuffer);
			displayDiff(diff, buffer1, buffer2, lineBuffer);
			lastPos = diff;
		}
		displayAfterDiff(lastPos, size, buffer1, buffer2, lineBuffer, true);
		game->logger.fatal("*---------*-------------------------------*-------------------------------*");
	}

#define DATA_BEFORE 256
#define DATA_AFTER 256
#define EXTRA_SIZE (DATA_AFTER + DATA_BEFORE)
	RollbackMachine::UpdateStatus RollbackMachine::syncTestUpdate(bool useP1Inputs, bool useP2Inputs)
	{
		//TODO: Use useP1Inputs, useP2Inputs and check the fake pause
		(void)useP1Inputs;
		(void)useP2Inputs;

		unsigned int dataSizeBefore = game->battleMgr->getBufferSize();
		unsigned int dataSizeAfter;
		unsigned int dataSizeAfter2;
		std::unique_ptr<unsigned char, decltype(&Utils::deallocateManually)> dataBefore = {
			Utils::allocateManually(dataSizeBefore + EXTRA_SIZE),
			&Utils::deallocateManually
		};
		std::unique_ptr<unsigned char, decltype(&Utils::deallocateManually)> dataAfter = {
			nullptr, &Utils::deallocateManually
		};
		std::unique_ptr<unsigned char, decltype(&Utils::deallocateManually)> dataAfter2 = {
			nullptr, &Utils::deallocateManually
		};
		int checksum1;
		int checksum2;
		auto lDur = this->inputLeft->_keyDuration;
		auto rDur = this->inputRight->_keyDuration;
		char garbageBefore[DATA_BEFORE];
		char garbageAfter[DATA_AFTER];

		memset(garbageBefore, 0xDD, sizeof(garbageBefore));
		memset(garbageAfter, 0xDD, sizeof(garbageAfter));
		memset(&*dataBefore, 0xDD, dataSizeBefore + EXTRA_SIZE);
		game->battleMgr->copyToBuffer(&*dataBefore + DATA_BEFORE);
		assert_eq(memcmp(&*dataBefore + dataSizeBefore + DATA_AFTER, garbageAfter, DATA_AFTER), 0);
		assert_eq(memcmp(&*dataBefore, garbageBefore, DATA_BEFORE), 0);
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
		dataAfter.reset(Utils::allocateManually(dataSizeAfter + EXTRA_SIZE));
		memset(garbageBefore, 0xCD, sizeof(garbageBefore));
		memset(garbageAfter, 0xCD, sizeof(garbageAfter));
		memset(&*dataAfter, 0xCD, dataSizeAfter + EXTRA_SIZE);
		game->battleMgr->copyToBuffer(&*dataAfter + DATA_BEFORE);
		assert_eq(memcmp(&*dataAfter + dataSizeAfter + DATA_AFTER, garbageAfter, DATA_AFTER), 0);
		assert_eq(memcmp(&*dataAfter, garbageBefore, DATA_BEFORE), 0);
		checksum1 = _computeCheckSum(reinterpret_cast<short *>(&*dataAfter + DATA_BEFORE), dataSizeAfter / sizeof(short));

		game->battleMgr->restoreFromBuffer(&*dataBefore + DATA_BEFORE);
		this->inputLeft->_keyDuration = lDur;
		this->inputRight->_keyDuration = rDur;

		auto result2 = game->battleMgr->update();

		dataSizeAfter2 = game->battleMgr->getBufferSize();
		dataAfter2.reset(Utils::allocateManually(dataSizeAfter2 + EXTRA_SIZE));
		memset(garbageBefore, 0xDC, sizeof(garbageBefore));
		memset(garbageAfter, 0xDC, sizeof(garbageAfter));
		memset(&*dataAfter2, 0xDC, dataSizeAfter2 + EXTRA_SIZE);
		game->battleMgr->copyToBuffer(&*dataAfter2 + DATA_BEFORE);
		assert_eq(memcmp(&*dataAfter2 + dataSizeAfter2 + DATA_AFTER, garbageAfter, DATA_AFTER), 0);
		assert_eq(memcmp(&*dataAfter2, garbageBefore, DATA_BEFORE), 0);
		checksum2 = _computeCheckSum(reinterpret_cast<short *>(&*dataAfter2 + DATA_BEFORE), dataSizeAfter2 / sizeof(short));

		if (checksum1 != checksum2) {
			std::vector<size_t> diffs;
			auto ptr1 = reinterpret_cast<char *>(&*dataAfter);
			auto ptr2 = reinterpret_cast<char *>(&*dataAfter2);

			game->logger.fatal("RollbackMachine::debugRollback: Checksum mismatch");
			game->logger.fatal("Old checksum: 0x" + Utils::toHex(checksum1) + " vs new checksum: 0x" + Utils::toHex(checksum2));
			if (dataSizeAfter != dataSizeAfter2)
				game->logger.fatal("Old data size: " + std::to_string(dataSizeAfter) + " vs new data size: " + std::to_string(dataSizeAfter2));
			else for (size_t i = DATA_BEFORE; i < dataSizeAfter; i++)
				if (ptr1[i] != ptr2[i]) {
					game->logger.fatal(
						"Old data at index " + std::to_string(i - DATA_BEFORE) + ": 0x" + Utils::toHex((&*dataAfter)[i]) + " vs "
						"New data at index " + std::to_string(i - DATA_BEFORE) + ": 0x" + Utils::toHex((&*dataAfter2)[i])
					);
					diffs.push_back(i);
				}

			displayDiffs(diffs, ptr1, ptr2, dataSizeAfter + DATA_AFTER);
			game->battleMgr->logDifference(&*dataAfter + DATA_BEFORE, &*dataAfter2 + DATA_BEFORE);
			throw AssertionFailedExceptionMsg(
				"checksum1 == checksum2",
				Utils::toHex(checksum1) + " != " + Utils::toHex(checksum2)
			);
		}
		assert_exp(result1 == result2);
		assert_exp(lDur2 == this->inputLeft->_keyDuration);
		assert_exp(rDur2 == this->inputRight->_keyDuration);
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
			++it;
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
			++it;
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

	std::pair<long long int, long long int> RollbackMachine::getLastAvgDelayTimes() const
	{
		std::pair<long long int, long long int> times{0, 0};

		for (auto t : this->_opDiffTimes)
			times.first += t;
		for (auto t : this->_diffTimes)
			times.second += t;
		if (!this->_opDiffTimes.empty())
			times.first /= (long long int)this->_opDiffTimes.size();
		if (!this->_diffTimes.empty())
			times.second /= (long long int)this->_diffTimes.size();
		return times;
	}

	std::pair<long long int, long long int> RollbackMachine::getLastDelayTimes() const
	{
		std::pair<long long int, long long int> times{0, 0};

		if (!this->_opDiffTimes.empty())
			times.first = this->_opDiffTimes.front();
		if (!this->_diffTimes.empty())
			times.second = this->_diffTimes.front();
		return times;
	}
}
