//
// Created by PinkySmile on 11/02/23.
//

#ifndef SOFGV_SCENEARGUMENT_HPP
#define SOFGV_SCENEARGUMENT_HPP


#include <functional>
#include <string>

namespace SpiralOfFate
{
	struct SceneArguments {
		std::function<void (const std::string &)> reportProgressA;
		std::function<void (const std::wstring &)> reportProgressW;

		virtual ~SceneArguments() = default;
	};

	struct GameStartParams {
		unsigned seed;
		unsigned p1chr;
		unsigned p1pal;
		unsigned p2chr;
		unsigned p2pal;
		unsigned stage;
		unsigned platformConfig;
	};

	struct TitleScreenArguments : public SceneArguments {
		std::string errorMessage;
	};
	struct InGameArguments : public SceneArguments {
		class Connection *connection;
		GameStartParams startParams;
		class IScene *currentScene;
	};
	struct CharSelectArguments : public SceneArguments {
		class Connection *connection;
		GameStartParams startParams;
		bool restore;
	};
}


#endif //SOFGV_SCENEARGUMENT_HPP
