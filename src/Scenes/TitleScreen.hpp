//
// Created by Gegel85 on 24/09/2021.
//

#ifndef BATTLE_TITLESCREEN_HPP
#define BATTLE_TITLESCREEN_HPP


#include <SFML/Graphics/Font.hpp>
#include "IScene.hpp"

namespace Battle
{
	class TitleScreen : public IScene {
	private:
		sf::Font _font;
		unsigned _selectedEntry = 0;
		IScene *_nextScene = nullptr;

		void _onConfirm();
		void _host();
		void _connect();

	public:
		TitleScreen();
		void render() const override;
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_TITLESCREEN_HPP
