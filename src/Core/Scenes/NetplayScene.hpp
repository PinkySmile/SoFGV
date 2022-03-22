//
// Created by Gegel85 on 07/02/2022.
//

#ifndef BATTLE_NETPLAYSCENE_HPP
#define BATTLE_NETPLAYSCENE_HPP


#include "IScene.hpp"

namespace Battle
{
	class NetplayScene {
	private:
		virtual void _saveState(void *data, int *len) = 0;
		virtual void _loadState(void *data) = 0;
		virtual IScene *_realUpdate() = 0;

		friend class NetManager;
	};
}


#endif //BATTLE_NETPLAYSCENE_HPP
