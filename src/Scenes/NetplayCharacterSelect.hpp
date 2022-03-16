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
#pragma pack(push, 1)
		struct Data {
			int _leftPos;
			int _rightPos;
			int _leftPalette;
			int _rightPalette;
			int _stage;
			int _platform;
			bool _selectingStage;
		};
#pragma pack(pop)

		void _saveState(void *data, int *len) override;
		void _loadState(void *data) override;
		IScene *_realUpdate() override;

	protected:
		InGame *_launchGame() override;

	public:
		NetplayCharacterSelect();
		~NetplayCharacterSelect() override;
		IScene *update() override;

		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_NETPLAYCHARACTERSELECT_HPP
