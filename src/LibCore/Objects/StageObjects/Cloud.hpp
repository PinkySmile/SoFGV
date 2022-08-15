//
// Created by andgel on 15/08/22
//

#ifndef SOFGV_CLOUD_HPP
#define SOFGV_CLOUD_HPP


#include "../Object.hpp"

namespace SpiralOfFate
{
	class Cloud : public Object {
	protected:
		float _minSpeed;
		float _maxSpeed;
		unsigned _minY;
		unsigned _maxY;
		void _reload();
		void _refresh();

	public:
		Cloud(nlohmann::json &json);
		void update() override;
	};
}


#endif //SOFGV_CLOUD_HPP
