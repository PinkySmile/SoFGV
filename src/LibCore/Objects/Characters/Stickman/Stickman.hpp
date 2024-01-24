//
// Created by PinkySmile on 01/03/2022.
//

#ifndef SOFGV_STICKMAN_HPP
#define SOFGV_STICKMAN_HPP


#include <random>
#include "Objects/Characters/Character.hpp"

namespace SpiralOfFate
{
	class Stickman : public Character {
	private:
#pragma pack(push, 1)
		struct Data {
			unsigned _time;
			unsigned _oldAction;
			bool _hasBuff;
		};
		static_assert(sizeof(Data) == 9, "Data has wrong size");
#pragma pack(pop)

		// Game State
		unsigned _time = 0;
		unsigned _oldAction = 0;
		bool _hasBuff = false;

	protected:
		void _onMoveEnd(const FrameData &lastData) override;
		void _computeFrameDataCache() override;
		void _renderExtraEffects(const Vector2f &pos) const override;

		void _forceStartMove(unsigned int action) override;

	public:
		Stickman() = default;
		Stickman(unsigned index, const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette, std::shared_ptr<IInput> input);
		unsigned int getClassId() const override;
		void update() override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const override;
		size_t printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const override;
		bool matchEndUpdate() override;
		void onMatchEnd() override;
	};
}


#endif //SOFGV_STICKMAN_HPP
