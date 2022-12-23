//
// Created by PinkySmile on 05/02/2022.
//

#ifndef SOFGV_FAKEOBJECT_HPP
#define SOFGV_FAKEOBJECT_HPP


#include "Object.hpp"

namespace SpiralOfFate
{
	class FakeObject : public Object {
	protected:
		void _onMoveEnd(const FrameData &lastData) override;
		void _applyNewAnimFlags() override;

	public:
		FakeObject(const std::map<unsigned, std::vector<std::vector<FrameData>>> &frameData);
		void render() const override;

		friend class InGame;
	};
}

#endif //SOFGV_FAKEOBJECT_HPP
