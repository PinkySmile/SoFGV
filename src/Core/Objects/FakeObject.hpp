//
// Created by Gegel85 on 05/02/2022.
//

#ifndef BATTLE_FAKEOBJECT_HPP
#define BATTLE_FAKEOBJECT_HPP


#include "Object.hpp"

namespace SpiralOfFate
{
	class FakeObject : public Object {
	protected:
		void _onMoveEnd(const FrameData &lastData) override;

	public:
		FakeObject(const std::map<unsigned, std::vector<std::vector<FrameData>>> &frameData);
		void render() const override;

		friend class InGame;
	};
}

#endif //BATTLE_FAKEOBJECT_HPP
