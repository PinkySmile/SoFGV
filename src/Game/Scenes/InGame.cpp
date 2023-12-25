//
// Created by PinkySmile on 24/09/2021.
//

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define makedir(path, _) _mkdir(path)
#else
#include <limits.h>
#define MAX_PATH PATH_MAX
#define makedir mkdir
#endif
#include <sys/stat.h>
#include "InGame.hpp"
#include "Objects/Character.hpp"
#include "Resources/Game.hpp"
#include "Logger.hpp"
#include "CharacterSelect.hpp"
#include "TitleScreen.hpp"
#include "Utils.hpp"
#include "Resources/version.h"
#include "Objects/StageObjects/StageObject.hpp"
#include "Objects/StageObjects/Cloud.hpp"

unsigned getMagic()
{
	unsigned magic = REPLAY_MAGIC;

	for (char c : REAL_VERSION_STR)
		magic += c * 5;
	return magic;
}

namespace SpiralOfFate
{
	static const char * const text[] = {
		"in the air",
		"during transform",
		"+",
		"charge",
		"(also in the air)",
		"together"
	};

	InGame::InGame(const GameParams &params) :
		_random(game->battleRandom.ser),
		_params(params)
	{
		for (unsigned i = 0; i < spritesPaths.size(); i++)
			this->_moveSprites[i] = game->textureMgr.load(spritesPaths[i]);
		game->logger.info("InGame scene created");
	}

	InGame::InGame(const InGame::InitParams &params, const std::string &endScene, bool saveReplay) :
		InGame(
			params.params,
			params.platforms,
			params.stage,
			params.leftChr,
			params.rightChr,
			params.licon,
			params.ricon,
			params.lJson,
			params.rJson,
			saveReplay,
			endScene
		)
	{
	}

	InGame::InGame(
		const GameParams &params,
		const std::vector<struct PlatformSkeleton> &platforms,
		const struct StageEntry &stage,
		Character *leftChr,
		Character *rightChr,
		unsigned licon,
		unsigned ricon,
		const nlohmann::json &lJson,
		const nlohmann::json &rJson,
		bool saveReplay,
		const std::string &endScene
	) :
		InGame(params)
	{
		this->_replaySaved = !saveReplay;
		this->_endScene = endScene;

		game->battleMgr = std::make_unique<BattleManager>(
			BattleManager::StageParams{
				stage.imagePath,
				[&stage, this]{
					return this->_generateStageObjects(stage);
				},
				[&platforms]{
					std::vector<Platform *> objects;

					for (auto &platform : platforms)
						objects.push_back(new Platform(platform.framedata, platform.width, platform.hp, platform.cd, platform.pos));
					return objects;
				}
			},
			BattleManager::CharacterParams{
				leftChr,
				licon,
				{
					true,
					lJson["hp"],
					lJson["jump_count"],
					lJson["air_dash_count"],
					lJson["air_movements"],
					lJson["mana_max"],
					lJson["mana_start"],
					lJson["mana_regen"],
					lJson["guard_bar"],
					lJson["guard_break_cooldown"],
					lJson["overdrive_cooldown"],
					lJson["ground_drag"],
					{lJson["air_drag"]["x"], lJson["air_drag"]["y"]},
					{lJson["gravity"]["x"], lJson["gravity"]["y"]},
					{
						lJson["airdrift"]["up"]["accel"],
						lJson["airdrift"]["up"]["max"]
					},
					{
						lJson["airdrift"]["down"]["accel"],
						lJson["airdrift"]["down"]["max"]
					},
					{
						lJson["airdrift"]["back"]["accel"],
						lJson["airdrift"]["back"]["max"]
					},
					{
						lJson["airdrift"]["front"]["accel"],
						lJson["airdrift"]["front"]["max"]
					},
				}
			},
			BattleManager::CharacterParams{
				rightChr,
				ricon,
				{
					false,
					rJson["hp"],
					rJson["jump_count"],
					rJson["air_dash_count"],
					rJson["air_movements"],
					rJson["mana_max"],
					rJson["mana_start"],
					rJson["mana_regen"],
					rJson["guard_bar"],
					rJson["guard_break_cooldown"],
					rJson["overdrive_cooldown"],
					rJson["ground_drag"],
					{rJson["air_drag"]["x"], rJson["air_drag"]["y"]},
					{rJson["gravity"]["x"], rJson["gravity"]["y"]},
					{
						rJson["airdrift"]["up"]["accel"],
						rJson["airdrift"]["up"]["max"]
					},
					{
						rJson["airdrift"]["down"]["accel"],
						rJson["airdrift"]["down"]["max"]
					},
					{
						rJson["airdrift"]["back"]["accel"],
						rJson["airdrift"]["back"]["max"]
					},
					{
						rJson["airdrift"]["front"]["accel"],
						rJson["airdrift"]["front"]["max"]
					},
				}
			}
		);
	}

	InGame::~InGame()
	{
		for (auto id : this->_moveSprites)
			game->textureMgr.remove(id);
		this->saveReplay();
	}

	void InGame::render() const
	{
		ViewPort view{{-50, -600, 1100, 700}};

		game->screen->setView(view);
		game->battleMgr->render();
		if (this->_moveList) {
			auto linput = game->battleMgr->getLeftCharacter();
			auto rinput = game->battleMgr->getRightCharacter();
			auto relevent = (this->_paused == 1 ? linput : rinput);

			this->_renderMoveList(relevent, L"P" + sf::String(std::to_string(this->_paused)) + L" | " + relevent->name + L"'s " + this->_moveListName);
		} else if (this->_paused)
			this->_renderPause();
	}

	void InGame::update()
	{
		auto linput = game->battleMgr->getLeftCharacter()->getInput();
		auto rinput = game->battleMgr->getRightCharacter()->getInput();

		if (this->_moveList) {
			linput->update();
			rinput->update();
			this->_moveListUpdate((this->_paused == 1 ? linput : rinput)->getInputs());
		} else if (!this->_paused) {
			if (!game->battleMgr->update()) {
				auto args = new CharacterSelect::Arguments();

				args->leftInput = game->battleMgr->getLeftCharacter()->getInput();
				args->rightInput = game->battleMgr->getRightCharacter()->getInput();
				args->leftPos = game->battleMgr->getLeftCharacter()->index & 0xFFFF;
				args->rightPos = game->battleMgr->getRightCharacter()->index & 0xFFFF;
				args->leftPalette = game->battleMgr->getLeftCharacter()->index >> 16;
				args->rightPalette = game->battleMgr->getRightCharacter()->index >> 16;
				//TODO: Save the stage and platform config properly
				args->stage = 0;
				args->platformCfg = 0;
				args->inGameName = game->scene.getCurrentScene().first;
				game->scene.switchScene(this->_endScene, args);
				return;
			}
			if (linput->getInputs().pause == 1)
				this->_paused = 1;
			else if (rinput->getInputs().pause == 1)
				this->_paused = 2;
		} else
			this->_pauseUpdate();
	}

	void InGame::consumeEvent(const sf::Event &event)
	{
		game->battleMgr->consumeEvent(event);
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
			this->_moveList = nullptr;
			if (this->_paused) {
				this->_paused = 3;
				this->_pauseCursor = 0;
			} else if (game->battleMgr->getLeftCharacter()->getInput()->getName() == "Keyboard" || game->battleMgr->getRightCharacter()->getInput()->getName() != "Keyboard")
				this->_paused = 1;
			else
				this->_paused = 2;
		}
	}

	void InGame::_renderPause() const
	{
		if (this->_paused == 3)
			return;
		game->screen->displayElement({340 - 50, 240 - 600, 400, 175}, sf::Color{0x50, 0x50, 0x50, 0xC0});

		game->screen->textSize(20);
		game->screen->fillColor(sf::Color::White);
		game->screen->displayElement("P" + std::to_string(this->_paused) + " | Paused", {340 - 50, 245 - 600}, 400, Screen::ALIGN_CENTER);
		for (size_t i = 0; i < sizeof(InGame::_menuStrings) / sizeof(*InGame::_menuStrings); i++) {
			game->screen->fillColor(i == this->_pauseCursor ? sf::Color::Yellow : sf::Color::White);
			game->screen->displayElement(InGame::_menuStrings[i], {350 - 50, 285 - 600 + 25.f * i});
		}
		game->screen->fillColor(sf::Color::White);
		game->screen->textSize(30);
	}

	void InGame::_pauseUpdate()
	{
		auto linput = game->battleMgr->getLeftCharacter()->getInput();
		auto rinput = game->battleMgr->getRightCharacter()->getInput();

		linput->update();
		rinput->update();

		auto relevant = (this->_paused == 1 ? linput : rinput)->getInputs();

		if (this->_paused == 3) {
			auto l = linput->getInputs();
			auto r = rinput->getInputs();

			for (size_t i = 0; i < sizeof(l) / sizeof(int); i++)
				if (((int *)&l)[i])
					return;
			for (size_t i = 0; i < sizeof(r) / sizeof(int); i++)
				if (((int *)&r)[i])
					return;
			this->_paused = 0;
			return;
		}
		if (relevant.pause == 1 || relevant.s == 1) {
			this->_pauseCursor = 0;
			this->_paused = 3;
			return;
		}
		if (relevant.verticalAxis == 1 || (relevant.verticalAxis >= 36 && relevant.verticalAxis % 6 == 0)) {
			this->_pauseCursor += sizeof(InGame::_menuStrings) / sizeof(*InGame::_menuStrings);
			this->_pauseCursor--;
			this->_pauseCursor %= sizeof(InGame::_menuStrings) / sizeof(*InGame::_menuStrings);
		} else if (relevant.verticalAxis == -1 || (relevant.verticalAxis <= -36 && relevant.verticalAxis % 6 == 0)) {
			this->_pauseCursor++;
			this->_pauseCursor %= sizeof(InGame::_menuStrings) / sizeof(*InGame::_menuStrings);
		}
		if (relevant.n == 1 && this->_pauseConfirm()) {
			this->_pauseCursor = 0;
			this->_paused = 3;
		}
	}

	bool InGame::_pauseConfirm()
	{
		switch (this->_pauseCursor) {
		case 0:
			return true;
		case 1:
			this->_moveList = &(this->_paused == 1 ? game->battleMgr->getLeftCharacter() : game->battleMgr->getRightCharacter())->getFrameData();
			this->_moveListCursor = 0;
			this->_moveListTop = 0;
			this->_moveOrder = defaultMoveOrder;
			this->_moveData = defaultMoveData;
			this->_moveListName = L"Move List";
			this->_calculateMoveListOrder();
			this->_moveListObject = std::make_unique<FakeObject>(*this->_moveList);
			return false;
		case 2:
			this->_moveList = &(this->_paused == 1 ? game->battleMgr->getLeftCharacter() : game->battleMgr->getRightCharacter())->getFrameData();
			this->_moveListCursor = 0;
			this->_moveListTop = 0;
			this->_moveOrder = defaultCommandOrder;
			this->_moveData = defaultMoveData;
			this->_moveListName = L"Command List";
			this->_calculateMoveListOrder();
			this->_moveListObject = std::make_unique<FakeObject>(*this->_moveList);
			return false;
		case 3: {
			auto args = new CharacterSelect::Arguments();

			args->leftInput = game->battleMgr->getLeftCharacter()->getInput();
			args->rightInput = game->battleMgr->getRightCharacter()->getInput();
			args->leftPos = game->battleMgr->getLeftCharacter()->index & 0xFFFF;
			args->rightPos = game->battleMgr->getRightCharacter()->index & 0xFFFF;
			args->leftPalette = game->battleMgr->getLeftCharacter()->index >> 16;
			args->rightPalette = game->battleMgr->getRightCharacter()->index >> 16;
			//TODO: Save the stage and platform config properly
			args->stage = 0;
			args->platformCfg = 0;
			args->inGameName = game->scene.getCurrentScene().first;
			game->scene.switchScene("char_select", args);
			return false;
		}
		case 4:
			game->scene.switchScene("title_screen");
			return false;
		default:
			return false;
		}
	}

	void InGame::_renderMoveList(Character *relevent, const std::wstring &title) const
	{
		Sprite sprite;
		unsigned noText[] = {
			ACTION_NEUTRAL_OVERDRIVE,
			ACTION_MATTER_OVERDRIVE,
			ACTION_SPIRIT_OVERDRIVE,
			ACTION_VOID_OVERDRIVE,
			ACTION_GROUND_HIGH_NEUTRAL_PARRY,
			ACTION_GROUND_HIGH_MATTER_PARRY,
			ACTION_GROUND_HIGH_SPIRIT_PARRY,
			ACTION_GROUND_HIGH_VOID_PARRY
		};

		sprite.setScale(0.5f, 0.5f);
		game->screen->displayElement({140 - 50, 10 - 600, 800, 680}, sf::Color{0x50, 0x50, 0x50, 0xF0});

		game->screen->textSize(20);
		game->screen->displayElement(title, {140 - 50, 15 - 600}, 800, Screen::ALIGN_CENTER);
		game->screen->textSize(15);
		if (this->_moveListTop > 0)
			game->screen->displayElement("^^^^^^^^", {140 - 50, 50 - 600}, 400, Screen::ALIGN_CENTER);
		if (this->_moveListTop < this->_moveListCursorMax - 10 && this->_moveListCursorMax > 10)
			game->screen->displayElement("VVVVVVVV", {140 - 50, 670 - 600}, 400, Screen::ALIGN_CENTER);
		for (size_t i = this->_moveListTop, k = 0; i < this->_moveOrder.size() && k < 10; i++) {
			auto move = this->_moveList->find(this->_moveOrder[i]);

			if (move == this->_moveList->end())
				continue;

			auto data = this->_moveData.find(this->_moveOrder[i]);
			auto str = data->second.name;

			if (data == this->_moveData.end())
				continue;

			Vector2f pos{150 - 50, 70 - 600 + k * 60.f};
			auto prio = relevent->getAttackTier(this->_moveOrder[i]);

			k++;
			if (this->_moveListCursor == i)
				game->screen->displayElement({
					static_cast<int>(pos.x - 5),
					static_cast<int>(pos.y - 5),
					400, 60
				}, sf::Color{0xA0, 0xA0, 0xFF, 0xC0});

			game->screen->fillColor(sf::Color::White);
			if (std::find(noText, noText + (sizeof(noText) / sizeof(*noText)), this->_moveOrder[i]) != noText + (sizeof(noText) / sizeof(*noText)))
				game->screen->fillColor(sf::Color::Green);
			else if (prio >= 800) {
				game->screen->fillColor(sf::Color::Red);
				str += " | Ultimate";
			} else if (prio >= 700) {
				game->screen->fillColor(sf::Color{0xFF, 0x80, 0x00});
				str += " | Super";
			} else if (prio >= 600)
				str += " | Skill";
			else if (prio >= 300)
				str += " | Typed command normal";
			else if (prio >= 200)
				str += " | Command normal";
			else if (prio >= 100)
				str += " | Typed light attack";
			else if (prio >= 0)
				str += " | Light attack";

			game->screen->displayElement(str, pos);
			game->screen->fillColor(sf::Color::White);

			for (auto input : data->second.input) {
				if (input < NB_SPRITES) {
					sprite.setPosition(pos + Vector2f{0, 18});
					sprite.textureHandle = this->_moveSprites[input];
					game->textureMgr.render(sprite);
					pos.x += 35;
				} else {
					game->screen->displayElement(text[input - NB_SPRITES], pos + Vector2f{0, 25});
					pos.x += game->screen->getTextSize(text[input - NB_SPRITES]) + 5;
				}
			}
		}
		game->screen->displayElement({590 - 50, 75 - 600, 300, 360}, sf::Color::White);
		for (int x = -static_cast<int>(this->_moveListObject->_position.x) % 16, i = 0; x < 300; x += 16, i++) {
			bool color = static_cast<int>((this->_moveListObject->_position.x + x) / 16) % 2 == 1;

			if (x <= -16)
				continue;
			for (int y = -static_cast<int>(this->_moveListObject->_position.y) % 32; y < 360; y += 16) {
				Vector2i size{16, 16};
				Vector2i pos{x, y};

				color = !color;
				if (y <= -16)
					continue;
				if (y < 0) {
					size.y += y;
					pos.y = 0;
				} else if (y > 344)
					size.y -= y - 344;
				if (x < 0) {
					size.x += x;
					pos.x = 0;
				} else if (x > 284)
					size.x -= x - 284;
				game->screen->displayElement({
					590 - 50 + pos.x,
					75 - 600 + pos.y,
					size.x, size.y
				}, color ? sf::Color{0xA0, 0xA0, 0xA0} : sf::Color::White);
			}
		}
		this->_moveListObject->render();
		game->screen->textSize(30);
	}

	void InGame::_moveListUpdate(InputStruct relevent)
	{
		if (relevent.pause == 1 || relevent.s == 1) {
			this->_moveList = nullptr;
			this->_moveListObject.reset();
			return;
		}
		if (relevent.verticalAxis == 1 || (relevent.verticalAxis >= 36 && relevent.verticalAxis % 6 == 0)) {
			this->_moveListCursor += this->_moveListCursorMax;
			this->_moveListCursor--;
			this->_moveListCursor %= this->_moveListCursorMax;
			if (this->_moveListCursor < this->_moveListTop)
				this->_moveListTop = this->_moveListCursor;
			else if (this->_moveListCursor == this->_moveListCursorMax - 1)
				this->_moveListTop = this->_moveListCursorMax < 10 ? 0 : this->_moveListCursorMax - 10;
			this->_moveListObject->_forceStartMove(this->_moveDisplayed[this->_moveListCursor]);
			this->_moveListObject->_position = {500, 500};
			this->_moveListObject->_speed = {0, 0};
			this->_moveListObject->_rotation = 0;
			this->_moveListTimer = 0;
		} else if (relevent.verticalAxis == -1 || (relevent.verticalAxis <= -36 && relevent.verticalAxis % 6 == 0)) {
			this->_moveListCursor++;
			this->_moveListCursor %= this->_moveListCursorMax;
			if (this->_moveListCursor >= this->_moveListTop + 10)
				this->_moveListTop = this->_moveListCursor - 9;
			else if (this->_moveListCursor == 0)
				this->_moveListTop = 0;
			this->_moveListObject->_forceStartMove(this->_moveDisplayed[this->_moveListCursor]);
			this->_moveListObject->_position = {500, 500};
			this->_moveListObject->_speed = {0, 0};
			this->_moveListObject->_rotation = 0;
			this->_moveListTimer = 0;
		}
		this->_moveListObject->update();
		if (this->_moveDisplayed[this->_moveListCursor]) {
			this->_moveListTimer += this->_moveListObject->_action == ACTION_IDLE || this->_moveListObject->_action == ACTION_CROUCH || this->_moveListObject->_action == ACTION_FALLING;
			if (this->_moveListTimer > 60) {
				this->_moveListObject->_forceStartMove(this->_moveDisplayed[this->_moveListCursor]);
				this->_moveListObject->_position = {500, 500};
				this->_moveListObject->_speed = {0, 0};
				this->_moveListObject->_rotation = 0;
				this->_moveListTimer = 0;
			}
		}
	}

	void InGame::_calculateMoveListOrder()
	{
		std::vector<unsigned> ultimates;
		std::vector<unsigned> supers;
		auto moveData = this->_paused == 1 ? this->_leftMoveData : this->_rightMoveData;

		for (size_t i = 0; i < this->_moveOrder.size(); i++) {
			auto it = this->_moveList->find(this->_moveOrder[i]);
			auto data = this->_moveData.find(this->_moveOrder[i]);
			auto str = data->second.name;

			if (data == this->_moveData.end() || !data->second.displayed){
				this->_moveOrder.erase(this->_moveOrder.begin() + i);
				i--;
			} else if (it == this->_moveList->end()) {
				this->_moveOrder.erase(this->_moveOrder.begin() + i);
				i--;
			} else if (it->second.front().front().oFlag.ultimate) {
				ultimates.push_back(this->_moveOrder[i]);
				this->_moveOrder.erase(this->_moveOrder.begin() + i);
				i--;
			} else if (it->second.front().front().oFlag.super) {
				supers.push_back(this->_moveOrder[i]);
				this->_moveOrder.erase(this->_moveOrder.begin() + i);
				i--;
			}
		}
		this->_moveOrder.insert(this->_moveOrder.end(), supers.begin(), supers.end());
		this->_moveOrder.insert(this->_moveOrder.end(), ultimates.begin(), ultimates.end());

		for (auto &data : moveData)
			this->_moveData[data.first] = data.second;

		this->_moveListCursorMax = 0;
		this->_moveDisplayed.clear();
		this->_moveDisplayed.reserve(this->_moveOrder.size());
		for (unsigned int id : this->_moveOrder) {
			auto move = this->_moveList->find(id);

			if (move == this->_moveList->end())
				continue;

			auto data = this->_moveData.find(id);
			auto str = data->second.name;

			if (data == this->_moveData.end() || !data->second.displayed)
				continue;
			this->_moveDisplayed.push_back(id);
			this->_moveListCursorMax++;
		}
		this->_moveDisplayed.shrink_to_fit();
	}

	void InGame::saveReplay()
	{
		if (this->_replaySaved)
			return;
		this->_replaySaved = true;

		char buf[60];
		char buf2[MAX_PATH];
		time_t timer;
		char timebuffer[40];
		char timebuffer2[40];
		struct tm* tm_info;
		unsigned magic = getMagic();
		unsigned frame = game->battleMgr->getCurrentFrame();
		auto leftChr    = game->battleMgr->getLeftCharacter();
		auto rightChr   = game->battleMgr->getRightCharacter();
		auto leftInputs = game->battleMgr->getLeftReplayData();
		auto rightInputs= game->battleMgr->getRightReplayData();
#pragma pack(push, 1)
		struct CharacterData {
			unsigned index;
			unsigned nbInputs;
		} leftChrSer, rightChrSer;
#pragma pack(pop)

		time(&timer);
		tm_info = localtime(&timer);
		strftime(timebuffer, 40, "%Y-%m-%d", tm_info);
		strftime(timebuffer2, 40, "%H_%M_%S", tm_info);
		sprintf(buf, "replays/%s", timebuffer);
		sprintf(buf2, "%s/%s_(%s_vs_%s).replay", buf, timebuffer2, sf::String(leftChr->name).toUtf8().c_str(), sf::String(rightChr->name).toUtf8().c_str());

		if (makedir("replays", 0755) && errno != EEXIST) {
			SpiralOfFate::game->logger.error("Failed to create replays folder: " + std::string(strerror(errno)));
			Utils::dispMsg("Replay saving failure", "Failed to create replays folder: " + std::string(strerror(errno)), MB_ICONERROR, &*game->screen);
			return;
		}
		if (makedir(buf, 0755) && errno != EEXIST) {
			SpiralOfFate::game->logger.error("Failed to create " + std::string(buf) + " folder: " + strerror(errno));
			Utils::dispMsg("Replay saving failure", "Failed to create " + std::string(buf) + " folder: " + strerror(errno), MB_ICONERROR, &*game->screen);
			return;
		}

		std::ofstream stream{buf2, std::ofstream::binary};

		if (stream.fail()) {
			SpiralOfFate::game->logger.error("Failed to create " + std::string(buf2) + ": " + strerror(errno));
			Utils::dispMsg("Replay saving failure", "Failed to create " + std::string(buf2) + ": " + strerror(errno), MB_ICONERROR, &*game->screen);
			return;
		}
		stream.write(reinterpret_cast<char *>(&magic), sizeof(magic));
		stream.write(reinterpret_cast<char *>(&frame), sizeof(frame));
		stream.write(reinterpret_cast<char *>(&this->_random), sizeof(this->_random));
		stream.write(reinterpret_cast<char *>(&this->_params), sizeof(this->_params));
		leftChrSer.index = leftChr->index;
		leftChrSer.nbInputs = leftInputs.size();
		stream.write(reinterpret_cast<char *>(&leftChrSer), sizeof(leftChrSer));
		stream.write(reinterpret_cast<char *>(leftInputs.data()), leftInputs.size() * sizeof(ReplayData));

		rightChrSer.index = rightChr->index;
		rightChrSer.nbInputs = rightInputs.size();
		stream.write(reinterpret_cast<char *>(&rightChrSer), sizeof(rightChrSer));
		stream.write(reinterpret_cast<char *>(rightInputs.data()), rightInputs.size() * sizeof(ReplayData));
		game->logger.info(std::string(buf2) + " created.");
	}

	std::vector<Object *> InGame::_generateStageObjects(const StageEntry &stage)
	{
		if (stage.objectPath.empty())
			return std::vector<Object *>{};

		auto data = game->fileMgr.readFull(stage.objectPath);
		nlohmann::json json;
		std::vector<Object *> objects;

		try {
			json = nlohmann::json::parse(data);
			for (auto &obj : json) {
				switch (obj["class"].get<int>()) {
				case 1:
					objects.push_back(new Cloud(obj));
					break;
				default:
					objects.push_back(new StageObject(obj));
				}
			}
			return objects;
		} catch (std::exception &e) {
			game->logger.error("Error while loading objects: " + std::string(e.what()));
			for (auto object : objects)
				delete object;
		}
		return {};
	}

	InGame *InGame::create(SceneArguments *args)
	{
		checked_cast(realArgs, InGame::Arguments, args);

		auto params = realArgs->characterSelectScene->createParams(args);

		if (args->reportProgressA)
			args->reportProgressA("Creating scene...");
		return new InGame(params, realArgs->endScene, realArgs->saveReplay);
	}
}
