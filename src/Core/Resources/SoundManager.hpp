//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_SOUNDMANAGER_HPP
#define BATTLE_SOUNDMANAGER_HPP


#include <map>
#include <string>
#include <SFML/Audio.hpp>
#include "Sprite.hpp"
#include "../Data/Vector.hpp"
#include "../Data/Color.hpp"

namespace SpiralOfFate
{
	class SoundManager {
	private:
		mutable unsigned _lastSound = 0;
		mutable sf::Sound _sound[64];
		unsigned _lastIndex = 0;
		std::vector<unsigned> _freedIndexes;
		std::map<unsigned, sf::SoundBuffer> _sounds;
		std::map<std::string, std::pair<unsigned, unsigned>> _allocatedSounds;

	public:
		~SoundManager();
		unsigned load(std::string file);
		void addRef(unsigned id);
		void remove(unsigned id);
		void play(unsigned id) const;
		void setVolume(float volume);
	};
}


#endif //BATTLE_SOUNDMANAGER_HPP
