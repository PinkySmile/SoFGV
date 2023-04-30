//
// Created by PinkySmile on 14/02/2022.
//

#ifndef SOFGV_PLATFORM_HPP
#define SOFGV_PLATFORM_HPP


#include "Object.hpp"

namespace SpiralOfFate
{
	class Platform : public Object {
	private:
#pragma pack(push, 1)
		struct Data {
			float _width;
			unsigned _cooldown;
			unsigned _deathTimer;
		};
#pragma pack(pop)

		float _width;
		unsigned _cooldown = 0;
		unsigned _deathTimer = 0;

	protected:
		void _onMoveEnd(const FrameData &lastData) override;

	public:
		Platform(const std::string &framedata, float width, short hp, unsigned cooldown, Vector2f pos);
		float getWidth();
		Vector2f getPosition();
		bool isDestructed();
		void update() override;
		void render() const override;
		virtual void touch(IObject &object);
		void getHit(IObject &other, const FrameData *data) override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		unsigned int getBufferSize() const override;
		size_t printDifference(const char *msgStart, void *pVoid, void *pVoid1) const override;
		int getLayer() const override;
	};
}


#endif //SOFGV_PLATFORM_HPP
