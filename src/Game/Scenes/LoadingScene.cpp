//
// Created by PinkySmile on 03/12/22
//

#include "LoadingScene.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	LoadingScene::LoadingScene(
		const std::function<IScene *(LoadingScene *)> &fct,
		const std::function<void (IScene *)> &onLoadingFinished,
		const std::function<void (LoadingScene *)> &onUpdate
	) :
		onLoadingFinished(onLoadingFinished),
		onUpdate(onUpdate)
	{
		auto fctCopy = new std::function<IScene *(LoadingScene *)>(fct);

		std::thread{[this, fctCopy]{
#if !defined(_DEBUG) || defined(_WIN32) || defined(__ANDROID__)
			try {
#endif
				auto val = (*fctCopy)(this);

				this->setStatus("Cleaning up...");
				delete fctCopy;
				this->_nextScene = val;
#if !defined(_DEBUG) || defined(_WIN32) || defined(__ANDROID__)
			} catch (std::exception &e) {
				this->_errored = true;
				this->setStatus(e.what());
				game->logger.error(e.what());
				//TODO: What to here ?
			}
#endif
		}}.detach();
	}

	LoadingScene::~LoadingScene()
	{
		game->logger.debug("~LoadingScene()");
	}

	void SpiralOfFate::LoadingScene::render() const
	{
		ViewPort view{{0, 0, 1680, 960}};

		game->screen->setView(view);
		game->screen->clear({0xA0, 0xA0, 0xA0});
		game->screen->borderColor(2, sf::Color::Black);
		game->screen->fillColor();
		game->screen->textSize(50);
		game->screen->displayElement(this->_errored ? "Error." : "Loading...", {0, 440}, 1680, Screen::ALIGN_CENTER);
		this->_mutex.lock();
		game->screen->displayElement(this->_status, {0, 490}, 1680, Screen::ALIGN_CENTER);
		this->_mutex.unlock();
		game->screen->borderColor();
		game->screen->textSize(30);
	}

	void SpiralOfFate::LoadingScene::update()
	{
		if (this->onUpdate)
			this->onUpdate(this);
		if (this->_nextScene) {
			this->onLoadingFinished(this->_nextScene);
			this->_nextScene = nullptr;
		}
	}

	void SpiralOfFate::LoadingScene::consumeEvent(const sf::Event &event)
	{
		game->P1.first->consumeEvent(event);
		game->P2.first->consumeEvent(event);
		game->P1.second->consumeEvent(event);
		game->P2.second->consumeEvent(event);
	}

	void LoadingScene::setStatus(const std::wstring &status)
	{
		this->_mutex.lock();
		this->_status = status;
		this->_mutex.unlock();
	}

	void LoadingScene::setStatus(const std::string &status)
	{
		this->_mutex.lock();
		this->_status = status;
		this->_mutex.unlock();
	}

	LoadingScene *LoadingScene::create(SceneArguments *args)
	{
		checked_cast(arg, LoadingArguments, args);

		// We copy the arguments because we will run everything in a thread.
		auto &fct = arg->fct;
		auto realArgs = arg->args;

		return new LoadingScene([&fct, realArgs](LoadingScene *me){
			if (realArgs) {
				realArgs->reportProgressA = [me](const std::string &t) {
					me->setStatus(t);
				};
				realArgs->reportProgressW = [me](const std::wstring &t) {
					me->setStatus(t);
				};
			}
			return fct(realArgs);
		}, arg->onLoadingFinished);
	}
}