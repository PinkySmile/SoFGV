//
// Created by PinkySmile on 30/04/23.
//

#ifndef SOFGV_BUTTERFLY_HPP
#define SOFGV_BUTTERFLY_HPP


#include "Objects/Characters/SubObject.hpp"

namespace SpiralOfFate
{
	class Character;
	class VictoriaStar;

	class Butterfly : public SubObject {
	protected:
#pragma pack(push, 1)
		struct HappyData {
			float _counter;
			Vector2f _target;
			Vector2f _base;
			unsigned char _ctr;
			unsigned char _maxAlpha;
		};
		struct WeirdData {
		};
#pragma pack(pop)

		// Game state
		float _counter = 0;
		Vector2f _target;
		Vector2f _base;
		unsigned char _defenseCtr = 0;
		unsigned char _ctr = 1;
		unsigned char _maxAlpha = 0;

		// Non-game state
		VictoriaStar *_owner;
		Character *_opponent;
		Butterfly *_copy;
		float _alpha = 1.f;
		bool _disabled = false;
		mutable FrameData _fakeFrameData;

	public:
		Butterfly(VictoriaStar *owner, Character *opponent, bool isLeft, Butterfly *copy, const std::vector<std::vector<FrameData>> &frameData, unsigned id);
		int getLayer() const override;
		void update() override;
		float getAngle() const;
		const FrameData *getCurrentFrameData() const override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		void defensiveFormation(const Object &target);
		size_t printDifference(const char *msgStart, void *pVoid, void *pVoid1) const override;
	};
}


#endif //SOFGV_BUTTERFLY_HPP
