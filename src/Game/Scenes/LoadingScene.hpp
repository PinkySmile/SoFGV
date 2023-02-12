//
// Created by andgel on 03/12/22
//

#ifndef SOFGV_LOADINGSCENE_HPP
#define SOFGV_LOADINGSCENE_HPP


#include <mutex>
#include <functional>
#include "Resources/IScene.hpp"
#include "Resources/SceneArgument.hpp"

namespace SpiralOfFate
{
	class LoadingScene : public IScene {
	private:
		bool _errored = false;
		mutable std::mutex _mutex;
		sf::String _status;
		IScene *_nextScene = nullptr;
		std::function<void (IScene *)> onLoadingFinished;
		std::function<void (LoadingScene *)> onUpdate;

	public:
		LoadingScene(
			const std::function<IScene *(LoadingScene *me)> &fct,
			const std::function<void (IScene *)> &onLoadingFinished,
			const std::function<void (LoadingScene *)> &onUpdate = nullptr
		);
		~LoadingScene() override;
		void render() const override;
		void update() override;
		void setStatus(const std::string &status);
		void setStatus(const std::wstring &status);
		void consumeEvent(const sf::Event &event) override;

		static LoadingScene *create(SceneArguments *);
	};
}


#endif //SOFGV_LOADINGSCENE_HPP
