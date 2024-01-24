//
// Created by PinkySmile on 02/02/2022.
//

#ifndef SOFGV_PROJECTILE_HPP
#define SOFGV_PROJECTILE_HPP


#include "Character.hpp"
#include "SubObject.hpp"

namespace SpiralOfFate
{
	class Projectile : public SubObject {
	private:
		enum ProjectileAnimation {
			ANIMATION_DISAPPEAR,
			ANIMATION_FADE,
			ANIMATION_BLOCK
		};

#pragma pack(push, 1)
		struct Data {
			unsigned nbHit;
			unsigned animationCtr;
			bool disabled;
			bool fadingOut;
		};
		static_assert(sizeof(Data) == 10, "Data has wrong size");
#pragma pack(pop)

		// Game State
		unsigned _animationCtr = 0;
		unsigned _nbHit = 0;
		bool _disabled = false;
		bool _fadingOut = false;

		// Non-game state
		unsigned _maxHit;
		unsigned _endBlock;
		unsigned _animationData;
		ProjectileAnimation _anim;
		bool _loop;
		bool _disableOnHit;

	protected:
		void _onMoveEnd(const FrameData &lastData) override;
		void _computeFrameDataCache() override;
		void _disableObject();

	public:
		Projectile(
			bool owner,
			class Character *ownerObj,
			unsigned id,
			const nlohmann::json &json
		);
		Projectile(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned team,
			bool direction,
			Vector2f pos,
			bool owner,
			class Character *ownerObj,
			unsigned id,
			const nlohmann::json &json
		);
		bool isDead() const override;
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
	};
}


#endif //SOFGV_PROJECTILE_HPP
