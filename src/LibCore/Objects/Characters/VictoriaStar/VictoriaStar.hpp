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

		};

	public:
		VictoriaStar() = default;
		VictoriaStar(unsigned index, const std::string &folder, const std::pair<std::vector<Color>, std::vector<Color>> &palette, std::shared_ptr<IInput> input);
		unsigned int getClassId() const override;
		unsigned int getBufferSize() const override;
		void copyToBuffer(void *data) const override;
		void restoreFromBuffer(void *data) override;

	protected:
		bool _startMove(unsigned int action) override;

		std::pair<unsigned int, std::shared_ptr<IObject>>
		_spawnSubObject(unsigned int id, bool needRegister) override;
	};
}


#endif //SOFGV_VICTORIASTAR_HPP
