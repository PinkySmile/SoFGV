//
// Created by PinkySmile on 18/09/2021
//

#ifndef SOFGV_BATTLEMANAGER_HPP
#define SOFGV_BATTLEMANAGER_HPP


#include "../Objects/Character.hpp"
#include "../Objects/Platform.hpp"
#include "MoveListData.hpp"

namespace SpiralOfFate
{
	class BattleManager {
	protected:
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
		unsigned _nbPlatform = 0;
		sf::Clock _tpsClock;
		std::list<unsigned> _tpsTimes;
		mutable sf::Clock _fpsClock;
		mutable std::list<unsigned> _fpsTimes;
		Sprite _stage;
		unsigned int _leftGuardCrossTimer = 0;
		unsigned int _rightGuardCrossTimer = 0;
		unsigned int _leftOverdriveCrossTimer = 0;
		unsigned int _rightOverdriveCrossTimer = 0;
		sf::Sprite _roundSprite;
		sf::Texture _cross;
		std::vector<sf::Texture> _roundSprites;
		bool _step = false;
		bool _next = false;
		unsigned char _speed = 60;
		float _time = 0;
		unsigned _lastObjectId = 0;
		std::vector<std::unique_ptr<IObject>> _stageObjects;
		std::array<sf::Texture, NB_SPRITES> _moveSprites;

		// Game State
		std::unique_ptr<Character> _leftCharacter;
		std::unique_ptr<Character> _rightCharacter;
		std::vector<std::shared_ptr<Platform>> _platforms;
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
		bool _leftCounter = false;
		float _leftProration = 0;
		unsigned _rightComboCtr = 0;
		unsigned _rightHitCtr = 0;
		unsigned _rightNeutralLimit = 0;
		unsigned _rightVoidLimit = 0;
		unsigned _rightSpiritLimit = 0;
		unsigned _rightMatterLimit = 0;
		unsigned _rightTotalDamage = 0;
		bool _rightCounter = false;
		float _rightProration = 0;

		void _gameUpdate();
		virtual bool _updateLoop();
		bool _updateEndGameAnimation();
		void _renderEndGameAnimation() const;
		void _updateRoundEndAnimation();
		void _renderRoundEndAnimation() const;
		void _updateRoundStartAnimation();
		void _renderRoundStartAnimation() const;
		void _renderLeftHUD() const;
		void _renderRightHUD() const;
		void _renderInputs(const std::vector<Character::ReplayData> &data, Vector2f pos, bool side);

	public:
		struct CharacterParams {
			Character *character;
			unsigned hp;
			Vector2f gravity;
			unsigned char maxJumps;
			unsigned char maxAirDash;
			unsigned voidManaMax;
			unsigned spiritManaMax;
			unsigned matterManaMax;
			float manaRegen;
			unsigned maxGuard;
			unsigned guardCooldown;
			unsigned odCd;
			float groundDrag;
			Vector2f airDrag;
		};
		struct StageParams {
			std::string path;
			std::function<std::vector<IObject *>()> objects;
			std::function<std::vector<Platform *>()> platforms;
		};

		BattleManager(const StageParams &stage, const CharacterParams &leftCharacter, const CharacterParams &rightCharacter);
		virtual ~BattleManager() = default;
		void addHitStop(unsigned stop);
		virtual bool update();
		virtual void render();
		void renderInputs();
		void renderLeftInputs();
		void renderRightInputs();
		unsigned registerObject(const std::shared_ptr<IObject> &object);
		virtual void consumeEvent(const sf::Event &);
		std::shared_ptr<IObject> getObjectFromId(unsigned id) const;
		Character *getLeftCharacter();
		Character *getRightCharacter();
		const Character *getLeftCharacter() const;
		const Character *getRightCharacter() const;
		const std::vector<Character::ReplayData> &getLeftReplayData() const;
		const std::vector<Character::ReplayData> &getRightReplayData() const;
		template <typename T, typename ...Args>
		std::pair<unsigned, std::shared_ptr<IObject>> registerObject(bool needRegister, const Args &... args)
		{
			auto obj = std::make_shared<T>(args...);

			return {needRegister ? this->registerObject(obj) : 0, obj};
		}
		unsigned int getBufferSize() const;
		void copyToBuffer(void *data) const;
		void restoreFromBuffer(void *data);
		const std::vector<std::shared_ptr<Platform>> &getPlatforms() const;
	};
}


#endif //SOFGV_BATTLEMANAGER_HPP
