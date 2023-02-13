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
}


#endif //SOFGV_SCENEARGUMENT_HPP
