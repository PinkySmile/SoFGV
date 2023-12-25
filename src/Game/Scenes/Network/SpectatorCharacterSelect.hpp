//
// Created by PinkySmile on 03/12/22
//

#ifndef SOFGV_SPECTATORCHARACTERSELECT_HPP
#define SOFGV_SPECTATORCHARACTERSELECT_HPP


#include <mutex>
#include <functional>
#include "Resources/IScene.hpp"
#include "Resources/SceneArgument.hpp"

namespace SpiralOfFate
{
	class SpectatorCharacterSelect : public IScene {
	private:
		SpectatorConnection &connection;

	public:
		SpectatorCharacterSelect(SpectatorConnection &connection);
		~SpectatorCharacterSelect() override;
		void render() const override;
		void update() override;
		void consumeEvent(const sf::Event &event) override;

		static SpectatorCharacterSelect *create(SceneArguments *);
	};
}


#endif //SOFGV_SPECTATORCHARACTERSELECT_HPP
