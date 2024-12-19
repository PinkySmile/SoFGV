//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_IOBJECT_HPP
#define SOFGV_IOBJECT_HPP


#include "Resources/Assets/FrameData.hpp"

namespace SpiralOfFate
{
	class BattleManager;

	class IObject {
	public:
		bool showBoxes = false;
		bool showAttributes = false;

		virtual ~IObject() = default;
		virtual void render() const = 0;
		virtual void update() = 0;
		virtual bool isDead() const = 0;
		virtual void kill() = 0;
		virtual unsigned getBufferSize() const = 0;
		virtual void copyToBuffer(void *data) const = 0;
		virtual void restoreFromBuffer(void *data) = 0;
		// return a std::pair<size_t, size_t> instead
		virtual size_t printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const = 0;
		virtual size_t printContent(const char *msgStart, void *data, unsigned startOffset, size_t dataSize) const = 0;
		virtual unsigned getClassId() const = 0;
		virtual int getLayer() const = 0;

		friend class BattleManager;
	};
}


#endif //SOFGV_IOBJECT_HPP
