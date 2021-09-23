//
// Created by PinkySmile on 18/09/2021
//

#include "BattleManager.hpp"

namespace Battle
{
	BattleManager::BattleManager(ACharacter *leftCharacter, ACharacter *rightCharacter) :
		_leftCharacter(leftCharacter),
		_rightCharacter(rightCharacter)
	{
		this->_leftCharacter->init(true);
		this->_rightCharacter->init(false);
	}

	void BattleManager::update()
	{
		std::vector<std::tuple<IObject *, IObject *, const FrameData *>> collisions;

		this->_leftCharacter->update();
		this->_rightCharacter->update();
		for (auto &object : this->_objects)
			object->update();

		for (auto &object : this->_objects) {
			if (this->_leftCharacter->hits(*object))
				collisions.emplace_back(&*this->_leftCharacter, &*object, this->_leftCharacter->getCurrentFrameData());
			if (object->hits(*this->_leftCharacter))
				collisions.emplace_back(&*object, &*this->_leftCharacter, object->getCurrentFrameData());

			if (this->_rightCharacter->hits(*object))
				collisions.emplace_back(&*this->_rightCharacter, &*object, this->_rightCharacter->getCurrentFrameData());
			if (object->hits(*this->_rightCharacter))
				collisions.emplace_back(&*object, &*this->_rightCharacter, object->getCurrentFrameData());

			for (auto &object2 : this->_objects)
				if (object2 != object)
					if (object->hits(*object2))
						collisions.emplace_back(&*object, &*object2, object->getCurrentFrameData());
		}

		for (auto &[attacker, defender, data] : collisions) {
			attacker->hit(*defender, data);
			defender->getHit(*attacker, data);
		}

		if (this->_leftCharacter->isDead())
			// The state is messed up
			// TODO: Do real exceptions
			throw std::invalid_argument("Invalid state");
		if (this->_rightCharacter->isDead())
			// The state is messed up
			// TODO: Do real exceptions
			throw std::invalid_argument("Invalid state");

		for (unsigned i = 0; i < this->_objects.size(); i++)
			if (this->_objects[i]->isDead())
				this->_objects.erase(this->_objects.begin() + i--);
	}

	void BattleManager::render()
	{
		this->_leftCharacter->render();
		this->_rightCharacter->render();
		for (auto &object : this->_objects)
			object->render();
	}

	void BattleManager::registerObject(IObject *object)
	{
		this->_objects.emplace_back(object);
	}
}