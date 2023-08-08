//
// Created by PinkySmile on 11/07/23.
//

#include <Resources/Game.hpp>
#include <fstream>
#include "Objects/StageObjects/Cloud.hpp"
#include "Objects/StageObjects/StageObject.hpp"
#include "Objects/Characters/VictoriaStar/VictoriaStar.hpp"
#include "Objects/Characters/Stickman/Stickman.hpp"

using namespace SpiralOfFate;

Character *createCharacter(std::string folder, int _class)
{
	switch (_class) {
	case 2:
		return new VictoriaStar{
			0,
			folder,
			{},
			nullptr,
			""
		};
	case 1:
		return new Stickman{
			0,
			folder,
			{},
			nullptr
		};
	default:
		return new Character{
			0,
			folder,
			{},
			nullptr
		};
	}
}

int main(int argc, char **argv)
{
	try {
		if (argc < 6) {
			printf("Usage: %s <stage_id> <platform_id> <chr1_path> <chr2_path> <file1.frame> [<file2.frame>]\n", argv[0]);
			return EXIT_FAILURE;
		}

		std::ifstream stream;
		nlohmann::json stagesJson;
		nlohmann::json lJson;
		nlohmann::json rJson;
		unsigned stage = atoi(argv[1]);
		unsigned platforms = atoi(argv[2]);
		char *chr1_path = argv[3];
		char *chr2_path = argv[4];
		char *frame_file = argv[5];
		char *frame_file2 = argv[6];

		new Game("comparer.log");
	//	game->screen->setView(view);
	//	game->logger.info("CharacterSelect scene created");
	//	this->_entries.reserve(chrList.size());
	//	for (auto &entry : chrList) {
	//		auto file = entry / "chr.json";
	//		std::ifstream s{file};

	//		game->logger.debug("Loading character from " + file.string());
	//		my_assert2(!s.fail(), file.string() + ": " + strerror(errno));
	//		s >> json;
	//#ifndef _DEBUG
	//		if (json.contains("hidden") && json["hidden"])
	//				continue;
	//#endif
	//		this->_entries.emplace_back(json, entry.string());
	//	}

		stream.open("assets/stages/list.json");
		my_assert2(!stream.fail(), "assets/stages/list.json: " + strerror(errno));
		stream >> stagesJson;
		stream.close();
		stream.open(chr1_path + std::string("/chr.json"));
		my_assert2(!stream.fail(), chr1_path + std::string("/chr.json: ") + strerror(errno));
		stream >> lJson;
		stream.close();
		stream.open(chr2_path + std::string("/chr.json"));
		my_assert2(!stream.fail(), chr2_path + std::string("/chr.json: ") + strerror(errno));
		stream >> rJson;
		stream.close();
		my_assert(stagesJson.size() > stage);
		my_assert(stagesJson[stage]["platforms"].size() > platforms);

		game->battleMgr.reset(new BattleManager{
			BattleManager::StageParams{
				stagesJson[stage]["image"],
				[&stagesJson]{
					if (!stagesJson.contains("objects"))
						return std::vector<IObject *>{};

					std::ifstream stream2{stagesJson["objects"].get<std::string>()};
					nlohmann::json json;
					std::vector<IObject *> objects;

					if (stream2.fail()) {
						game->logger.error("Failed to open stage object file: " + stagesJson["objects"].get<std::string>() + ": " + strerror(errno));
						return objects;
					}

					try {
						stream2 >> json;
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
					return std::vector<IObject *>{};
				},
				[&stagesJson, stage, platforms]{
					std::vector<Platform *> objects;

					for (auto &platform : stagesJson[stage]["platforms"][platforms])
						objects.push_back(new Platform(platform["framedata"], platform["width"], platform["hp"], platform["cd"], Vector2f{
							platform["pos"]["x"],
							platform["pos"]["y"]
						}));
					return objects;
				}
			},
			BattleManager::CharacterParams{
				createCharacter(chr1_path, lJson["class"]),
				0,
				lJson["hp"],
				{lJson["gravity"]["x"], lJson["gravity"]["y"]},
				lJson["jump_count"],
				lJson["air_dash_count"],
				lJson["void_mana_max"],
				lJson["spirit_mana_max"],
				lJson["matter_mana_max"],
				lJson["void_mana_start"],
				lJson["spirit_mana_start"],
				lJson["matter_mana_start"],
				lJson["mana_regen"],
				lJson["guard_bar"],
				lJson["guard_break_cooldown"],
				lJson["overdrive_cooldown"],
				lJson["ground_drag"],
				{lJson["air_drag"]["x"], lJson["air_drag"]["y"]}
			},
			BattleManager::CharacterParams{
				createCharacter(chr2_path, rJson["class"]),
				0,
				rJson["hp"],
				{rJson["gravity"]["x"], rJson["gravity"]["y"]},
				rJson["jump_count"],
				rJson["air_dash_count"],
				rJson["void_mana_max"],
				rJson["spirit_mana_max"],
				rJson["matter_mana_max"],
				rJson["void_mana_start"],
				rJson["spirit_mana_start"],
				rJson["matter_mana_start"],
				rJson["mana_regen"],
				rJson["guard_bar"],
				rJson["guard_break_cooldown"],
				rJson["overdrive_cooldown"],
				rJson["ground_drag"],
				{rJson["air_drag"]["x"], rJson["air_drag"]["y"]}
			}
		});
		char buffer1[16384];
		char buffer2[16384];

		memset(buffer1, 0, sizeof(buffer1));
		stream.open(frame_file);
		my_assert2(!stream.fail(), frame_file + std::string(": ") + strerror(errno));
		stream.read(buffer1, sizeof(buffer1));

		auto size = stream.tellg();

		stream.close();
		if (frame_file2) {
			stream.open(frame_file2);
			my_assert2(!stream.fail(), frame_file2 + std::string(": ") + strerror(errno));
			stream.read(buffer2, sizeof(buffer2));
			stream.close();
		}
		if (frame_file2)
			game->battleMgr->logDifference(buffer1, buffer2);
		else
			game->battleMgr->printContent(buffer1, size);
		return EXIT_SUCCESS;
	} catch (std::exception &e) {
		printf("%s\n", e.what());
	}
}
