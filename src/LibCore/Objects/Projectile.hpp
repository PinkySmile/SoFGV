//
// Created by Gegel85 on 02/02/2022.
//

#ifndef SOFGV_PROJECTILE_HPP
#define SOFGV_PROJECTILE_HPP


#include "Object.hpp"

namespace SpiralOfFate
{
	class Projectile : public Object {
	private:
		struct Data {
			unsigned maxHit;
			unsigned nbHit;
		};

		unsigned maxHit;
		unsigned nbHit = 0;
		bool owner;
		unsigned id;

	public:
		Projectile(bool owner, unsigned id, unsigned maxHit = 1);
		Projectile(const std::vector<std::vector<FrameData>> &frameData, unsigned team, bool direction, Vector2f pos, bool owner, unsigned id, unsigned maxHit = 1);
		bool isDead() const override;
		void update() override;
		unsigned int getClassId() const override;
		bool getOwner() const;
		unsigned int getId() const;
		void hit(IObject &other, const FrameData *data) override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;

		void getHit(IObject &other, const FrameData *data) override;
	};
}


#endif //SOFGV_PROJECTILE_HPP
