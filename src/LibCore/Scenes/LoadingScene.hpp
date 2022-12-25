//
// Created by andgel on 03/12/22
//

#ifndef SOFGV_LOADINGSCENE_HPP
#define SOFGV_LOADINGSCENE_HPP


#include <mutex>
#include <functional>
#include "IScene.hpp"

namespace SpiralOfFate
{
	class LoadingScene : public IScene {
	private:
		bool _errored = false;
		mutable std::mutex _mutex;
		sf::String _status;
		IScene *_nextScene = nullptr;
		std::function<void (LoadingScene *)> onUpdate;

	public:
		LoadingScene(const std::function<IScene *(LoadingScene *me)> &fct, const std::function<void (LoadingScene *)> &onUpdate = nullptr);
		~LoadingScene() override;
		void render() const override;
		IScene *update() override;
		void setStatus(const std::string &status);
		void setStatus(const std::wstring &status);
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //SOFGV_LOADINGSCENE_HPP
