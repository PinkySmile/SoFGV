//
// Created by PinkySmile on 15/08/2022.
//

#include "RollbackMachine.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	RollbackMachine::RollbackData::RollbackData()
	{
		this->dataSize = game->battleMgr->getBufferSize();
		this->data = malloc(this->dataSize);
		game->battleMgr->copyToBuffer(this->data);
	}

	RollbackMachine::RollbackData::RollbackData(IInput &left, IInput &right, RollbackData *old) :
		left(left, old ? &old->left : nullptr),
		right(right, old ? &old->right : nullptr)
	{
		this->dataSize = game->battleMgr->getBufferSize();
		this->data = malloc(this->dataSize);
		game->battleMgr->copyToBuffer(this->data);
	}

	RollbackMachine::RollbackData::~RollbackData()
	{
		free(this->data);
	}

	RollbackMachine::InputData::InputData(IInput &input, InputData *old)
	{
		if (old)
			this->keyDuration = old->keyDuration;
		else
			this->keyDuration.fill(0);
		if (input.hasInputs()) {
			input.update();
			for (int i = 0; i < INPUT_NUMBER - 1; ++i)
				this->keyStates[i] = input.isPressed(static_cast<InputEnum>(i));
			this->predicted = false;
		} else {
			if (old)
				this->keyStates = old->keyStates;
			else
				this->keyStates.reset();
			this->predicted = true;
		}
	}

	RollbackMachine::RollbackMachine(Character *left, Character *right) :
		_realInputLeft(left->_input),
		_realInputRight(right->_input)
	{
		left->_input = this->inputLeft;
		right->_input = this->inputRight;
	}

	RollbackMachine::UpdateStatus RollbackMachine::update(bool useP1Inputs, bool useP2Inputs)
	{
		//TODO: Use useP1Inputs, useP2Inputs and check the fake pause
		if (this->_savedData.size() == MAX_ROLLBACK && (
			(!this->_realInputLeft->hasInputs() && this->_savedData.front().left.predicted) ||
			(!this->_realInputRight->hasInputs() && this->_savedData.front().right.predicted)
		))
			return UPDATESTATUS_NO_INPUTS;

		auto it = this->_savedData.begin();

		while (it != this->_savedData.end() && (
			(!it->left.predicted && !it->right.predicted) ||
			(it->left.predicted  && this->_realInputLeft->hasInputs()) ||
			(it->right.predicted && this->_realInputRight->hasInputs())
		))
			++it;

		this->_manageRollback(it);
		if (this->_realInputLeft->hasInputs())
			this->_realInputLeft->update();
		if (this->_realInputRight->hasInputs())
			this->_realInputRight->update();
		this->_savedData.emplace_back(*this->_realInputLeft, *this->_realInputRight, this->_savedData.empty() ? nullptr : &this->_savedData.back());
		this->inputLeft->_keyStates = this->_savedData.back().left.keyStates;
		this->inputRight->_keyStates = this->_savedData.back().right.keyStates;
		this->inputLeft->_keyDuration = this->_savedData.back().left.keyDuration;
		this->inputRight->_keyDuration = this->_savedData.back().right.keyDuration;
		while (this->_savedData.size() > MAX_ROLLBACK)
			this->_savedData.pop_front();
		return game->battleMgr->update() ? UPDATESTATUS_OK : UPDATESTATUS_GAME_ENDED;
	}

	void RollbackMachine::debugRollback()
	{
		if (this->_savedData.end() == this->_savedData.begin())
			return;

		auto it = std::prev(this->_savedData.end());
		unsigned int dataSize = game->battleMgr->getBufferSize();
		void *data = malloc(dataSize);
		unsigned int dataSize2;
		void *data2;
		int oldChecksum;
		int newChecksum;

		game->battleMgr->copyToBuffer(data);
		oldChecksum = _computeCheckSum((short *)data, dataSize / sizeof(short));

		game->battleMgr->restoreFromBuffer(it->data);
		this->inputLeft->_keyStates = it->left.keyStates;
		this->inputRight->_keyStates = it->right.keyStates;
		this->inputLeft->_keyDuration = it->left.keyDuration;
		this->inputRight->_keyDuration = it->right.keyDuration;
		game->battleMgr->update();

		dataSize2 = game->battleMgr->getBufferSize();
		data2 = malloc(dataSize);
		game->battleMgr->copyToBuffer(data2);
		newChecksum = _computeCheckSum((short *)data2, dataSize2 / sizeof(short));

		if (oldChecksum != newChecksum) {
			game->logger.fatal("RollbackMachine::debugRollback: Checksum mismatch");
			game->logger.fatal("Old checksum: " + std::to_string(oldChecksum) + " vs new checksum: " + std::to_string(newChecksum));
			if (dataSize != dataSize2)
				game->logger.fatal("Old data size: " + std::to_string(dataSize) + " vs new data size: " + std::to_string(dataSize2));
			else
				for (size_t i = 0; i < dataSize; i++)
					if (((char *)data)[i] != ((char *)data2)[i])
						game->logger.fatal("Old data at index " + std::to_string(i) + ": " + std::to_string(((char *)data)[i]) + " vs new data at index " + std::to_string(i) + ": " + std::to_string(((char *)data2)[i]));
			game->battleMgr->logDifference(data, data2);
			throw AssertionFailedException(
				"oldChecksum == newChecksum",
				std::to_string(oldChecksum) + " != " + std::to_string(newChecksum)
			);
		}
		free(data);
		free(data2);
	}

	void RollbackMachine::_manageRollback(std::list<RollbackData>::iterator it)
	{
		auto old = it;

        if (it != this->_savedData.begin())
		    --old;
		while (it != this->_savedData.end()) {
			auto left  = it->left.predicted  ? InputData(*this->_realInputLeft,  it == this->_savedData.begin() ? nullptr : &old->left)  : it->left;
			auto right = it->right.predicted ? InputData(*this->_realInputRight, it == this->_savedData.begin() ? nullptr : &old->right) : it->right;

			this->inputLeft->_keyStates = left.keyStates;
			this->inputRight->_keyStates = right.keyStates;
			this->inputLeft->_keyDuration = left.keyDuration;
			this->inputRight->_keyDuration = right.keyDuration;
			if (!it->data) {
				it->dataSize = game->battleMgr->getBufferSize();
				it->data = malloc(it->dataSize);
				game->battleMgr->copyToBuffer(it->data);
				game->battleMgr->update();
			} else if (left.keyStates != it->left.keyStates || right.keyStates != it->right.keyStates) {
				if (it->data) {
					game->battleMgr->restoreFromBuffer(it->data);

					auto i = it;

					++i;
					while (i != this->_savedData.end()) {
						free(i->data);
						i->data = nullptr;
						++i;
					}
				}
				game->battleMgr->update();
			}
			it->left = left;
			it->right = right;
			++it;
			++old;
		}
	}

	/*
	 * Simple checksum function stolen from wikipedia:
	 *
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
}
