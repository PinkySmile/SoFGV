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
		this->_leftCharacter->init(
			true,
			leftCharacter.hp,
			leftCharacter.maxJumps,
			leftCharacter.maxAirDash,
			leftCharacter.matterManaMax,
			leftCharacter.voidManaMax,
			leftCharacter.spiritManaMax,
			leftCharacter.manaRegen,
			leftCharacter.gravity
		);
		this->_rightCharacter->init(
			false,
			rightCharacter.hp,
			rightCharacter.maxJumps,
			rightCharacter.maxAirDash,
			rightCharacter.matterManaMax,
			rightCharacter.voidManaMax,
			rightCharacter.spiritManaMax,
			rightCharacter.manaRegen,
			rightCharacter.gravity
		);
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
			this->_leftCharacter->updateInputs();
			this->_rightCharacter->updateInputs();
			return;
		}

		auto ldata = this->_leftCharacter->getCurrentFrameData();
		auto rdata = this->_rightCharacter->getCurrentFrameData();
		auto lchr = &*this->_leftCharacter;
		auto rchr = &*this->_rightCharacter;

		if (!rdata->dFlag.flash || ldata->dFlag.flash)
			lchr->update();
		if (!ldata->dFlag.flash)
			rchr->update();

		if (!ldata->dFlag.flash && !rdata->dFlag.flash) {
			for (auto &object : this->_objects)
				object->update();
			if (lchr->hits(*rchr))
				collisions.emplace_back(&*lchr, &*rchr, lchr->getCurrentFrameData());
			if (rchr->hits(*lchr))
				collisions.emplace_back(&*rchr, &*lchr, rchr->getCurrentFrameData());

			for (auto &object : this->_objects) {
				if (lchr->hits(*object))
					collisions.emplace_back(&*lchr, &*object, lchr->getCurrentFrameData());
				if (object->hits(*lchr))
					collisions.emplace_back(&*object, &*lchr, object->getCurrentFrameData());

				if (rchr->hits(*object))
					collisions.emplace_back(&*rchr, &*object, rchr->getCurrentFrameData());
				if (object->hits(*rchr))
					collisions.emplace_back(&*object, &*rchr, object->getCurrentFrameData());

				for (auto &object2 : this->_objects)
					if (object2 != object)
						if (object->hits(*object2))
							collisions.emplace_back(&*object, &*object2, object->getCurrentFrameData());
			}

			for (auto &[attacker, defender, data] : collisions) {
				attacker->hit(*defender, data);
				defender->getHit(*attacker, data);
			}
		}

		if (lchr->isDead())
			// The state is messed up
			// TODO: Do real exceptions
			throw std::invalid_argument("Invalid state");
		if (rchr->isDead())
			// The state is messed up
			// TODO: Do real exceptions
			throw std::invalid_argument("Invalid state");

		for (unsigned i = 0; i < this->_objects.size(); i++)
			if (this->_objects[i]->isDead())
				this->_objects.erase(this->_objects.begin() + i--);

		if (lchr->collides(*rchr))
			lchr->collide(*rchr);
		for (auto &object : this->_objects) {
			if (lchr->collides(*object))
				lchr->collide(*object);
			if (rchr->collides(*object))
				rchr->collide(*object);
		}
		lchr->postUpdate();
		rchr->postUpdate();
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
		rect.setSize({400.f * this->_leftCharacter->_hp / this->_leftCharacter->_baseHp, 20});
		game.screen->draw(rect);
		rect.setPosition(1000 - 400.f * this->_rightCharacter->_hp / this->_rightCharacter->_baseHp, -490);
		rect.setSize({400.f * this->_rightCharacter->_hp / this->_rightCharacter->_baseHp, 20});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color::Cyan);
		rect.setPosition(100, 40);
		rect.setSize({200.f * this->_leftCharacter->_spiritMana / this->_leftCharacter->_spiritManaMax, 10});
		game.screen->draw(rect);
		rect.setPosition(900 - 200.f * this->_rightCharacter->_spiritMana / this->_rightCharacter->_spiritManaMax, 40);
		rect.setSize({200.f * this->_rightCharacter->_spiritMana / this->_rightCharacter->_spiritManaMax, 10});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color{0xFF, 0x40, 0x40});
		rect.setPosition(100, 55);
		rect.setSize({200.f * this->_leftCharacter->_matterMana / this->_leftCharacter->_matterManaMax, 10});
		game.screen->draw(rect);
		rect.setPosition(900 - 200.f * this->_rightCharacter->_matterMana / this->_rightCharacter->_matterManaMax, 55);
		rect.setSize({200.f * this->_rightCharacter->_matterMana / this->_rightCharacter->_matterManaMax, 10});
		game.screen->draw(rect);

		rect.setFillColor(sf::Color{0x80, 0x00, 0x80});
		rect.setPosition(100, 70);
		rect.setSize({200.f * this->_leftCharacter->_voidMana / this->_leftCharacter->_voidManaMax, 10});
		game.screen->draw(rect);
		rect.setPosition(900 - 200.f * this->_rightCharacter->_voidMana / this->_rightCharacter->_voidManaMax, 70);
		rect.setSize({200.f * this->_rightCharacter->_voidMana / this->_rightCharacter->_voidManaMax, 10});
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