//
// Created by PinkySmile on 02/02/2022.
//

#ifndef SOFGV_PROJECTILE_HPP
#define SOFGV_PROJECTILE_HPP


#include "SubObject.hpp"

namespace SpiralOfFate
{
	class Projectile : public SubObject {
	public:
		enum TypeSwitch : unsigned char {
			TYPESWITCH_NONE,
			TYPESWITCH_NEUTRAL,
			TYPESWITCH_VOID,
			TYPESWITCH_MATTER,
			TYPESWITCH_SPIRIT,
			TYPESWITCH_NON_TYPED
		};

	private:
		enum ProjectileAnimation : unsigned char {
			ANIMATION_DISAPPEAR,
			ANIMATION_FADE,
			ANIMATION_BLOCK
		};

		struct ProjectileAnimationData {
			bool hasValue;
			ProjectileAnimation type;
			unsigned data;

			ProjectileAnimationData(const nlohmann::json &json);
		};

#pragma pack(push, 1)
		struct Data {
			unsigned _nbHit;
			unsigned _animationCtr;
			unsigned _debuffDuration;
			unsigned _animData;
			bool _disabled;
			TypeSwitch _typeSwitchFlags;
			ProjectileAnimation _animType;
		};
		static_assert(sizeof(Data) == 19, "Data has wrong size");
#pragma pack(pop)

		// Game State
		unsigned _animationCtr = 0;
		unsigned _nbHit = 0;
		unsigned _debuffDuration = 0;
		bool _disabled = false;
		unsigned _animData = 0;
		ProjectileAnimation _animType = ANIMATION_DISAPPEAR;
		// Technically non-game state, but is required to be saved
		// because the object is recreated from scratch
		TypeSwitch _typeSwitch;

		// Non-game state
		const unsigned _maxHit;
		const unsigned _endBlock;
		const ProjectileAnimationData _onHitDieAnim;
		const ProjectileAnimationData _onBlockDieAnim;
		const ProjectileAnimationData _onGetHitDieAnim;
		const ProjectileAnimationData _onOwnerHitDieAnim;
		const bool _loop;

	protected:
		void _onMoveEnd(const FrameData &lastData) override;
		void _computeFrameDataCache() override;
		void _disableObject(const ProjectileAnimationData &data);

	public:
		Projectile(
			bool owner,
			class Character *ownerObj,
			unsigned id,
			const nlohmann::json &json,
			TypeSwitch typeSwitchFlags,
			unsigned debuffDuration
		);
		Projectile(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned team,
			bool direction,
			Vector2f pos,
			bool owner,
			class Character *ownerObj,
			unsigned id,
			const nlohmann::json &json,
			TypeSwitch typeSwitchFlags,
			unsigned debuffDuration
		);
		void update() override;
		void hit(Object &other, const FrameData *data) override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		size_t printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const override;
		size_t printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const override;
		void getHit(Object &other, const FrameData *data) override;
		bool hits(const Object &other) const override;


		static ProjectileAnimation animationFromString(const std::string &str);

		unsigned int getDebuffDuration() const override;

		void kill() override;
	};
}


#endif //SOFGV_PROJECTILE_HPP
