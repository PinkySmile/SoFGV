//
// Created by PinkySmile on 18/09/2021
//

#include "BattleManager.hpp"
#include "../Logger.hpp"
#include "Game.hpp"

namespace Battle
{
	BattleManager::BattleManager(const CharacterParams &leftCharacter, const CharacterParams &rightCharacter) :
		_leftCharacter(leftCharacter.character),
		_rightCharacter(rightCharacter.character)
	{
		this->_leftCharacter->setOpponent(rightCharacter.character);
		this->_rightCharacter->setOpponent(leftCharacter.character);
		this->_leftCharacter->init(true, leftCharacter.hp, leftCharacter.maxJumps, leftCharacter.gravity);
		this->_rightCharacter->init(false, rightCharacter.hp, rightCharacter.maxJumps, rightCharacter.gravity);
	}

	void BattleManager::consumeEvent(const sf::Event &event)
	{
		this->_leftCharacter->consumeEvent(event);
		this->_rightCharacter->consumeEvent(event);
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::J)
			this->_step = !this->_step;
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::K)
			this->_next = true;
	}

	void BattleManager::update()
	{
		std::vector<std::tuple<IObject *, IObject *, const FrameData *>> collisions;

		if (this->_step && !this->_next)
			return;
		this->_next = false;

		if (this->_hitStop) {
			this->_hitStop--;
			return;
		}

		this->_leftCharacter->update();
		this->_rightCharacter->update();
		for (auto &object : this->_objects)
			object->update();

		if (this->_leftCharacter->hits(*this->_rightCharacter))
			collisions.emplace_back(&*this->_leftCharacter, &*this->_rightCharacter, this->_leftCharacter->getCurrentFrameData());
		if (this->_rightCharacter->hits(*this->_leftCharacter))
			collisions.emplace_back(&*this->_rightCharacter, &*this->_leftCharacter, this->_rightCharacter->getCurrentFrameData());

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

		if (this->_leftCharacter->collides(*this->_rightCharacter))
			this->_leftCharacter->collide(*this->_rightCharacter);
		for (auto &object : this->_objects) {
			if (this->_leftCharacter->collides(*object))
				this->_leftCharacter->collide(*object);
			if (this->_rightCharacter->collides(*object))
				this->_rightCharacter->collide(*object);
		}
	}

	void BattleManager::render()
	{
		this->_leftCharacter->render();
		this->_rightCharacter->render();
		for (auto &object : this->_objects)
			object->render();

		sf::RectangleShape rect;

		rect.setOutlineThickness(1);
		rect.setOutlineColor(sf::Color::Black);
		rect.setFillColor(sf::Color::Yellow);
		rect.setPosition(0, -490);
		rect.setSize({400.f * this->_leftCharacter->_hp / 20000.f, 20});
		game.screen->draw(rect);
		rect.setPosition(1000 - 400.f * this->_rightCharacter->_hp / 20000.f, -490);
		rect.setSize({400.f * this->_rightCharacter->_hp / 20000.f, 20});
		game.screen->draw(rect);
	}

	void BattleManager::registerObject(IObject *object)
	{
		this->_objects.emplace_back(object);
	}

	void BattleManager::addHitStop(unsigned int stop)
	{
		this->_hitStop = std::max(stop, this->_hitStop);
	}
}