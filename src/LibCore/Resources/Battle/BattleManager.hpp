//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_BATTLEMANAGER_HPP
#define SOFGV_BATTLEMANAGER_HPP


#include "Objects/Characters/Character.hpp"
#include "Objects/Platform.hpp"
#include "Resources/Assets/MoveListData.hpp"

#define STAGE_X_MIN 0
#define STAGE_X_MAX 1000
#define STAGE_Y_MIN 0
#define STAGE_Y_MAX 750

namespace SpiralOfFate
{
	enum SystemParticleGenerator {
		SYS_PARTICLE_GENERATOR_SPIRIT_TYPE_SWITCH,
		SYS_PARTICLE_GENERATOR_MATTER_TYPE_SWITCH,
		SYS_PARTICLE_GENERATOR_VOID_TYPE_SWITCH,
		SYS_PARTICLE_GENERATOR_NEUTRAL_TYPE_SWITCH,
		SYS_PARTICLE_GENERATOR_SPIRIT_TYPE_DEBUFF,
		SYS_PARTICLE_GENERATOR_MATTER_TYPE_DEBUFF,
		SYS_PARTICLE_GENERATOR_VOID_TYPE_DEBUFF,
		SYS_PARTICLE_GENERATOR_NEUTRAL_TYPE_DEBUFF,
	};

	enum BattleUiSprite {
		BATTLEUI_HUD_SEAT,
		BATTLEUI_MANA_BAR,
		BATTLEUI_MANA_BAR_CROSS,
		BATTLEUI_GUARD_TEXT,
		BATTLEUI_GUARD_BAR,
		BATTLEUI_GUARD_BAR_TMP,
		BATTLEUI_GUARD_BAR_DISABLED,
		BATTLEUI_LIFE_BAR,
		BATTLEUI_LIFE_BAR_RED,
		BATTLEUI_LIFE_BAR_EFFECT,
		BATTLEUI_OVERDRIVE,
		BATTLEUI_OVERDRIVE_OUTLINE,
		BATTLEUI_SCORE_SEAT,
		BATTLEUI_SCORE_BULLET,
		BATTLEUI_NB_SPRITES,
	};

	class BattleManager {
	protected:
		struct HUDDataPacked;
		struct HUDData {
			BattleManager &mgr;
			Character &base;
			Sprite &icon;
			bool side;
			mutable sf::RenderTexture target;
			unsigned comboCtr = 0;
			unsigned hitCtr = 0;
			unsigned neutralLimit = 0;
			unsigned voidLimit = 0;
			unsigned spiritLimit = 0;
			unsigned matterLimit = 0;
			unsigned totalDamage = 0;
			unsigned guardCrossTimer = 0;
			unsigned overdriveCrossTimer = 0;
			unsigned lifeBarEffect = 0;
			float penaltyTimer = 0;
			float proration = 0;
			bool counter = false;
			unsigned char score = 0;

			HUDData(BattleManager &mgr, Character &base, Sprite &icon, bool side);
			void renderMeterBar(sf::RenderTarget &output, Vector2i pos, float bar, sf::Color minColor, sf::Color maxColor) const;
			void render(sf::RenderTarget &output) const;
			void renderNoReverse(sf::RenderTarget &output) const;
			void update();
			HUDData &operator=(HUDDataPacked &data);
			HUDData &operator=(const HUDDataPacked &data);
		};

#pragma pack(push, 1)
		struct HUDDataPacked {
			unsigned comboCtr = 0;
			unsigned hitCtr = 0;
			unsigned neutralLimit = 0;
			unsigned voidLimit = 0;
			unsigned spiritLimit = 0;
			unsigned matterLimit = 0;
			unsigned totalDamage = 0;
			unsigned guardCrossTimer = 0;
			unsigned overdriveCrossTimer = 0;
			unsigned lifeBarEffect = 0;
			float penaltyTimer = 0;
			float proration = 0;
			bool counter = false;
			unsigned char score = 0;

			HUDDataPacked &operator=(HUDData &data);
			HUDDataPacked &operator=(const HUDData &data);
		};
		static_assert(sizeof(HUDDataPacked) == 50, "HUDDataPacket has wrong size");

		struct Data {
			uint64_t random;
			unsigned _currentFrame;
			unsigned _lastObjectId;
			unsigned _currentRound;
			unsigned _roundEndTimer;
			unsigned _nbObjects;
			unsigned _nbIObjects;
			unsigned _nbStageObjects;
			int _roundStartTimer;
			HUDDataPacked _leftHUDData;
			HUDDataPacked _rightHUDData;
			unsigned short _limitAnimTimer;
			bool _ended;
		};
		static_assert(sizeof(Data) == 143, "Data has wrong size");
#pragma pack(pop)

		// Non-Game State
		std::vector<ParticleGenerator::InitData> _systemParticles;
		unsigned _nbPlatform = 0;
		sf::Clock _tpsClock;
		std::list<unsigned> _tpsTimes;
		mutable sf::Clock _fpsClock;
		mutable std::list<unsigned> _fpsTimes;
		Sprite _stage;
		sf::Font _font;
		sf::RenderTexture _tex;
		sf::RenderTexture _hud;
		sf::RenderTexture _leftHUD;
		sf::RenderTexture _rightHUD;
		Sprite _stallWarn;
		Sprite _stallDown;
		Sprite _leftIcon;
		Sprite _rightIcon;
		Sprite _leftHUDIcon;
		Sprite _rightHUDIcon;
		Sprite _oosBubble;
		Sprite _oosBubbleMask;
		Sprite _battleUi[BATTLEUI_NB_SPRITES];
		Sprite _limitSprites[8];
		sf::Sprite _roundSprite;
		sf::Texture _cross;
		std::vector<sf::Texture> _roundSprites;
		unsigned char _speed = 60;
		//TODO: Also save these in the rollback
		std::vector<std::unique_ptr<Object>> _stageObjects;
		std::array<unsigned, NB_SPRITES> _moveSprites;
		bool _leftFirst = false;

		// Game State
		std::unique_ptr<Character> _leftCharacter;
		std::unique_ptr<Character> _rightCharacter;
		std::vector<std::shared_ptr<Platform>> _platforms;
		std::vector<std::pair<unsigned, std::shared_ptr<Object>>> _objects;
		std::vector<std::pair<unsigned, std::shared_ptr<IObject>>> _iobjects;
		unsigned _currentRound = 0;
		unsigned _lastObjectId = 0;
		unsigned _roundEndTimer = 0;
		unsigned _currentFrame = 0;
		int _roundStartTimer = 0;
		unsigned short _limitAnimTimer = 0;
		bool _ended = false;
		HUDData _leftHUDData;
		HUDData _rightHUDData;

		void _gameUpdate();
		virtual bool _updateLoop();
		bool _updateEndGameAnimation();
		void _renderEndGameAnimation() const;
		void _updateRoundEndAnimation();
		void _renderRoundEndAnimation() const;
		void _updateRoundStartAnimation();
		void _renderRoundStartAnimation() const;
		void _renderInputs(const std::vector<ReplayData> &data, Vector2f pos, bool side);
		void _renderButton(unsigned spriteId, float offset, int k, Vector2f pos);
		void _renderCharacter(const Character &chr);

	public:
		struct CharacterParams {
			Character *character;
			unsigned icon;
			Character::InitData data;
		};
		struct StageParams {
			std::string path;
			std::function<std::vector<Object *>()> objects;
			std::function<std::vector<Platform *>()> platforms;
		};

		// Called when frames are skipped for the animation of round transitions
		std::function<void ()> onFrameSkipped;

		BattleManager(const StageParams &stage, const CharacterParams &leftCharacter, const CharacterParams &rightCharacter);
		virtual ~BattleManager();
		virtual bool update();
		virtual void render();
		void renderInputs();
		void renderLeftInputs();
		void renderRightInputs();
		unsigned registerObject(const std::shared_ptr<Object> &object);
		unsigned registerObject(const std::shared_ptr<IObject> &object);
		virtual void consumeEvent(const sf::Event &);
		std::shared_ptr<Object> getObjectFromId(unsigned id) const;
		std::shared_ptr<IObject> getIObjectFromId(unsigned id) const;
		Character *getLeftCharacter();
		Character *getRightCharacter();
		const Character *getLeftCharacter() const;
		const Character *getRightCharacter() const;
		const std::vector<ReplayData> &getLeftReplayData() const;
		const std::vector<ReplayData> &getRightReplayData() const;
		unsigned getCurrentFrame() const;
		void logDifference(void *data1, void *data2);
		void printContent(void *data, size_t size);
		template <typename T, typename ...Args>
		auto registerObject(bool needRegister, const Args &... args)
		{
			auto obj = std::make_shared<T>(args...);

			if constexpr (std::is_base_of<Object, T>::value)
				return std::pair<unsigned, std::shared_ptr<Object>>{needRegister ? this->registerObject(static_cast<std::shared_ptr<Object>>(obj)) : 0, obj};
			else
				return std::pair<unsigned, std::shared_ptr<IObject>>{needRegister ? this->registerObject(obj) : 0, obj};
		}
		unsigned getBufferSize() const;
		void copyToBuffer(void *data) const;
		void restoreFromBuffer(void *data);
		const std::vector<std::pair<unsigned, std::shared_ptr<Object>>> &getObjects() const;
		const std::vector<std::shared_ptr<Platform>> &getPlatforms() const;
		bool isLeftFirst() const;

		static unsigned getFrame(void *data);

		friend class ObjectFactory;
	};
}


#endif //SOFGV_BATTLEMANAGER_HPP
