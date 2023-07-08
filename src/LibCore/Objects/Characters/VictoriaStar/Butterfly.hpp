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
			Vector2f _attackPos;
			Vector2f _attackPosStep;
			unsigned _attackId;
			unsigned char _attackFadeCtr;
			unsigned char _attackAppearCtr;
			unsigned char _attackAttackCtr;
			unsigned char _attackTravelCtr;
			unsigned char _defenseCtr;
			unsigned char _ctr;
			unsigned char _maxAlpha;
		};
#pragma pack(pop)

		// Game state
		float _counter = 0;
		Vector2f _target;
		Vector2f _base;
		Vector2f _attackPos;
		Vector2f _attackPosStep;
		unsigned _attackId = 0;
		unsigned char _attackFadeCtr = 0;
		unsigned char _attackAppearCtr = 0;
		unsigned char _attackAttackCtr = 0;
		unsigned char _attackTravelCtr = 0;
		unsigned char _defenseCtr = 0;
		unsigned char _ctr = 1;
		unsigned char _maxAlpha = 0;

		// Non-game state
		VictoriaStar *_owner;
		Character *_opponent;
		Butterfly *_copy;
		float _alpha = 1.f;
		bool _disabled = false;
		unsigned char _attackFadeTime = 0;

		void _updateCurrentAttack();
		void _computeFrameDataCache() override;
		void _onMoveEnd(const FrameData &lastData) override;

	public:
		Butterfly(VictoriaStar *owner, Character *opponent, bool isLeft, Butterfly *copy, const std::vector<std::vector<FrameData>> &frameData, unsigned id);
		int getLayer() const override;
		void update() override;
		float getAngle() const;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		void defensiveFormation(const Object &target);
		size_t printDifference(const char *msgStart, void *pVoid, void *pVoid1, unsigned startOffset) const override;
		void startAttack(Vector2f pos, unsigned block, unsigned char fadeTime, unsigned char attackTime, unsigned char travelTime);
		bool isDisabled(const IObject &target) const override;
		void hit(IObject &other, const FrameData *data) override;
		bool hits(const IObject &other) const override;

		friend VictoriaStar;
	};
}


#endif //SOFGV_BUTTERFLY_HPP
