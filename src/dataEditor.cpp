#include <iostream>
#include <memory>
#include <TGUI/TGUI.hpp>
#include "Logger.hpp"
#include "Resources/Screen.hpp"
#include "Resources/Game.hpp"
#include "Objects/ACharacter.hpp"
#include "FrameDataEditor/EditableObject.hpp"
#include "Utils.hpp"

Battle::Logger logger("./editor.log");
float updateTimer = std::numeric_limits<float>::infinity();
float timer = 0;
std::string loadedPath;
bool dragUp = false;
bool dragDown = false;
bool dragLeft = false;
bool dragRight = false;
Battle::Box *selectedBox;
Battle::Box startValues;
std::array<tgui::Button::Ptr, 8> resizeButtons;

void	refreshFrameDataPanel(tgui::Panel::Ptr panel, Battle::EditableObject &object)
{
	auto action = panel->get<tgui::EditBox>("Action");
	auto block = panel->get<tgui::SpinButton>("Block");
	auto progress = panel->get<tgui::Slider>("Progress");
	auto play = panel->get<tgui::Button>("Play");
	auto step = panel->get<tgui::Button>("Step");
	auto speedCtrl = panel->get<tgui::SpinButton>("Speed");
	auto speedLabel = panel->get<tgui::Label>("SpeedLabel");
	auto sprite = panel->get<tgui::EditBox>("Sprite");
	auto offset = panel->get<tgui::EditBox>("Offset");
	auto bounds = panel->get<tgui::EditBox>("Bounds");
	auto size = panel->get<tgui::EditBox>("Size");
	auto rotation = panel->get<tgui::Slider>("Rotation");
	auto collisionBox = panel->get<tgui::CheckBox>("Collision");
	auto duration = panel->get<tgui::EditBox>("Duration");
	auto marker = panel->get<tgui::EditBox>("Marker");
	auto subObj = panel->get<tgui::EditBox>("SubObj");
	auto pushBack = panel->get<tgui::EditBox>("PushBack");
	auto pushBlock = panel->get<tgui::EditBox>("PushBlock");
	auto blockStun = panel->get<tgui::EditBox>("BlockStun");
	auto hitStun = panel->get<tgui::EditBox>("HitStun");
	auto spiritLimit = panel->get<tgui::EditBox>("SpiritLimit");
	auto voidLimit = panel->get<tgui::EditBox>("VoidLimit");
	auto matterLimit = panel->get<tgui::EditBox>("MatterLimit");
	auto prorate = panel->get<tgui::EditBox>("Rate");
	auto oFlags = panel->get<tgui::EditBox>("oFlags");
	auto dFlags = panel->get<tgui::EditBox>("dFlags");
	auto &data = object._moves.at(object._action)[object._actionBlock][object._animation];

	dFlags->setText(std::to_string(data.dFlag.flags));
	oFlags->setText(std::to_string(data.oFlag.flags));
}

void	refreshRightPanel(tgui::Gui &gui, Battle::EditableObject &object)
{
	auto panel = gui.get<tgui::Panel>("Panel1");
	auto animPanel = panel->get<tgui::Panel>("AnimPanel");

	panel->setEnabled(true);
	if (object._moves[0].empty()) {
		object._moves[0].emplace_back();
		object._moves[0][0].emplace_back();
	}
	refreshFrameDataPanel(animPanel, object);
}

void	placeAnimPanelHooks(tgui::Panel::Ptr panel, std::unique_ptr<Battle::EditableObject> &object)
{
	auto c = std::make_shared<bool>(false);
	auto action = panel->get<tgui::EditBox>("Action");
	auto block = panel->get<tgui::SpinButton>("Block");
	auto frame = panel->get<tgui::SpinButton>("Frame");
	auto progress = panel->get<tgui::Slider>("Progress");
	auto play = panel->get<tgui::Button>("Play");
	auto step = panel->get<tgui::Button>("Step");
	auto speedCtrl = panel->get<tgui::SpinButton>("Speed");
	auto speedLabel = panel->get<tgui::Label>("SpeedLabel");
	auto sprite = panel->get<tgui::EditBox>("Sprite");
	auto offset = panel->get<tgui::EditBox>("Offset");
	auto bounds = panel->get<tgui::EditBox>("Bounds");
	auto size = panel->get<tgui::EditBox>("Size");
	auto rotation = panel->get<tgui::Slider>("Rotation");
	auto collisionBox = panel->get<tgui::CheckBox>("Collision");
	auto duration = panel->get<tgui::EditBox>("Duration");
	auto marker = panel->get<tgui::EditBox>("Marker");
	auto subObj = panel->get<tgui::EditBox>("SubObj");
	auto pushBack = panel->get<tgui::EditBox>("PushBack");
	auto pushBlock = panel->get<tgui::EditBox>("PushBlock");
	auto blockStun = panel->get<tgui::EditBox>("BlockStun");
	auto hitStun = panel->get<tgui::EditBox>("HitStun");
	auto spiritLimit = panel->get<tgui::EditBox>("SpiritLimit");
	auto voidLimit = panel->get<tgui::EditBox>("VoidLimit");
	auto matterLimit = panel->get<tgui::EditBox>("MatterLimit");
	auto prorate = panel->get<tgui::EditBox>("Rate");
	auto oFlags = panel->get<tgui::EditBox>("oFlags");
	auto dFlags = panel->get<tgui::EditBox>("dFlags");

	auto grab = panel->get<tgui::CheckBox>("Grab");
	auto aub = panel->get<tgui::CheckBox>("AUB");
	auto ub = panel->get<tgui::CheckBox>("UB");
	auto voidElem = panel->get<tgui::CheckBox>("VoidElem");
	auto spiritElem = panel->get<tgui::CheckBox>("SpiritElement");
	auto matterElem = panel->get<tgui::CheckBox>("MatterElement");
	auto lowHit = panel->get<tgui::CheckBox>("LowHit");
	auto highHit = panel->get<tgui::CheckBox>("HighHit");
	auto autoHitPos = panel->get<tgui::CheckBox>("AutoHitPos");
	auto canCH = panel->get<tgui::CheckBox>("CanCH");
	auto hitSwitch = panel->get<tgui::CheckBox>("HitSwitch");
	auto cancelable = panel->get<tgui::CheckBox>("Cancelable");
	auto jab = panel->get<tgui::CheckBox>("Jab");
	auto resetHit = panel->get<tgui::CheckBox>("ResetHit");
	auto resetSpeed = panel->get<tgui::CheckBox>("ResetSpeed");
	auto restand = panel->get<tgui::CheckBox>("Restand");

	auto invulnerable = panel->get<tgui::CheckBox>("Invulnerable");
	auto invulnerableArmor = panel->get<tgui::CheckBox>("InvulnerableArmor");
	auto superArmor = panel->get<tgui::CheckBox>("SuperArmor");
	auto grabInvul = panel->get<tgui::CheckBox>("GrabInvul");
	auto voidBlock = panel->get<tgui::CheckBox>("VoidBlock");
	auto spiritBlock = panel->get<tgui::CheckBox>("SpiritBlock");
	auto matterBlock = panel->get<tgui::CheckBox>("MatterBlock");
	auto neutralBlock = panel->get<tgui::CheckBox>("NeutralBlock");
	auto airborne = panel->get<tgui::CheckBox>("Air");
	auto canBlock = panel->get<tgui::CheckBox>("CanBlock");
	auto highBlock = panel->get<tgui::CheckBox>("HighBlock");
	auto lowBlock = panel->get<tgui::CheckBox>("LowBlock");
	auto dashSpeed = panel->get<tgui::CheckBox>("DashSpeed");
	auto resetRotation = panel->get<tgui::CheckBox>("ResetRotation");
	auto counterHit = panel->get<tgui::CheckBox>("CounterHit");

	grab->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.grab = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	aub->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.airUnblockable = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	ub->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.unblockable = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	voidElem->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.voidElement = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	spiritElem->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.spiritElement = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	matterElem->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.matterElement = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	lowHit->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.lowHit = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	highHit->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.highHit = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	autoHitPos->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.autoHitPos = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	canCH->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.canCounterHit = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	hitSwitch->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.hitSwitch = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	cancelable->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.cancelable = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	jab->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.jab = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	resetHit->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.resetHits = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	resetSpeed->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.resetSpeed = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	restand->connect("Changed", [oFlags, &object, c](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.restand = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	oFlags->connect("TextChanged", [c, &object, grab, aub, ub, voidElem, spiritElem, matterElem, lowHit, highHit, autoHitPos, canCH, hitSwitch, cancelable, jab, resetHit, resetSpeed, restand](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.flags = std::stoul(t);
		grab->setChecked(data.oFlag.grab);
		aub->setChecked(data.oFlag.airUnblockable);
		ub->setChecked(data.oFlag.unblockable);
		voidElem->setChecked(data.oFlag.voidElement);
		spiritElem->setChecked(data.oFlag.spiritElement);
		matterElem->setChecked(data.oFlag.matterElement);
		lowHit->setChecked(data.oFlag.lowHit);
		highHit->setChecked(data.oFlag.highHit);
		autoHitPos->setChecked(data.oFlag.autoHitPos);
		canCH->setChecked(data.oFlag.canCounterHit);
		hitSwitch->setChecked(data.oFlag.hitSwitch);
		cancelable->setChecked(data.oFlag.cancelable);
		jab->setChecked(data.oFlag.jab);
		resetHit->setChecked(data.oFlag.resetHits);
		resetSpeed->setChecked(data.oFlag.resetSpeed);
		restand->setChecked(data.oFlag.restand);
		*c = false;
	});

	invulnerable->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.invulnerable = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	invulnerableArmor->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.invulnerableArmor = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	superArmor->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.superarmor = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	grabInvul->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.grabInvulnerable = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	voidBlock->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.voidBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	spiritBlock->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.spiritBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	matterBlock->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.matterBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	neutralBlock->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.neutralBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	airborne->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.airborne = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	canBlock->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.canBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	highBlock->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.highBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	lowBlock->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.lowBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	dashSpeed->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.dashSpeed = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	resetRotation->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.resetRotation = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	counterHit->connect("Changed", [c, &object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.counterHit = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	dFlags->connect("TextChanged", [invulnerable, invulnerableArmor, superArmor, grabInvul, voidBlock, spiritBlock, matterBlock, neutralBlock, airborne, canBlock, highBlock, lowBlock, dashSpeed, resetRotation, counterHit, c, &object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.flags = std::stoul(t);
		invulnerable->setChecked(data.dFlag.invulnerable);
		invulnerableArmor->setChecked(data.dFlag.invulnerableArmor);
		superArmor->setChecked(data.dFlag.superarmor);
		grabInvul->setChecked(data.dFlag.grabInvulnerable);
		voidBlock->setChecked(data.dFlag.voidBlock);
		spiritBlock->setChecked(data.dFlag.spiritBlock);
		matterBlock->setChecked(data.dFlag.matterBlock);
		neutralBlock->setChecked(data.dFlag.neutralBlock);
		airborne->setChecked(data.dFlag.airborne);
		canBlock->setChecked(data.dFlag.canBlock);
		highBlock->setChecked(data.dFlag.highBlock);
		lowBlock->setChecked(data.dFlag.lowBlock);
		dashSpeed->setChecked(data.dFlag.dashSpeed);
		resetRotation->setChecked(data.dFlag.resetRotation);
		counterHit->setChecked(data.dFlag.counterHit);
		*c = false;
	});
}

void	placeGuiHooks(tgui::Gui &gui, std::unique_ptr<Battle::EditableObject> &object)
{
	auto bar = gui.get<tgui::MenuBar>("main_bar");
	auto panel = gui.get<tgui::Panel>("Panel1");
	auto animPanel = panel->get<tgui::Panel>("AnimPanel");

	placeAnimPanelHooks(animPanel, object);
	bar->connectMenuItem({"File", "Save"}, [&object]{
		if (!object)
			return;
		if (loadedPath.empty())
			loadedPath = Utils::saveFileDialog("Save framedata", "assets", {{".*\\.json", "Frame data file"}});
		if (loadedPath.empty())
			return;

		std::ofstream stream{loadedPath};

		if (stream.fail()) {
			Utils::dispMsg("Saving failed", loadedPath + ": " + strerror(errno), MB_ICONERROR);
			return;
		}

		nlohmann::json j = nlohmann::json::array();

		for (auto &[key, value] : object->_moves) {
			j.push_back({
				{"action", key},
				{"framedata", value}
			});
		}
		stream << j.dump(4) << std::endl;
	});
	bar->connectMenuItem({"File", "Save as"}, [&object]{
		if (!object)
			return;

		auto path = Utils::saveFileDialog("Save framedata", loadedPath.empty() ? "assets" : loadedPath, {{".*\\.json", "Frame data file"}});

		if (path.empty())
			return;
		loadedPath = path;

		std::ofstream stream{path};

		if (stream.fail()) {
			Utils::dispMsg("Saving failed", path + ": " + strerror(errno), MB_ICONERROR);
			return;
		}

		nlohmann::json j = nlohmann::json::array();

		for (auto &[key, value] : object->_moves) {
			j.push_back({
				{"action", key},
				{"framedata", value}
			});
		}
		stream << j.dump(4) << std::endl;
	});
	bar->connectMenuItem({"File", "New framedata"}, [&gui, &object]{
		object = std::make_unique<Battle::EditableObject>();
		loadedPath.clear();
		refreshRightPanel(gui, *object);
	});
	bar->connectMenuItem({"File", "Load framedata"}, [&gui, &object]{
		auto path = Utils::openFileDialog("Open framedata", "assets", {{".*\\.json", "Frame data file"}});

		if (path.empty())
			return;
		try {
			object = std::make_unique<Battle::EditableObject>(path);
			loadedPath = path;
			refreshRightPanel(gui, *object);
		} catch (std::exception &e) {
			Utils::dispMsg("Error", e.what(), MB_ICONERROR);
		}
	});

	for (unsigned i = 0; i < resizeButtons.size(); i++) {
		auto &resizeButton = resizeButtons[i];

		resizeButton = tgui::Button::create();
		resizeButton->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
		resizeButton->getRenderer()->setBorderColor(tgui::Color::Red);
		resizeButton->connect("MousePressed", [i]{
			startValues = *selectedBox;
			dragUp = i < 3;
			dragDown = i > 4;
			dragLeft = i == 0 || i == 3 || i == 5;
			dragRight = i == 2 || i == 4 || i == 7;
		});
	}
}

void	run()
{
	Battle::game.screen = std::make_unique<Battle::Screen>("Frame data editor");

	std::unique_ptr<Battle::EditableObject> object;
	tgui::Gui gui{*Battle::game.screen};
	sf::View view{{-535, -480, 1680, 960}};
	sf::Image icon;
	sf::Event event;

	if (icon.loadFromFile("assets/editorIcon.png"))
		Battle::game.screen->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	gui.loadWidgetsFromFile("assets/gui/editor.gui");
	placeGuiHooks(gui, object);
	Battle::game.screen->setView(view);
	while (Battle::game.screen->isOpen()) {
		timer++;
		Battle::game.screen->clear(sf::Color::Cyan);
		if (object) {
			if (timer >= updateTimer) {
				object->update();
				timer -= updateTimer;
			}
			object->render();
		}

		while (Battle::game.screen->pollEvent(event)) {
			gui.handleEvent(event);
			if (event.type == sf::Event::Closed)
				Battle::game.screen->close();
		}
		gui.draw();
		Battle::game.screen->display();
	}
}

int	main()
{
	logger.info("Starting editor.");
	run();
	logger.info("Goodbye !");
	return EXIT_SUCCESS;
}