//
// Created by PinkySmile on 11/06/23.
//

#ifndef SOFGV_FLOWER_HPP
#define SOFGV_FLOWER_HPP


#include "Objects/Characters/SubObject.hpp"

namespace SpiralOfFate
{
	class Flower : public SubObject {
	private:
		class VictoriaStar *_owner;

	protected:
		void _onMoveEnd(const FrameData &lastData) override;

	public:
		Flower(
			class VictoriaStar *ownerObj,
			const std::vector<std::vector<FrameData>> &frameData,
			bool direction,
			Vector2f pos,
			bool owner,
			unsigned id
		);

		void update() override;
	};
}


#endif //SOFGV_FLOWER_HPP
