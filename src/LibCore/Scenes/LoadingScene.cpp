//
// Created by PinkySmile on 03/12/22
//

#include "LoadingScene.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	LoadingScene::LoadingScene(const std::function<IScene *(LoadingScene *)> &fct, const std::function<void (LoadingScene *)> &onUpdate) :
		onUpdate(onUpdate)
	{
		auto fctCopy = new std::function<IScene *(LoadingScene *)>(fct);

		std::thread{[this, fctCopy]{
			auto val = (*fctCopy)(this);

			this->setStatus("Cleaning up...");
			delete fctCopy;
			this->_nextScene = val;
		}}.detach();
	}

	LoadingScene::~LoadingScene()
	{
		game->logger.debug("~LoadingScene()");
	}

	void SpiralOfFate::LoadingScene::render() const
	{
		sf::View view{{0, 0, 1680, 960}};

		game->screen->setView(view);
		game->screen->clear(sf::Color{0xA0, 0xA0, 0xA0, 0xFF});
		game->screen->borderColor(2, sf::Color::Black);
		game->screen->fillColor();
		game->screen->textSize(50);
		game->screen->displayElement("Loading...", {0, 440}, 1680, Screen::ALIGN_CENTER);
		this->_mutex.lock();
		game->screen->displayElement(this->_status, {0, 490}, 1680, Screen::ALIGN_CENTER);
		this->_mutex.unlock();
		game->screen->borderColor();
		game->screen->textSize(30);
	}

	SpiralOfFate::IScene *SpiralOfFate::LoadingScene::update()
	{
		if (this->onUpdate)
			this->onUpdate(this);
		return this->_nextScene;
	}

	void SpiralOfFate::LoadingScene::consumeEvent(const sf::Event &event)
	{
		game->P1.first->consumeEvent(event);
		game->P2.first->consumeEvent(event);
		game->P1.second->consumeEvent(event);
		game->P2.second->consumeEvent(event);
	}

	void LoadingScene::setStatus(const std::string &status)
	{
		this->_mutex.lock();
		this->_status = status;
		this->_mutex.unlock();
	}
}