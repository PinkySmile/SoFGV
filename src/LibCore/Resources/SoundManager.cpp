//
// Created by PinkySmile on 18/09/2021
//

#include "SoundManager.hpp"
#include "Game.hpp"
#include "../Logger.hpp"

namespace SpiralOfFate
{
	unsigned SoundManager::load(std::string file)
	{
		for (auto pos = file.find('\\'); pos != std::string::npos; pos = file.find('\\'))
			file[pos] = '/';
		if (this->_allocatedSounds[file].second != 0) {
			this->_allocatedSounds[file].second++;
			game->logger.debug("Returning already loaded file " + file);
			return this->_allocatedSounds[file].first;
		}

		unsigned index;

		if (this->_freedIndexes.empty()) {
			this->_lastIndex += this->_lastIndex == 0;
			index = this->_lastIndex;
			this->_lastIndex++;
		} else {
			index = this->_freedIndexes.back();
			this->_freedIndexes.pop_back();
		}

		game->logger.debug("Loading file " + file);
		if (!this->_sounds[index].loadFromFile(file)) {
			this->_freedIndexes.push_back(index);
			return 0;
		}

		this->_allocatedSounds[file].first = index;
		this->_allocatedSounds[file].second = 1;
		return index;
	}

	void SoundManager::remove(unsigned int id)
	{
		if (!id)
			return;

		for (auto &[loadedPath, attr] : this->_allocatedSounds)
			if (attr.first == id && attr.second) {
				attr.second--;
				if (attr.second) {
					game->logger.debug("Remove ref to " + loadedPath);
					return;
				}
				game->logger.debug("Destroying sound " + loadedPath);
				break;
			}

		auto it = this->_sounds.find(id);

		if (it == this->_sounds.end())
			return game->logger.warn("Trying to remove invalid index " + std::to_string(id));
		this->_sounds.erase(it);
		this->_freedIndexes.push_back(id);
	}

	void SoundManager::play(unsigned id) const
	{
		if (!id)
			return;
		game->logger.debug("Playing sound " + std::to_string(id));

		auto &sound = this->_sound[this->_lastSound];

		try {
			sound.setBuffer(this->_sounds.at(id));
			sound.play();
			this->_lastSound++;
			this->_lastSound %= 64;
		} catch (...) {}
	}

	void SoundManager::addRef(unsigned int id)
	{
		if (id == 0)
			return;
		for (auto &[loadedPath, attr] : this->_allocatedSounds)
			if (attr.first == id && attr.second) {
				if (attr.second < 1)
					return game->logger.warn("Cannot add ref to " + loadedPath + " (" + std::to_string(id) + ") because it was unloaded");
				game->logger.debug("Adding ref to " + loadedPath);
				attr.second++;
				return;
			}
		game->logger.warn("Cannot add ref to unknown sound " + std::to_string(id));
	}

	void SoundManager::setVolume(float volume)
	{
		for (auto &sound : this->_sound)
			sound.setVolume(volume);
	}

	SoundManager::~SoundManager()
	{
		game->logger.debug("~SoundManager()");
	}
}
