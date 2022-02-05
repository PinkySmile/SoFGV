//
// Created by Gegel85 on 05/02/2022.
//

#ifndef BATTLE_FAKEOBJECT_HPP
#define BATTLE_FAKEOBJECT_HPP


#include "AObject.hpp"

namespace Battle
{
	class FakeObject : public AObject {
	public:
		FakeObject(const std::map<unsigned, std::vector<std::vector<FrameData>>> &frameData);
		void render() const override;

	protected:
		void _onMoveEnd(const FrameData &lastData) override;

	public:

		friend class InGame;
	};
}

#endif //BATTLE_FAKEOBJECT_HPP
