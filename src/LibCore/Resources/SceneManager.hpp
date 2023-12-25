//
// Created by PinkySmile on 11/02/23.
//

#ifndef SOFGV_SCENEMANAGER_HPP
#define SOFGV_SCENEMANAGER_HPP


#include <thread>
#include <functional>
#include "IScene.hpp"
#include "SceneArgument.hpp"

namespace SpiralOfFate
{
	struct LoadingArguments : public SceneArguments {
		std::function<void (IScene *)> onLoadingFinished;
		const std::function<IScene *(SceneArguments *args)> &fct;
		SceneArguments *args;
		IScene *loadingScene = nullptr;

		LoadingArguments(const std::function<IScene *(SceneArguments *args)> &fct) : fct(fct) {};
		~LoadingArguments() override = default;
	};

	class SceneManager {
	private:
		struct SceneInitializer {
			std::string name;
			SceneArguments *args = nullptr;

			~SceneInitializer() { delete this->args; }
		};
		struct SceneConstructor {
			std::function<IScene *(SceneArguments *args)> callback;
			bool needLoading;
		};

		bool _loading = false;
		std::string _currentScene;
		SceneInitializer _nextScene;
		std::shared_ptr<IScene> _oldScene;
		std::shared_ptr<IScene> _scene;
		std::map<std::string, SceneConstructor> _factory;
		std::thread _loadingThread;

		void _applySwitchScene(bool sync);

	public:
		~SceneManager();
		void update();
		void render() const;
		void consumeEvent(const sf::Event &event);
		void switchScene(const std::string &name, SceneArguments *args = nullptr, bool force = false);
		SceneConstructor registerScene(
			const std::string &name,
			const std::function<IScene *(SceneArguments *args)> &constructor,
			bool requireLoadingScene
		);
		std::pair<std::string, IScene *> getCurrentScene();
		bool isLoading();
	};
}


#endif //SOFGV_SCENEMANAGER_HPP
