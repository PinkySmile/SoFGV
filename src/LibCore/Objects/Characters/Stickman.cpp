//
// Created by PinkySmile on 01/03/2022.
//

#include "Stickman.hpp"
#include "Resources/Game.hpp"
#include "Resources/Battle/PracticeBattleManager.hpp"

#define DAMAGE_DIFF 0.5
#define HITSTUN_DIFF 0.25
#define MANA_COST_DIFF 0.25
#define PRORATION_DIFF 0.95
#define SPEED_DIFF 0.1

namespace SpiralOfFate
{
	Stickman::Stickman(
		unsigned index,
		const std::string &folder,
		const std::pair<std::vector<SpiralOfFate::Color>,
		std::vector<SpiralOfFate::Color>> &palette,
		std::shared_ptr<IInput> input
	) :
		Character(index, folder, palette, input)
	{
		this->_fakeFrameData.setSlave();
		game->logger.debug("Stickman class created");
	}

	unsigned int Stickman::getClassId() const
	{
		return 3;
	}

	const FrameData *Stickman::getCurrentFrameData() const
	{
		auto data = Character::getCurrentFrameData();

		if (!this->_buffTimer)
			return data;
		this->_fakeFrameData = *data;
		this->_allyBuffEffect(this->_fakeFrameData);
		return &this->_fakeFrameData;
	}

	void Stickman::_forceStartMove(unsigned int action)
	{
		Character::_forceStartMove(action);
		if (action == ACTION_5A && this->_buffTimer) {
			this->_actionBlock = 1;
			this->_applyNewAnimFlags();
		}
	}

	void Stickman::_applyMoveAttributes()
	{
		Character::_applyMoveAttributes();
		if (this->_action >= ACTION_5A && this->_action <= ACTION_c64A) {
			auto data = this->getCurrentFrameData();

			if (data->specialMarker == 1) {
				this->_buff = random_distrib(game->battleRandom, 0, timers.size());
				this->_buffTimer = timers[this->_buff];
				this->_guardCooldown = 300;
			} else if (data->specialMarker == 2) {
				if (this->_buffTimer)
					this->_guardCooldown += 300;
				this->_buffTimer = 0;
			}
		}
	}

	bool Stickman::_canStartMove(unsigned int action, const FrameData &data)
	{
		if (action == ACTION_5A)
			return (this->_guardCooldown == 0 || this->_buffTimer != 0) && this->_action != ACTION_5A;
		return Character::_canStartMove(action, data);
	}

	void Stickman::_onMoveEnd(const FrameData &lastData)
	{
		if (this->_action == ACTION_WIN_MATCH2 && this->_actionBlock == 1) {
			this->_actionBlock++;
			assert(this->_moves.at(this->_action).size() != this->_actionBlock);
			return Character::_onMoveEnd(lastData);
		}
		Character::_onMoveEnd(lastData);
	}

	void Stickman::update()
	{
		Character::update();
		if (!dynamic_cast<PracticeBattleManager *>(&*game->battleMgr))
			this->_buffTimer -= !!this->_buffTimer;
	}

	unsigned int Stickman::getBufferSize() const
	{
		return Character::getBufferSize() + sizeof(Data);
	}

	void Stickman::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());

		Character::copyToBuffer(data);
		game->logger.verbose("Saving Stickman (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
		dat->_buff = this->_buff;
		dat->_time = this->_time;
		dat->_oldAction = this->_oldAction;
		dat->_buffTimer = this->_buffTimer;
	}

	void Stickman::restoreFromBuffer(void *data)
	{
		Character::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());

		this->_buff = dat->_buff;
		this->_time = dat->_time;
		this->_oldAction = dat->_oldAction;
		this->_buffTimer = dat->_buffTimer;
		game->logger.verbose("Restored Stickman @" + std::to_string((uintptr_t)dat));
	}

	void Stickman::onMatchEnd()
	{
		Character::onMatchEnd();
		this->_oldAction = this->_action;
		if (this->_oldAction <= ACTION_WIN_MATCH2)
			game->soundMgr.play(BASICSOUND_DASH);
	}

	bool Stickman::matchEndUpdate()
	{
		if (this->_oldAction < ACTION_WIN_MATCH1)
			return false;
		if (this->_action < ACTION_WIN_MATCH1)
			this->_forceStartMove(this->_oldAction);
		if (!this->_isGrounded())
			this->_speed.x = 0;
		if (std::abs(this->_position.x - reinterpret_cast<Stickman *>(&*this->_opponent)->_position.x) < 30 && this->_actionBlock == 0) {
			this->_actionBlock++;
			this->_animation = 0;
			this->_animationCtr = 0;
			this->_speed.x = 0;
		}
		this->_time += this->_actionBlock;
		return this->_time < 30;
	}

	void Stickman::_mutateHitFramedata(FrameData &framedata) const
	{
		this->_enemyBuffEffect(framedata);
	}

	void Stickman::_allyBuffEffect(FrameData &framedata) const
	{
		if (this->_buffTimer == 0)
			return;
		switch (this->_buff) {
		case BUFFTYPE_GUARD_BAR_IGNORED:
			framedata.guardDmg = 0;
			return;
		case BUFFTYPE_INSTA_CRUSH:
			framedata.guardDmg = UINT32_MAX;
			return;
		case BUFFTYPE_PLUS_DAMAGE:
			framedata.damage *= 1 + DAMAGE_DIFF;
			return;
		case BUFFTYPE_MINUS_DAMAGE:
			framedata.damage /= 1 + DAMAGE_DIFF;
			return;
		case BUFFTYPE_PLUS_HITSTUN:
			framedata.hitStun *= 1 + HITSTUN_DIFF;
			framedata.untech *= 1 + HITSTUN_DIFF;
			return;
		case BUFFTYPE_MINUS_HITSTUN:
			framedata.hitStun /= 1 + HITSTUN_DIFF;
			framedata.untech /= 1 + HITSTUN_DIFF;
			return;
		case BUFFTYPE_PLUS_MANA_COST:
			framedata.manaCost *= 1 + MANA_COST_DIFF;
			return;
		case BUFFTYPE_MINUS_MANA_COST:
			framedata.manaCost /= 1 + MANA_COST_DIFF;
			return;
		case BUFFTYPE_PLUS_PRORATION:
			framedata.prorate *= 0.95;
			return;
		case BUFFTYPE_PLUS_SPEED:
			framedata.speed.x *= 1 + SPEED_DIFF;
			return;
		case BUFFTYPE_MINUS_SPEED:
			framedata.speed.x /= 1 + SPEED_DIFF * 2;
			return;
		}
	}

	void Stickman::_enemyBuffEffect(FrameData &framedata) const
	{
		this->_allyBuffEffect(framedata);
	}

	size_t Stickman::printDifference(const char *msgStart, void *data1, void *data2) const
	{
		auto length = Character::printDifference(msgStart, data1, data2);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>((uintptr_t)data1 + length);
		auto dat2 = reinterpret_cast<Data *>((uintptr_t)data2 + length);

		if (dat1->_buff != dat2->_buff)
			game->logger.fatal(std::string(msgStart) + "Stickman::_buff: " + std::to_string(dat1->_buff) + " vs " + std::to_string(dat2->_buff));
		if (dat1->_time != dat2->_time)
			game->logger.fatal(std::string(msgStart) + "Stickman::_time: " + std::to_string(dat1->_time) + " vs " + std::to_string(dat2->_time));
		if (dat1->_oldAction != dat2->_oldAction)
			game->logger.fatal(std::string(msgStart) + "Stickman::_oldAction: " + std::to_string(dat1->_oldAction) + " vs " + std::to_string(dat2->_oldAction));
		if (dat1->_buffTimer != dat2->_buffTimer)
			game->logger.fatal(std::string(msgStart) + "Stickman::_buffTimer: " + std::to_string(dat1->_buffTimer) + " vs " + std::to_string(dat2->_buffTimer));
		return length + sizeof(Data);
	}

	void Stickman::render() const
	{
		Character::render();
		if (/*this->showBoxes && */this->_buffTimer) {
			game->screen->displayElement(
				{static_cast<int>(this->_position.x - this->_buffTimer / 4), -static_cast<int>(this->_position.y), static_cast<int>(this->_buffTimer / 2), 10},
				sf::Color::Green
			);
			game->screen->fillColor(sf::Color::Black);
			game->screen->textSize(10);
			game->screen->displayElement(
				buffName[this->_buff],
				{static_cast<float>(this->_position.x - 50), -static_cast<float>(this->_position.y)},
				100, Screen::ALIGN_CENTER
			);
			game->screen->textSize(30);
		}
	}
}
