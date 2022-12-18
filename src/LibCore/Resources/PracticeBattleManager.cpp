//
// Created by PinkySmile on 18/02/2022.
//

#include "PracticeBattleManager.hpp"
#include "Game.hpp"

namespace SpiralOfFate
{
	PracticeBattleManager::PracticeBattleManager(const StageParams &stage, const CharacterParams &leftCharacter, const CharacterParams &rightCharacter) :
		BattleManager(stage, leftCharacter, rightCharacter)
	{
		this->_startingState = new unsigned char[this->getBufferSize()];
		this->copyToBuffer(this->_startingState);
	}

	PracticeBattleManager::~PracticeBattleManager()
	{
		delete[] this->_savedState;
		delete[] this->_startingState;
	}

	bool PracticeBattleManager::_updateLoop()
	{
		bool b = BattleManager::_updateLoop();

		this->_updateFrameStuff();
		return b;
	}

	void PracticeBattleManager::render()
	{
		this->_leftCharacter->showBoxes = this->_showBoxes;
		this->_rightCharacter->showBoxes = this->_showBoxes;
		for (auto &platform : this->_platforms)
			platform->showBoxes = this->_showBoxes;
		for (auto &obj : this->_objects)
			obj.second->showBoxes = this->_showBoxes;
		for (auto &obj : this->_stageObjects)
			obj->showBoxes = this->_showBoxes;
		BattleManager::render();
		if (this->_showBoxes) {
			game->screen->borderColor(2, sf::Color::White);
			game->screen->displayElement({0, 0, 1000, -1000}, sf::Color::Transparent);
			game->screen->borderColor(0, sf::Color::Transparent);
		}
		this->_displayFrameStuff();
	}

	void PracticeBattleManager::consumeEvent(const sf::Event &event)
	{
		BattleManager::consumeEvent(event);
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F11)
			this->_step = !this->_step;
#ifdef _DEBUG
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F2)
#else
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F12)
#endif
			this->_next = true;
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F9)
			this->_speed--;
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F10)
			this->_speed++;
		if (!this->replay) {
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F8 && this->_savedState)
				this->restoreFromBuffer(this->_savedState);
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F7) {
				delete[] this->_savedState;
				this->_savedState = new unsigned char[this->getBufferSize()];
				this->copyToBuffer(this->_savedState);
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F6)
				this->restoreFromBuffer(this->_startingState);
		}
	}

	void PracticeBattleManager::_updateFrameStuff()
	{
		PracticeBattleManager::_updateGapState(this->_left,  *this->_leftCharacter, *this->_rightCharacter);
		PracticeBattleManager::_updateGapState(this->_right, *this->_rightCharacter, *this->_leftCharacter);
	}

	void PracticeBattleManager::_displayFrameStuff() const
	{
		PracticeBattleManager::_renderGapState(this->_left,  {150, 0});
		PracticeBattleManager::_renderGapState(this->_right, {750, 0});
	}

	void PracticeBattleManager::_updateGapState(BlockingState &state, const Character &me, const Character &op)
	{
		bool atk = PracticeBattleManager::_isAttacking(me);
		bool blk = PracticeBattleManager::_isBlocking(op);

		if (state.fa.timer < 240)
			state.fa.timer++;

		auto it = state.gaps.begin();

		for (; it != state.gaps.end(); it++)
			it->timer++;
		while (!state.gaps.empty() && state.gaps.back().timer >= 240)
			state.gaps.pop_back();

		if (PracticeBattleManager::_canMashNextFrame(op))
			state.gapTimer++;
		else if (!state.blk && blk) {
			if (state.gapTimer < 20 && state.gapTimer)
				state.gaps.push_front({0, static_cast<int>(state.gapTimer)});
			state.gapTimer = 0;
		}

		if (!blk && atk)
			state.faTimer--;
		else if (!blk) {
			if (state.atk || state.blk) {
				if (state.hasBlocked) {
					state.fa.gap = state.faTimer;
					state.fa.timer = 0;
				}
				state.faTimer = 0;
			}
			state.hasBlocked = false;
		} else if (!atk)
			state.faTimer++;
		else
			state.faTimer = 0;
		state.atk = atk;
		state.blk = blk;
		state.hasBlocked |= blk;
	}

	bool PracticeBattleManager::_isAttacking(const Character &me)
	{
		return me._action >= ACTION_5N || (me._action <= ACTION_AIR_REVERSAL && me._action >= ACTION_GROUND_HIGH_REVERSAL);
	}

	bool PracticeBattleManager::_isBlocking(const Character &me)
	{
		if (me._blockStun)
			return true;

		switch (me._action) {
		case ACTION_BEING_KNOCKED_DOWN:
		case ACTION_KNOCKED_DOWN:
		case ACTION_NEUTRAL_TECH:
		case ACTION_BACKWARD_TECH:
		case ACTION_FORWARD_TECH:
		case ACTION_FALLING_TECH:
		case ACTION_UP_AIR_TECH:
		case ACTION_DOWN_AIR_TECH:
		case ACTION_FORWARD_AIR_TECH:
		case ACTION_BACKWARD_AIR_TECH:
		case ACTION_AIR_TECH_LANDING_LAG:
		case ACTION_AIR_HIT:
			return true;
		default:
			return false;
		}
	}

	void PracticeBattleManager::_renderGapState(const BlockingState &state, Vector2f pos)
	{
		game->screen->textSize(15);
		if (state.fa.timer < 240) {
			float alpha = state.fa.timer < 120 ? 1 : (240 - state.fa.timer) / 120.f;

			game->screen->displayElement({static_cast<int>(pos.x), static_cast<int>(pos.y), 100, 20}, sf::Color{0xA0, 0xA0, 0xA0, static_cast<unsigned char>(0xA0 * alpha)});
			game->screen->fillColor(sf::Color{0xFF, 0xFF, 0xFF, static_cast<unsigned char>(0xFF * alpha)});
			game->screen->displayElement((state.fa.gap >= 0 ? "Adv: +" : "Adv: ") + std::to_string(state.fa.gap) + "F", pos + Vector2f{2, 2});
			pos.y -= 25;
		}
		for (auto &gap : state.gaps) {
			float alpha = gap.timer < 120 ? 1 : (240 - gap.timer) / 120.f;

			game->screen->displayElement({static_cast<int>(pos.x), static_cast<int>(pos.y), 100, 20}, sf::Color{0xA0, 0xA0, 0xA0, static_cast<unsigned char>(0xA0 * alpha)});
			game->screen->fillColor(sf::Color{0xFF, 0xFF, 0xFF, static_cast<unsigned char>(0xFF * alpha)});
			game->screen->displayElement("Gap: " + std::to_string(gap.gap) + "F", pos + Vector2f{2, 2});
			pos.y -= 25;
		}
		game->screen->fillColor(sf::Color::White);
		game->screen->textSize(30);
	}

	bool PracticeBattleManager::_canMashNextFrame(const SpiralOfFate::Character &me)
	{
		switch (me._action) {
		case ACTION_GROUND_HIGH_NEUTRAL_BLOCK:
		case ACTION_GROUND_HIGH_HIT:
		case ACTION_GROUND_LOW_NEUTRAL_BLOCK:
		case ACTION_GROUND_LOW_HIT:
		case ACTION_AIR_NEUTRAL_BLOCK:
		case ACTION_AIR_HIT:
			return me._blockStun == 1;
		}
		return true;
	}

	bool PracticeBattleManager::update()
	{
		if (this->_step && !this->_next)
			return true;
		this->_next = false;

		this->_time += this->_speed / 60.f;
		while (this->_time >= 1) {
			this->_time -= 1;
			if (!this->_updateLoop())
				return false;
		}
		return true;
	}
}
