//
// Created by PinkySmile on 11/05/2022.
//

#include <stdexcept>
#include "SyncTestHandler.hpp"
#include "NetManager.hpp"

namespace SpiralOfFateNet
{
	SyncTestHandler::SyncTestHandler(NetManager::Params params) :
		_params(params)
	{
		this->_inputs.emplace_back(nullptr, nullptr);
	}

	SyncTestHandler::~SyncTestHandler()
	{
		this->_params.handlers.freeState(this->_rollbackBuffer.buffer, this->_rollbackBuffer.bufferSize);
		free(this->_rollbackBuffer.leftInputs);
		free(this->_rollbackBuffer.rightInputs);
	}

	void SyncTestHandler::addInputs(void *data, unsigned playerId)
	{
		auto &inputs = this->_inputs.back();
		auto &input = !playerId ? inputs.first : inputs.second;

		free(input);
		input = malloc(this->_params.inputSize);
		memcpy(input, data, this->_params.inputSize);
	}

	void SyncTestHandler::switchMenu(unsigned int menuId, void *initFrame, size_t frameSize)
	{
		if (this->_params.handlers.switchMenu(menuId, initFrame, frameSize)) {
			this->_currentFrame = 0;
			this->_rollbackBuffer.checksum = 0;
			this->_rollbackBuffer.bufferSize = 0;
			this->_rollbackBuffer.currentFrame = 0;
			this->_params.handlers.freeState(this->_rollbackBuffer.buffer, this->_rollbackBuffer.bufferSize);
			free(this->_rollbackBuffer.leftInputs);
			free(this->_rollbackBuffer.rightInputs);
			this->_rollbackBuffer.buffer = nullptr;
			this->_rollbackBuffer.leftInputs = nullptr;
			this->_rollbackBuffer.rightInputs = nullptr;
		}
	}

	NetStats SyncTestHandler::getNetStats()
	{
		return {};
	}

	SyncTestHandler::RollbackBuffer SyncTestHandler::_saveState()
	{
		RollbackBuffer buffer;

		this->_params.handlers.saveState(&buffer.buffer, &buffer.bufferSize, &buffer.checksum);
		buffer.leftInputs  = memcpy(malloc(this->_params.inputSize), this->_inputs.front().first, this->_params.inputSize);
		buffer.rightInputs = memcpy(malloc(this->_params.inputSize), this->_inputs.front().second, this->_params.inputSize);
		buffer.currentFrame = this->_currentFrame;
		return buffer;
	}

	void SyncTestHandler::update()
	{
		void *inputs[]{
			this->_inputs.front().first,
			this->_inputs.front().second
		};
		RollbackBuffer newBuffer;

		// We go from frame n to frame n + 1
		this->_params.handlers.nextFrame(inputs);

		// We save frame n + 1
		newBuffer = this->_saveState();

		if (this->_rollbackBuffer.buffer) {
			// We restore frame n
			this->_params.handlers.loadState(this->_rollbackBuffer.buffer, this->_rollbackBuffer.bufferSize);

			// We replay the frame n to n + 1
			this->_params.handlers.nextFrame(inputs);

			// We delete frame n because we don't need it anymore
			this->_params.handlers.freeState(this->_rollbackBuffer.buffer, this->_rollbackBuffer.bufferSize);
			free(this->_rollbackBuffer.leftInputs);
			free(this->_rollbackBuffer.rightInputs);

			// We save frame n + 1 a second time
			this->_rollbackBuffer = this->_saveState();
		}
		this->_currentFrame++;

		// Frame n cleanup
		free(this->_inputs.front().first);
		free(this->_inputs.front().second);
		this->_inputs.pop_front();
		this->_inputs.emplace_back(nullptr, nullptr);
		if (!this->_rollbackBuffer.buffer) {
			// Frame n doesn't exist, so we don't have much to do
			this->_rollbackBuffer = newBuffer;
			return;
		}
		// We free the duplicate buffer because we are only interested in the checksum
		this->_params.handlers.freeState(newBuffer.buffer, newBuffer.bufferSize);
		free(newBuffer.leftInputs);
		free(newBuffer.rightInputs);

		// We check if the 2 generated frame n + 1 match
		// (The one saved in newBuffer and the one saved in the internal buffer)
		if (newBuffer.checksum != this->_rollbackBuffer.checksum)
			throw std::runtime_error(
				"Debug Assertion newBuffer.checksum == this->_rollbackBuffer.checksum"
				" failed in " __FILE__
				" at line " + std::to_string(__LINE__) +
				" in void SyncTestHandler::update(): Frame " + std::to_string(newBuffer.currentFrame) + ": " + std::to_string(newBuffer.checksum) + " != " + std::to_string(this->_rollbackBuffer.checksum)
			);
	}

	void SyncTestHandler::setDelay(unsigned int delay)
	{
		if (!delay || delay > 16)
			throw std::runtime_error(
				"Debug Assertion delay && delay <= 16"
				" failed in " __FILE__
				" at line " + std::to_string(__LINE__) +
				" in void SyncTestHandler::setDelay(): Delay must be between 1 and 16"
			);
		while (this->_inputs.size() > delay) {
			free(this->_inputs.front().first);
			free(this->_inputs.front().second);
			this->_inputs.pop_front();
		}
		while (this->_inputs.size() < delay) {
			auto copy1 = malloc(this->_params.inputSize);
			auto copy2 = malloc(this->_params.inputSize);

			memcpy(copy1, this->_inputs.back().first, this->_params.inputSize);
			memcpy(copy2, this->_inputs.back().second, this->_params.inputSize);
			this->_inputs.emplace_back(copy1, copy2);
		}
	}
}