//
// Created by PinkySmile on 26/02/23.
//

#ifndef SOFGV_VICTORIASTAR_HPP
#define SOFGV_VICTORIASTAR_HPP


#include "Objects/Character.hpp"

namespace SpiralOfFate
{
	class VictoriaStar : public Character {
	private:
		struct Data {
			bool _hitShadow;
		};

		bool _hitShadow = false;

	protected:
		bool _startMove(unsigned int action) override;
		std::pair<unsigned int, std::shared_ptr<IObject>>_spawnSubObject(unsigned int id, bool needRegister) override;

		bool _canCancel(unsigned int action) override;

		void _forceStartMove(unsigned int action) override;

	public:
		VictoriaStar() = default;
		VictoriaStar(unsigned index, const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette, std::shared_ptr<IInput> input);
		unsigned int getClassId() const override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;
		void getHit(IObject &other, const FrameData *data) override;
		bool hits(const IObject &other) const override;
		void hit(IObject &other, const FrameData *data) override;
	};
}


#endif //SOFGV_VICTORIASTAR_HPP
