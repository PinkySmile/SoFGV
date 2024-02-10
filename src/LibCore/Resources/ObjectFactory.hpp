//
// Created by PinkySmile on 10/02/24.
//

#ifndef SOFGV_OBJECTFACTORY_HPP
#define SOFGV_OBJECTFACTORY_HPP


#include "Objects/Object.hpp"
#include "Assert.hpp"

namespace SpiralOfFate
{
	class ObjectFactory {
	public:
		size_t getObjectSize(const IObject &obj);
		void saveObject(void *data, const IObject &obj);
		std::shared_ptr<IObject> createObject(BattleManager &mgr, void *data, std::pair<class Character *, class Character *> players);

		template<typename T>
		std::shared_ptr<T> createObject(BattleManager &mgr, void *data, std::pair<class Character *, class Character *> players) {
			auto s = createObject(mgr, data, std::move(players));

			checked_cast(obj, T, &*s);
			return std::shared_ptr<T>(s, obj);
		}
	};
}

#include "Game.hpp"

#endif //SOFGV_OBJECTFACTORY_HPP
