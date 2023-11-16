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
		static_assert(sizeof(Data) == 12, "Data has wrong size");
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
		virtual void touch(Object &object);
		void getHit(Object &other, const FrameData *data) override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		unsigned int getBufferSize() const override;
		size_t printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const override;
		size_t printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const override;
		int getLayer() const override;
	};
}


#endif //SOFGV_PLATFORM_HPP
