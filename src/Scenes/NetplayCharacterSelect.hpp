//
// Created by Gegel85 on 07/02/2022.
//

#ifndef BATTLE_NETPLAYCHARACTERSELECT_HPP
#define BATTLE_NETPLAYCHARACTERSELECT_HPP


#include "CharacterSelect.hpp"
#include "NetplayScene.hpp"

namespace Battle
{
	class NetplayCharacterSelect : public CharacterSelect, public NetplayScene {
	private:
		void _saveState(void *data, int *len) override;
		void _loadState(void *data) override;
		IScene *_realUpdate() override;

	public:
		NetplayCharacterSelect();
		~NetplayCharacterSelect();
		IScene *update() override;

		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_NETPLAYCHARACTERSELECT_HPP
