//
// Created by PinkySmile on 11/02/23.
//

#include "Game.hpp"
#include "SceneManager.hpp"

namespace SpiralOfFate
{
	SceneManager::~SceneManager()
	{
		if (this->_loadingThread.joinable())
			this->_loadingThread.join();
	}

	void SceneManager::update()
	{
		if (this->_loading) {
			if (this->_scene)
				this->_scene->update();
			return;
		}
		this->_oldScene.reset();
		if (!this->_nextScene.name.empty() && this->_currentScene != this->_nextScene.name)
			this->_applySwitchScene(false);
		else if (this->_scene)
			this->_scene->update();
	}

	void SceneManager::render() const
	{
		assert_exp(this->_scene || this->_oldScene);
		(this->_scene ? this->_scene : this->_oldScene)->render();
	}

	void SceneManager::consumeEvent(const sf::Event &event)
	{
		assert_exp(this->_scene || this->_oldScene);
		(this->_scene ? this->_scene : this->_oldScene)->consumeEvent(event);
	}

	void SceneManager::switchScene(const std::string &name, SceneArguments *args, bool force)
	{
		if (!force)
			assert_exp(name != this->_currentScene);
		assert_msg(this->_factory.find(name) != this->_factory.end(), "Scene '" + name + "' doesn't exist");
		assert_exp(!this->_factory[name].needLoading || this->_factory.find("loading") != this->_factory.end());
		delete this->_nextScene.args;
		this->_nextScene.args = args;
		this->_nextScene.name = name;
		if (!this->_scene)
			this->_applySwitchScene(true);
	}

	SceneManager::SceneConstructor SceneManager::registerScene(
		const std::string &name,
		const std::function<IScene *(SceneArguments *)> &constructor,
		bool requireLoadingScene
	)
	{
		auto old = this->_factory[name];

		assert_exp(name != "loading" || !requireLoadingScene);
		this->_factory[name].callback = constructor;
		this->_factory[name].needLoading = requireLoadingScene;
		return old;
	}

	bool SceneManager::isLoading()
	{
		return this->_loading;
	}

	std::pair<std::string, IScene *> SceneManager::getCurrentScene()
	{
		return {
			this->_currentScene,
			&*this->_scene
		};
	}

	void SceneManager::_applySwitchScene(bool sync)
	{
		auto &factory = this->_factory[this->_nextScene.name];

		this->_loading = true;
		this->_oldScene = this->_scene;
		if (!factory.needLoading) {
			auto callback = [this, &factory]{
				this->_scene.reset(factory.callback(this->_nextScene.args));
				this->_currentScene = this->_nextScene.name;
				this->_loading = false;
			};

			this->_scene.reset();
			if (this->_loadingThread.joinable())
				this->_loadingThread.join();
			if (sync)
				callback();
			else
				this->_loadingThread = std::thread(callback);
		} else {
			LoadingArguments arg{factory.callback};

			arg.onLoadingFinished = [this](IScene *result){
				this->_oldScene = this->_scene;
				assert_exp(result);
				this->_scene.reset(result);
				this->_loading = false;
			};
			arg.args = this->_nextScene.args;
			this->_scene.reset(this->_factory["loading"].callback(&arg));
			this->_currentScene = this->_nextScene.name;
		}
		this->_nextScene.name.clear();
	}
}