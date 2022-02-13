//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_BATTLEMANAGER_HPP
#define BATTLE_BATTLEMANAGER_HPP


#include "../Objects/ACharacter.hpp"

namespace Battle
{
	class BattleManager {
	private:
#pragma pack(push, 1)
		struct Data {
			std::pair<unsigned char, unsigned char> _score;
			unsigned _lastObjectId;
			unsigned _currentRound;
			int _roundStartTimer;
			unsigned _roundEndTimer;
			unsigned _hitStop;
			unsigned _nbObjects;
			unsigned _leftComboCtr = 0;
			unsigned _leftHitCtr = 0;
			unsigned _leftNeutralLimit = 0;
			unsigned _leftVoidLimit = 0;
			unsigned _leftMatterLimit = 0;
			unsigned _leftSpiritLimit = 0;
			unsigned _leftTotalDamage = 0;
			float _leftProration = 0;
			unsigned _rightComboCtr = 0;
			unsigned _rightHitCtr = 0;
			unsigned _rightNeutralLimit = 0;
			unsigned _rightVoidLimit = 0;
			unsigned _rightSpiritLimit = 0;
			unsigned _rightMatterLimit = 0;
			unsigned _rightTotalDamage = 0;
			float _rightProration = 0;
		};
#pragma pack(pop)

		// Non Game State
		sf::Clock _tpsClock;
		std::list<unsigned> _tpsTimes;
		mutable sf::Clock _fpsClock;
		mutable std::list<unsigned> _fpsTimes;
		Sprite _stage;
		sf::Sprite _roundSprite;
		std::vector<sf::Texture> _roundSprites;
		bool _step = false;
		bool _next = false;
		unsigned char _speed = 60;
		float _time = 0;
		unsigned _lastObjectId = 0;

		// Game State
		std::unique_ptr<ACharacter> _leftCharacter;
		std::unique_ptr<ACharacter> _rightCharacter;
		std::vector<std::pair<unsigned, std::shared_ptr<IObject>>> _objects;
		std::pair<unsigned char, unsigned char> _score{0, 0};
		unsigned _currentRound = 0;
		int _roundStartTimer = 0;
		unsigned _roundEndTimer = 0;
		unsigned _hitStop = 0;
		unsigned _leftComboCtr = 0;
		unsigned _leftHitCtr = 0;
		unsigned _leftNeutralLimit = 0;
		unsigned _leftVoidLimit = 0;
		unsigned _leftMatterLimit = 0;
		unsigned _leftSpiritLimit = 0;
		unsigned _leftTotalDamage = 0;
		float _leftProration = 0;
		unsigned _rightComboCtr = 0;
		unsigned _rightHitCtr = 0;
		unsigned _rightNeutralLimit = 0;
		unsigned _rightVoidLimit = 0;
		unsigned _rightSpiritLimit = 0;
		unsigned _rightMatterLimit = 0;
		unsigned _rightTotalDamage = 0;
		float _rightProration = 0;

		void _gameUpdate();
		bool _updateLoop();
		bool _updateEndGameAnimation();
		void _renderEndGameAnimation() const;
		void _updateRoundEndAnimation();
		void _renderRoundEndAnimation() const;
		void _updateRoundStartAnimation();
		void _renderRoundStartAnimation() const;
		void _renderLeftHUD() const;
		void _renderRightHUD() const;

	public:
		struct CharacterParams {
			ACharacter *character;
			unsigned hp;
			Vector2f gravity;
			unsigned char maxJumps;
			unsigned char maxAirDash;
			unsigned voidManaMax;
			unsigned spiritManaMax;
			unsigned matterManaMax;
			float manaRegen;
			unsigned maxBlockStun;
		};

		BattleManager(const CharacterParams &leftCharacter, const CharacterParams &rightCharacter);
		void addHitStop(unsigned stop);
		bool update();
		void render();
		unsigned registerObject(const std::shared_ptr<IObject> &object);
		void consumeEvent(const sf::Event &);
		std::shared_ptr<IObject> getObjectFromId(unsigned id) const;
		ACharacter *getLeftCharacter();
		ACharacter *getRightCharacter();
		const ACharacter *getLeftCharacter() const;
		const ACharacter *getRightCharacter() const;
		template <typename T, typename ...Args>
		std::pair<unsigned, std::shared_ptr<IObject>> registerObject(bool needRegister, Args &... args)
		{
			auto obj = std::make_shared<T>(args...);

			return {needRegister ? this->registerObject(obj) : 0, obj};
		}
		unsigned int getBufferSize() const;
		void copyToBuffer(void *data) const;
		void restoreFromBuffer(void *data);
	};
}


#endif //BATTLE_BATTLEMANAGER_HPP
