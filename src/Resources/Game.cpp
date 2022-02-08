//
// Created by PinkySmile on 18/09/2021
//

#include "Game.hpp"

#include <memory>
#include "../Scenes/NetplayInGame.hpp"

namespace Battle
{
	Game game;

	namespace GGPONetplay
	{
		bool __cdecl startGame(const char *)
		{
			game.networkMgr.beginSession();
			return true;
		}

		bool __cdecl saveState(unsigned char **buffer, int *len, int *, int)
		{
			game.networkMgr.save(reinterpret_cast<void **>(buffer), len);
			return true;
		}

		bool __cdecl loadState(unsigned char *buffer, int)
		{
			game.networkMgr.load(buffer);
			return true;
		}

		bool __cdecl logState(char *, unsigned char *, int)
		{
			return true;
		}

		void __cdecl freeBuffer(void *buffer)
		{
			NetManager::free(buffer);
		}

		bool __cdecl updateGame(int)
		{
			game.networkMgr.advanceState();
			return true;
		}

		bool __cdecl onEvent(GGPOEvent *info)
		{
			game.networkMgr.consumeEvent(info);
			return true;
		}
	}
}