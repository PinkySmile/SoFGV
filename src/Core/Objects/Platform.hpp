//
// Created by Gegel85 on 14/02/2022.
//

#ifndef BATTLE_PLATFORM_HPP
#define BATTLE_PLATFORM_HPP


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
	};
}


#endif //BATTLE_PLATFORM_HPP
