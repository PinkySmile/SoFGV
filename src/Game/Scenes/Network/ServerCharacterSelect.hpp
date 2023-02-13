//
// Created by PinkySmile on 26/08/2022.
//

#ifndef SOFGV_SERVERCHARACTERSELECT_HPP
#define SOFGV_SERVERCHARACTERSELECT_HPP


#include <memory>
#include "NetworkCharacterSelect.hpp"

// I took examples on the bests

namespace SpiralOfFate
{
	class ServerCharacterSelect : public NetworkCharacterSelect {
	protected:
		void _launchGame() override;

	public:
		ServerCharacterSelect();
		ServerCharacterSelect(
			int leftPos, int rightPos,
			int leftPalette, int rightPalette,
			int stage, int platformCfg
		);

		void consumeEvent(const sf::Event &event) override;

		static ServerCharacterSelect *create(SceneArguments *);

		friend class ServerConnection;
	};
}


#endif //SOFGV_SERVERCHARACTERSELECT_HPP
