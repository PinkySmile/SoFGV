#include <iostream>
#include <memory>
#include <TGUI/TGUI.hpp>
#include "Logger.hpp"
#include "Resources/Screen.hpp"
#include "Resources/Game.hpp"
#include "FrameDataEditor/EditableObject.hpp"
#include "Utils.hpp"

auto c = std::make_shared<bool>(false);
Battle::Logger logger("./editor.log");
float updateTimer = 0;
float timer = 0;
std::string loadedPath;
bool canDrag = false;
bool dragStart = false;
bool dragUp = false;
bool dragDown = false;
bool dragLeft = false;
bool dragRight = false;
bool updateAnyway = false;
bool spriteSelected = false;
bool quitRequest = false;
tgui::Color normalColor;
tgui::Button::Ptr boxButton;
Battle::Vector2i mouseStart;
Battle::Vector2i lastMouse;
Battle::Box *selectedBox;
std::array<tgui::Button::Ptr, 8> resizeButtons;

void	arrangeButtons(Battle::EditableObject *object)
{
	auto *data = object ? &object->_moves.at(object->_action)[object->_actionBlock][object->_animation] : nullptr;
	Battle::Box box = spriteSelected ? Battle::Box{{static_cast<int>(data->offset.x - data->size.x / 2), static_cast<int>(-data->offset.y - data->size.y)}, data->size} : *selectedBox;

	for (int i = 0; i < 8; i++) {
		Battle::Vector2i pos;
		auto resizeButton = resizeButtons[i];

		if (i == 0 || i == 3 || i == 5)
			pos.x = box.pos.x - resizeButton->getSize().x;
		else if (i == 2 || i == 4 || i == 7)
			pos.x = box.pos.x + box.size.x;
		else
			pos.x = box.pos.x - resizeButton->getSize().x / 2 + box.size.x / 2;

		if (i < 3)
			pos.y = box.pos.y - resizeButton->getSize().x;
		else if (i > 4)
			pos.y = box.pos.y + box.size.y;
		else
			pos.y = box.pos.y - resizeButton->getSize().x / 2 + box.size.y / 2;
		resizeButton->setPosition("boxes.w / 2 + " + std::to_string(pos.x), "boxes.h / 2 + " + std::to_string(pos.y + 300));
	}
}

void	selectBox(tgui::Button::Ptr button, Battle::Box *box)
{
	spriteSelected = false;
	selectedBox = box;
	if (boxButton)
		boxButton->getRenderer()->setBackgroundColor(normalColor);
	if (button) {
		normalColor = button->getRenderer()->getBackgroundColor();
		button->getRenderer()->setBackgroundColor(button->getRenderer()->getBackgroundColorFocused());
	}
	boxButton = button;
	for (auto &b : resizeButtons)
		b->setVisible(box != nullptr);
	if (box)
		arrangeButtons(nullptr);
}

void	selectSprite(tgui::Button::Ptr button, std::unique_ptr<Battle::EditableObject> &object)
{
	boxButton = button;
	normalColor = button->getRenderer()->getBackgroundColor();
	button->getRenderer()->setBackgroundColor(button->getRenderer()->getBackgroundColorFocused());
	spriteSelected = true;
	for (auto &b : resizeButtons)
		b->setVisible(true);
	arrangeButtons(&*object);
}

void	refreshBoxes(tgui::Panel::Ptr panel, Battle::FrameData &data, std::unique_ptr<Battle::EditableObject> &object)
{
	int i = 0;
	auto button = tgui::Button::create();
	auto renderer = button->getRenderer();

	panel->removeAllWidgets();
	renderer->setBackgroundColor({0xFF, 0xFF, 0xFF, 0x00});
	renderer->setBackgroundColorDown({0xFF, 0xFF, 0xFF, 0xA0});
	renderer->setBackgroundColorHover({0xFF, 0xFF, 0xFF, 0x80});
	renderer->setBackgroundColorDisabled({0xFF, 0xFF, 0xFF, 0x4C});
	renderer->setBackgroundColorFocused({0xFF, 0xFF, 0xFF, 0xA0});
	renderer->setBorderColor({0xFF, 0xFF, 0xFF});
	renderer->setBorderColorDown({0xFF, 0xFF, 0xFF});
	renderer->setBorderColorHover({0xFF, 0xFF, 0xFF});
	renderer->setBorderColorDisabled({0xFF, 0xFF, 0xFF});
	renderer->setBorderColorFocused({0xFF, 0xFF, 0xFF});
	renderer->setBorders(1);
	button->setSize(data.size.x, data.size.y);
	button->setPosition(
		"&.w / 2 + " + std::to_string(static_cast<int>(data.offset.x - data.size.x / 2)),
		"&.h / 2 + " + std::to_string(-data.size.y - data.offset.y + 300)
	);
	button->connect("MousePressed", [&object](std::weak_ptr<tgui::Button> self){
		selectSprite(self.lock(), object);
		canDrag = true;
	}, std::weak_ptr<tgui::Button>(button));
	panel->add(button, "SpriteBox");

	for (auto &box : data.hurtBoxes) {
		button = tgui::Button::create();
		renderer = button->getRenderer();
		renderer->setBackgroundColor({0x00, 0xFF, 0x00, 0x4C});
		renderer->setBackgroundColorDown({0x00, 0xFF, 0x00, 0xA0});
		renderer->setBackgroundColorHover({0x00, 0xFF, 0x00, 0x80});
		renderer->setBackgroundColorDisabled({0x00, 0xFF, 0x00, 0x4C});
		renderer->setBackgroundColorFocused({0x00, 0xFF, 0x00, 0xA0});
		renderer->setBorderColor({0x00, 0xFF, 0x00});
		renderer->setBorderColorDown({0x00, 0xFF, 0x00});
		renderer->setBorderColorHover({0x00, 0xFF, 0x00});
		renderer->setBorderColorDisabled({0x00, 0xFF, 0x00});
		renderer->setBorderColorFocused({0x00, 0xFF, 0x00});
		renderer->setBorders(1);
		button->setSize(box.size.x, box.size.y);
		button->setPosition("&.w / 2 + " + std::to_string(box.pos.x), "&.h / 2 + " + std::to_string(box.pos.y + 300));
		button->connect("MousePressed", [&box](std::weak_ptr<tgui::Button> self){
			selectBox(self.lock(), &box);
			canDrag = true;
		}, std::weak_ptr<tgui::Button>(button));
		panel->add(button, "HurtBox" + std::to_string(i));
		i++;
	}
	i = 0;
	for (auto &box : data.hitBoxes) {
		button = tgui::Button::create();
		renderer = button->getRenderer();
		renderer->setBackgroundColor({0xFF, 0x00, 0x00, 0x4C});
		renderer->setBackgroundColorDown({0xFF, 0x00, 0x00, 0xA0});
		renderer->setBackgroundColorHover({0xFF, 0x00, 0x00, 0x80});
		renderer->setBackgroundColorDisabled({0xFF, 0x00, 0x00, 0x4C});
		renderer->setBackgroundColorFocused({0xFF, 0x00, 0x00, 0xA0});
		renderer->setBorderColor({0xFF, 0x00, 0x00});
		renderer->setBorderColorDown({0xFF, 0x00, 0x00});
		renderer->setBorderColorHover({0xFF, 0x00, 0x00});
		renderer->setBorderColorDisabled({0xFF, 0x00, 0x00});
		renderer->setBorderColorFocused({0xFF, 0x00, 0x00});
		renderer->setBorders(1);
		button->setSize(box.size.x, box.size.y);
		button->setPosition("&.w / 2 + " + std::to_string(box.pos.x), "&.h / 2 + " + std::to_string(box.pos.y + 300));
		button->connect("MousePressed", [&box](std::weak_ptr<tgui::Button> self){
			selectBox(self.lock(), &box);
			canDrag = true;
		}, std::weak_ptr<tgui::Button>(button));
		panel->add(button, "HitBox" + std::to_string(i));
		i++;
	}
	if (data.collisionBox) {
		button = tgui::Button::create();
		renderer = button->getRenderer();
		renderer->setBackgroundColor({0xFF, 0xFF, 0x00, 0x4C});
		renderer->setBackgroundColorDown({0xFF, 0xFF, 0x00, 0xA0});
		renderer->setBackgroundColorHover({0xFF, 0xFF, 0x00, 0x80});
		renderer->setBackgroundColorDisabled({0xFF, 0xFF, 0x00, 0x4C});
		renderer->setBackgroundColorFocused({0xFF, 0xFF, 0x00, 0xA0});
		renderer->setBorderColor({0xFF, 0xFF, 0x00});
		renderer->setBorderColorDown({0xFF, 0xFF, 0x00});
		renderer->setBorderColorHover({0xFF, 0xFF, 0x00});
		renderer->setBorderColorDisabled({0xFF, 0xFF, 0x00});
		renderer->setBorderColorFocused({0xFF, 0xFF, 0x00});
		renderer->setBorders(1);
		button->setSize(data.collisionBox->size.x, data.collisionBox->size.y);
		button->setPosition("&.w / 2 + " + std::to_string(data.collisionBox->pos.x), "&.h / 2 + " + std::to_string(data.collisionBox->pos.y + 300));
		button->connect("MousePressed", [&data](std::weak_ptr<tgui::Button> self){
			selectBox(self.lock(), data.collisionBox);
			canDrag = true;
		}, std::weak_ptr<tgui::Button>(button));
		panel->add(button, "CollisionBox");
	}
}

void	refreshFrameDataPanel(tgui::Panel::Ptr panel, tgui::Panel::Ptr boxes, std::unique_ptr<Battle::EditableObject> &object)
{
	auto progress = panel->get<tgui::Slider>("Progress");
	auto manaGain = panel->get<tgui::EditBox>("ManaGain");
	auto manaCost = panel->get<tgui::EditBox>("ManaCost");
	auto neutralLimit = panel->get<tgui::EditBox>("NLimit");
	auto gravity = panel->get<tgui::EditBox>("Gravity");
	auto sprite = panel->get<tgui::EditBox>("Sprite");
	auto sound = panel->get<tgui::EditBox>("SFX");
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
	auto hitStop = panel->get<tgui::EditBox>("HitStop");
	auto hitSpeed = panel->get<tgui::EditBox>("HitSpeed");
	auto counterHitSpeed = panel->get<tgui::EditBox>("CHSpeed");
	auto spiritLimit = panel->get<tgui::EditBox>("SpiritLimit");
	auto voidLimit = panel->get<tgui::EditBox>("VoidLimit");
	auto matterLimit = panel->get<tgui::EditBox>("MatterLimit");
	auto prorate = panel->get<tgui::EditBox>("Rate");
	auto damage = panel->get<tgui::EditBox>("Damage");
	auto speed = panel->get<tgui::EditBox>("MoveSpeed");
	auto oFlags = panel->get<tgui::EditBox>("oFlags");
	auto dFlags = panel->get<tgui::EditBox>("dFlags");
	auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
	auto actionName = panel->get<tgui::Button>("ActionName");
	auto name = Battle::actionNames.find(static_cast<Battle::CharacterActions>(object->_action));

	logger.debug("Soft refresh");
	*c = true;
	actionName->setText(name == Battle::actionNames.end() ? "Action #" + std::to_string(object->_action) : name->second);
	dFlags->setText(std::to_string(data.dFlag.flags));
	oFlags->setText(std::to_string(data.oFlag.flags));
	progress->setMinimum(0);
	progress->setMaximum(object->_moves.at(object->_action)[object->_actionBlock].size() - 1);
	progress->setValue(object->_animation);
	sprite->setText(data.spritePath);
	sound->setText(data.soundPath);
	damage->setText(std::to_string(data.damage));
	duration->setText(std::to_string(data.duration));
	marker->setText(std::to_string(data.specialMarker));
	subObj->setText(std::to_string(data.subObjectSpawn));
	pushBack->setText(std::to_string(data.pushBack));
	pushBlock->setText(std::to_string(data.pushBlock));
	blockStun->setText(std::to_string(data.blockStun));
	hitStun->setText(std::to_string(data.hitStun));
	hitStop->setText(std::to_string(data.hitStop));
	spiritLimit->setText(std::to_string(data.spiritLimit));
	voidLimit->setText(std::to_string(data.voidLimit));
	matterLimit->setText(std::to_string(data.matterLimit));
	prorate->setText(std::to_string(data.prorate));
	manaGain->setText(std::to_string(data.manaGain));
	manaCost->setText(std::to_string(data.manaCost));
	neutralLimit->setText(std::to_string(data.neutralLimit));

	auto newBounds = "(" + std::to_string(data.textureBounds.pos.x) + "," + std::to_string(data.textureBounds.pos.y) + "," + std::to_string(data.textureBounds.size.x) + "," + std::to_string(data.textureBounds.size.y) + ")";
	auto newSize = "(" + std::to_string(data.size.x) + "," + std::to_string(data.size.y) + ")";
	auto newOffset = "(" + std::to_string(data.offset.x) + "," + std::to_string(data.offset.y) + ")";
	auto newSpeed = "(" + std::to_string(data.speed.x) + "," + std::to_string(data.speed.y) + ")";
	auto newCHitSpeed = "(" + std::to_string(data.counterHitSpeed.x) + "," + std::to_string(data.counterHitSpeed.y) + ")";
	auto newHitSpeed = "(" + std::to_string(data.hitSpeed.x) + "," + std::to_string(data.hitSpeed.y) + ")";
	auto newGravity = data.gravity ? "(" + std::to_string(data.gravity->x) + "," + std::to_string(data.gravity->y) + ")" : "";

	counterHitSpeed->setText(newCHitSpeed);
	hitSpeed->setText(newHitSpeed);
	speed->setText(newSpeed);
	offset->setText(newOffset);
	bounds->setText(newBounds);
	size->setText(newSize);
	gravity->setText(newGravity);
	rotation->setValue(data.rotation * 180 / M_PI);
	collisionBox->setChecked(data.collisionBox != nullptr);
	selectBox(nullptr, nullptr);
	refreshBoxes(boxes, data, object);
	*c = false;
}

void	refreshRightPanel(tgui::Gui &gui, std::unique_ptr<Battle::EditableObject> &object, bool resetAction = true)
{
	auto panel = gui.get<tgui::Panel>("Panel1");
	auto animPanel = panel->get<tgui::Panel>("AnimPanel");
	auto action = panel->get<tgui::EditBox>("Action");
	auto block = panel->get<tgui::SpinButton>("Block");
	auto play = panel->get<tgui::Button>("Play");
	auto step = panel->get<tgui::Button>("Step");
	auto speedCtrl = panel->get<tgui::SpinButton>("Speed");
	auto speedLabel = panel->get<tgui::Label>("SpeedLabel");
	auto frame = panel->get<tgui::SpinButton>("Frame");

	logger.debug("Hard refresh");
	panel->setEnabled(static_cast<bool>(object));
	if (!object)
		return gui.get<tgui::Panel>("Boxes")->removeAllWidgets();
	step->onPress.emit(&*step, "Step");
	if (resetAction) {
		action->setText("0");
		action->onReturnKeyPress.emit(&*action, "0");
	}
	if (speedCtrl->getValue() == 0)
		speedCtrl->onValueChange.emit(&*speedCtrl, 0);
	else
		speedCtrl->setValue(0);
	if (object->_moves[0].empty()) {
		object->_moves[0].emplace_back();
		object->_moves[0][0].emplace_back();
	}
	refreshFrameDataPanel(panel, gui.get<tgui::Panel>("Boxes"), object);
}

void	placeAnimPanelHooks(tgui::Gui &gui, tgui::Panel::Ptr panel, tgui::Panel::Ptr boxes, std::unique_ptr<Battle::EditableObject> &object)
{
	auto panWeak = std::weak_ptr<tgui::Panel>(panel);
	auto animPanel = panel->get<tgui::Panel>("AnimPanel");
	auto action = panel->get<tgui::EditBox>("Action");
	auto actionName = panel->get<tgui::Button>("ActionName");
	auto block = panel->get<tgui::SpinButton>("Block");
	auto blockLabel = panel->get<tgui::Label>("Label1");
	auto frameLabel = panel->get<tgui::Label>("Label2");
	auto progress = panel->get<tgui::Slider>("Progress");
	auto play = panel->get<tgui::Button>("Play");
	auto step = panel->get<tgui::Button>("Step");
	auto speedCtrl = panel->get<tgui::SpinButton>("Speed");
	auto speedLabel = panel->get<tgui::Label>("SpeedLabel");
	auto sprite = panel->get<tgui::EditBox>("Sprite");
	auto sfx = panel->get<tgui::EditBox>("SFX");
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
	auto damage = panel->get<tgui::EditBox>("Damage");
	auto manaGain = panel->get<tgui::EditBox>("ManaGain");
	auto manaCost = panel->get<tgui::EditBox>("ManaCost");
	auto hitStop = panel->get<tgui::EditBox>("HitStop");
	auto hitSpeed = panel->get<tgui::EditBox>("HitSpeed");
	auto neutralLimit = panel->get<tgui::EditBox>("NLimit");
	auto gravity = panel->get<tgui::EditBox>("Gravity");
	auto speed = panel->get<tgui::EditBox>("MoveSpeed");
	auto counterHitSpeed = panel->get<tgui::EditBox>("CHSpeed");
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
	auto super = panel->get<tgui::CheckBox>("Super");
	auto ultimate = panel->get<tgui::CheckBox>("Ultimate");
	auto jumpCancelable = panel->get<tgui::CheckBox>("JumpCancel");
	auto transformCancelable = panel->get<tgui::CheckBox>("TransCancel");
	auto unTransformCancelable = panel->get<tgui::CheckBox>("UnTransCancel");
	auto dashCancelable = panel->get<tgui::CheckBox>("DashCancel");
	auto backDashCancelable = panel->get<tgui::CheckBox>("BDCancel");
	auto voidMana = panel->get<tgui::CheckBox>("VoidMana");
	auto matterMana = panel->get<tgui::CheckBox>("MatterMana");
	auto spiritMana = panel->get<tgui::CheckBox>("SpiritMana");

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
	auto flash = panel->get<tgui::CheckBox>("Flash");
	auto crouch = panel->get<tgui::CheckBox>("Crouch");
	auto projInvul = panel->get<tgui::CheckBox>("ProjInvul");
	auto proj = panel->get<tgui::CheckBox>("Proj");
	auto lc = panel->get<tgui::CheckBox>("LC");
	auto dc = panel->get<tgui::CheckBox>("DC");
	auto resOPS = panel->get<tgui::CheckBox>("ResetOPSpeed");

	actionName->connect("Clicked", [&gui, &object, block, action]{
		auto window = Utils::openWindowWithFocus(gui, 500, "&.h - 100");
		auto pan = tgui::ScrollablePanel::create({"&.w", "&.h"});
		unsigned i = 0;

		window->setTitle("Default character moves");
		window->add(pan);
		for (auto &move : Battle::actionNames) {
			auto label = tgui::Label::create(std::to_string(move.first));
			auto button = tgui::Button::create(move.second);

			label->setPosition(10, i * 25 + 12);
			button->setPosition(50, i * 25 + 10);
			button->setSize(430, 20);
			if (object->_moves.find(move.first) == object->_moves.end()) {
				button->getRenderer()->setTextColor(tgui::Color::Red);
				button->getRenderer()->setTextColorHover(tgui::Color{0xFF, 0x40, 0x40});
				button->getRenderer()->setTextColorDisabled(tgui::Color{0xA0, 0, 0});
				button->getRenderer()->setTextColorDown(tgui::Color{0x80, 0x00, 0x00});
				button->getRenderer()->setTextColorFocused(tgui::Color{0x80, 0x20, 0x20});
			}

			button->connect("Clicked", [move, action](std::weak_ptr<tgui::ChildWindow> win){
				win.lock()->close();
				action->setText(std::to_string(move.first));
				action->onReturnKeyPress.emit(&*action, action->getText());
			}, std::weak_ptr<tgui::ChildWindow>(window));

			pan->add(label);
			pan->add(button);
			i++;
		}

		auto label = tgui::Label::create("");

		label->setPosition(10, i * 25 + 10);
		label->setSize(10, 5);
		pan->add(label);
	});
	boxes->connect("Clicked", []{
		if (!dragStart && !canDrag)
			selectBox(nullptr, nullptr);
	});
	action->connect("ReturnKeyPressed", [&object, block](const std::string &t){
		if (t.empty())
			return;
		auto newAction = std::stoul(t);
		if (object->_moves[newAction].empty())
			object->_moves[newAction] = object->_moves[object->_action];
		object->_action = newAction;
		block->setMaximum(object->_moves[object->_action].size() - 1);
		block->setMinimum(0);
		if (block->getValue() == 0)
			block->onValueChange.emit(&*block, 0);
		else
			block->setValue(0);
	});
	block->connect("ValueChanged", [&object, blockLabel, progress](float f){
		int i = f;

		blockLabel->setText("Block " + std::to_string(i));
		object->_actionBlock = i;
		if (object->_moves[object->_action][i].empty())
			object->_moves[object->_action][i].emplace_back();
		progress->setMaximum(object->_moves[object->_action][i].size() - 1);
		progress->setMinimum(0);
		if (progress->getValue() == 0)
			progress->onValueChange.emit(&*progress, 0);
		else
			progress->setValue(0);
	});
	progress->connect("ValueChanged", [boxes, &object, frameLabel, panWeak](float f){
		int i = f;

		frameLabel->setText("Frame " + std::to_string(i));
		object->_animation = i;
		object->_animationCtr = 0;
		refreshFrameDataPanel(panWeak.lock(), boxes, object);
	});
	play->connect("Clicked", [speedLabel, speedCtrl]{
		speedCtrl->setValue(1);
	});
	step->connect("Clicked", [speedLabel, speedCtrl]{
		updateAnyway = true;
		speedCtrl->setValue(0);
	});
	speedCtrl->connect("ValueChanged", [speedLabel, animPanel](float f){
		speedLabel->setText(f == 0 ? "Paused" : "x" + std::to_string(f));
		updateTimer = 1 / f;
		timer = 0;
		animPanel->setEnabled(f == 0);
	});
	sprite->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.spritePath = t;
		data.reloadTexture();
	});
	sfx->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.soundPath = t;
		data.reloadSound();
	});
	speed->connect("TextChanged", [boxes, &object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 1);
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			std::stol(y);
			data.speed.x = std::stol(x);
			data.speed.y = std::stol(y);
			refreshBoxes(boxes, data, object);
			if (spriteSelected)
				arrangeButtons(&*object);
		} catch (std::exception &) {}
	});
	offset->connect("TextChanged", [boxes, &object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 1);
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			std::stol(y);
			data.offset.x = std::stol(x);
			data.offset.y = std::stol(y);
			refreshBoxes(boxes, data, object);
			if (spriteSelected)
				arrangeButtons(&*object);
		} catch (std::exception &) {}
	});
	hitSpeed->connect("TextChanged", [boxes, &object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 1);
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			data.hitSpeed.x = std::stol(x);
			data.hitSpeed.y = std::stol(y);
		} catch (...) {}
		refreshBoxes(boxes, data, object);
		if (spriteSelected)
			arrangeButtons(&*object);
	});
	counterHitSpeed->connect("TextChanged", [boxes, &object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 1);
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			data.counterHitSpeed.x = std::stol(x);
			data.counterHitSpeed.y = std::stol(y);
		} catch (...) {}
		refreshBoxes(boxes, data, object);
		if (spriteSelected)
			arrangeButtons(&*object);
	});
	size->connect("TextChanged", [&object, boxes](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 1);
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			std::stoul(y);
			data.size.x = std::stoul(x);
			data.size.y = std::stoul(y);
		} catch (...) {}
		refreshBoxes(boxes, data, object);
		if (spriteSelected)
			arrangeButtons(&*object);
	});
	gravity->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		if (t.size() <= 2) {
			data.gravity.reset();
			return;
		}
		if (t.front() != '(' || t.back() != ')') {
			data.gravity.reset();
			return;
		}

		auto pos = t.find(',');

		if (pos == std::string::npos) {
			data.gravity.reset();
			return;
		}

		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 2);

		try {
			data.gravity = Battle::Vector2f(
				std::stof(x),
				std::stof(y)
			);
		} catch (...) {
			data.gravity.reset();
			return;
		}
	});
	bounds->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);

		auto remainder = t.substr(pos + 1);
		auto pos2 = remainder.find(',');
		auto y = remainder.substr(0, pos2 + 1);

		auto remainder2 = remainder.substr(pos2 + 1);
		auto pos3 = remainder2.find(',');
		auto w = remainder2.substr(0, pos3 + 1);
		auto h = remainder2.substr(pos3 + 1, remainder2.size() - pos3 - 1);;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			std::stoul(x);
			std::stoul(y);
			std::stoul(w);
			data.textureBounds.pos.x = std::stoul(x);
			data.textureBounds.pos.y = std::stoul(y);
			data.textureBounds.size.x = std::stoul(w);
			data.textureBounds.size.y = std::stoul(h);
		} catch (...) {}
	});
	rotation->connect("ValueChanged", [&object](float f){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.rotation = f * M_PI / 180;
	});
	duration->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.duration = std::stoul(t);
	});
	damage->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.damage = std::stoul(t);
	});
	marker->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.specialMarker = std::stoul(t);
	});
	manaGain->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.manaGain = std::stoul(t);
	});
	manaCost->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.manaCost = std::stoul(t);
	});
	hitStop->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.hitStop = std::stoul(t);
	});
	subObj->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.subObjectSpawn = std::stoul(t);
	});
	pushBack->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.pushBack = std::stoi(t);
	});
	pushBlock->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.pushBlock = std::stoi(t);
	});
	blockStun->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.blockStun = std::stoul(t);
	});
	hitStun->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.hitStun = std::stoul(t);
	});
	neutralLimit->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.neutralLimit = std::stoul(t);
	});
	spiritLimit->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.spiritLimit = std::stoul(t);
	});
	voidLimit->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.voidLimit = std::stoul(t);
	});
	matterLimit->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.matterLimit = std::stoul(t);
	});
	prorate->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.prorate = std::stof(t);
	});
	collisionBox->connect("Checked", [&object, boxes]{
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		if (!data.collisionBox)
			data.collisionBox = new Battle::Box{{-static_cast<int>(data.size.x) / 2, 0}, data.size};
		refreshBoxes(boxes, data, object);
	});
	collisionBox->connect("Unchecked", [&object, boxes]{
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		if (selectedBox && selectedBox == data.collisionBox)
			selectBox(nullptr, nullptr);
		delete data.collisionBox;
		data.collisionBox = nullptr;
		refreshBoxes(boxes, data, object);
	});

	grab->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.grab = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	aub->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.airUnblockable = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	ub->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.unblockable = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	voidElem->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.voidElement = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	spiritElem->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.spiritElement = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	matterElem->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.matterElement = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	lowHit->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.lowHit = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	highHit->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.highHit = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	autoHitPos->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.autoHitPos = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	canCH->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.canCounterHit = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	hitSwitch->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.hitSwitch = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	cancelable->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.cancelable = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	jab->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.jab = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	resetHit->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.resetHits = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	resetSpeed->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.resetSpeed = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	restand->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.restand = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	super->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.super = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	ultimate->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.ultimate = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	jumpCancelable->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.jumpCancelable = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	transformCancelable->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.transformCancelable = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	unTransformCancelable->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.unTransformCancelable = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	dashCancelable->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.dashCancelable = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	backDashCancelable->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.backDashCancelable = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	voidMana->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.voidMana = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	matterMana->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.matterMana = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	spiritMana->connect("Changed", [oFlags, &object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.oFlag.spiritMana = b;
		oFlags->setText(std::to_string(data.oFlag.flags));
		*c = false;
	});
	oFlags->connect("TextChanged", [&object, voidMana, matterMana, spiritMana, super, ultimate, jumpCancelable, transformCancelable, unTransformCancelable, dashCancelable, backDashCancelable, grab, aub, ub, voidElem, spiritElem, matterElem, lowHit, highHit, autoHitPos, canCH, hitSwitch, cancelable, jab, resetHit, resetSpeed, restand](std::string t){
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
		auto g = *c;

		if (!*c) {
			*c = true;
			data.oFlag.flags = std::stoul(t);
		}
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
		super->setChecked(data.oFlag.super);
		ultimate->setChecked(data.oFlag.ultimate);
		jumpCancelable->setChecked(data.oFlag.jumpCancelable);
		transformCancelable->setChecked(data.oFlag.transformCancelable);
		unTransformCancelable->setChecked(data.oFlag.unTransformCancelable);
		dashCancelable->setChecked(data.oFlag.dashCancelable);
		backDashCancelable->setChecked(data.oFlag.backDashCancelable);
		voidMana->setChecked(data.oFlag.voidMana);
		matterMana->setChecked(data.oFlag.matterMana);
		spiritMana->setChecked(data.oFlag.spiritMana);
		if (!g)
			*c = false;
	});

	invulnerable->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.invulnerable = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	invulnerableArmor->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.invulnerableArmor = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	superArmor->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.superarmor = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	grabInvul->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.grabInvulnerable = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	voidBlock->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.voidBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	spiritBlock->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.spiritBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	matterBlock->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.matterBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	neutralBlock->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.neutralBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	airborne->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.airborne = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	canBlock->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.canBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	highBlock->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.highBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	lowBlock->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.lowBlock = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	dashSpeed->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.charaCancel = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	resetRotation->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.resetRotation = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	counterHit->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.counterHit = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	flash->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.flash = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	crouch->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.crouch = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	projInvul->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.projectileInvul = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	proj->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.projectile = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	lc->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.landCancel = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	dc->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.dashCancel = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	resOPS->connect("Changed", [&object, dFlags](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		*c = true;
		data.dFlag.resetSpeed = b;
		dFlags->setText(std::to_string(data.dFlag.flags));
		*c = false;
	});
	dFlags->connect("TextChanged", [projInvul, proj, resOPS, lc, dc, crouch, flash, invulnerable, invulnerableArmor, superArmor, grabInvul, voidBlock, spiritBlock, matterBlock, neutralBlock, airborne, canBlock, highBlock, lowBlock, dashSpeed, resetRotation, counterHit, &object](std::string t){
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
		auto g = *c;

		if (!*c) {
			*c = true;
			data.dFlag.flags = std::stoul(t);
		}
		invulnerable->setChecked(data.dFlag.invulnerable);
		invulnerableArmor->setChecked(data.dFlag.invulnerableArmor);
		superArmor->setChecked(data.dFlag.superarmor);
		grabInvul->setChecked(data.dFlag.grabInvulnerable);
		voidBlock->setChecked(data.dFlag.voidBlock);
		crouch->setChecked(data.dFlag.crouch);
		spiritBlock->setChecked(data.dFlag.spiritBlock);
		matterBlock->setChecked(data.dFlag.matterBlock);
		neutralBlock->setChecked(data.dFlag.neutralBlock);
		airborne->setChecked(data.dFlag.airborne);
		canBlock->setChecked(data.dFlag.canBlock);
		highBlock->setChecked(data.dFlag.highBlock);
		lowBlock->setChecked(data.dFlag.lowBlock);
		dashSpeed->setChecked(data.dFlag.charaCancel);
		resetRotation->setChecked(data.dFlag.resetRotation);
		counterHit->setChecked(data.dFlag.counterHit);
		flash->setChecked(data.dFlag.flash);
		projInvul->setChecked(data.dFlag.projectileInvul);
		proj->setChecked(data.dFlag.projectile);
		lc->setChecked(data.dFlag.landCancel);
		dc->setChecked(data.dFlag.dashCancel);
		resOPS->setChecked(data.dFlag.resetSpeed);
		if (!g)
			*c = false;
	});
}

void	saveCallback(std::unique_ptr<Battle::EditableObject> &object)
{
	if (!object)
		return;
	if (loadedPath.empty())
		loadedPath = Utils::saveFileDialog("Save framedata", "assets", {{".*\\.json", "Frame data file"}});
	if (loadedPath.empty())
		return;

	nlohmann::json j = nlohmann::json::array();

	for (auto &[key, value] : object->_moves) {
		j.push_back({
			{"action", key},
			{"framedata", value}
		});
	}

	std::ofstream stream{loadedPath};

	if (stream.fail()) {
		Utils::dispMsg("Saving failed", loadedPath + ": " + strerror(errno), MB_ICONERROR);
		return;
	}
	stream << j.dump(4) << std::endl;
}

void	saveAsCallback(std::unique_ptr<Battle::EditableObject> &object)
{
	if (!object)
		return;

	auto path = Utils::saveFileDialog("Save framedata", loadedPath.empty() ? "assets" : loadedPath, {{".*\\.json", "Frame data file"}});

	if (path.empty())
		return;
	loadedPath = path;

	nlohmann::json j = nlohmann::json::array();

	for (auto &[key, value] : object->_moves) {
		j.push_back({
			{"action", key},
			{"framedata", value}
		});
	}

	std::ofstream stream{path};

	if (stream.fail()) {
		Utils::dispMsg("Saving failed", path + ": " + strerror(errno), MB_ICONERROR);
		return;
	}
	stream << j.dump(4) << std::endl;
}

void	removeBoxCallback(tgui::Panel::Ptr boxes, std::unique_ptr<Battle::EditableObject> &object, tgui::Panel::Ptr panel)
{
	if (!selectedBox)
		return;

	auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

	if (selectedBox == data.collisionBox) {
		auto block = panel->get<tgui::CheckBox>("Collision");

		block->setChecked(false);
		selectBox(nullptr, nullptr);
		return;
	}
	for (auto it = data.hitBoxes.begin(); it < data.hitBoxes.end(); it++) {
		if (&*it == selectedBox) {
			data.hitBoxes.erase(it);
			refreshBoxes(boxes, data, object);
			selectBox(nullptr, nullptr);
			return;
		}
	}
	for (auto it = data.hurtBoxes.begin(); it < data.hurtBoxes.end(); it++) {
		if (&*it == selectedBox) {
			data.hurtBoxes.erase(it);
			refreshBoxes(boxes, data, object);
			selectBox(nullptr, nullptr);
			return;
		}
	}
	assert(false);
}

void	quitCallback()
{
	quitRequest = true;
}

void	newFileCallback(std::unique_ptr<Battle::EditableObject> &object, tgui::MenuBar::Ptr bar, tgui::Gui &gui)
{
	object = std::make_unique<Battle::EditableObject>();
	loadedPath.clear();
	refreshRightPanel(gui, object);
	bar->setMenuEnabled({"New"}, true);
	bar->setMenuEnabled({"Remove"}, true);
}

void	openFileCallback(std::unique_ptr<Battle::EditableObject> &object, tgui::MenuBar::Ptr bar, tgui::Gui &gui)
{
	auto path = Utils::openFileDialog("Open framedata", "assets", {{".*\\.json", "Frame data file"}});

	if (path.empty())
		return;
	try {
		object.reset();
		object = std::make_unique<Battle::EditableObject>(path);
		loadedPath = path;
		refreshRightPanel(gui, object);
		bar->setMenuEnabled({"New"}, true);
		bar->setMenuEnabled({"Remove"}, true);
	} catch (std::exception &e) {
		Utils::dispMsg("Error", e.what(), MB_ICONERROR);
		loadedPath = path;
		refreshRightPanel(gui, object);
		bar->setMenuEnabled({"New"}, false);
		bar->setMenuEnabled({"Remove"}, false);
	}
}

void	newFrameCallback(tgui::Gui &gui, std::unique_ptr<Battle::EditableObject> &object, tgui::Panel::Ptr panel)
{
	object->_moves.at(object->_action)[object->_actionBlock].insert(object->_moves.at(object->_action)[object->_actionBlock].begin() + object->_animation, object->_moves.at(object->_action)[object->_actionBlock][object->_animation]);

	auto anim = object->_animation + 1;
	auto oldBlock = object->_actionBlock;

	refreshRightPanel(gui, object, false);

	auto block = panel->get<tgui::SpinButton>("Block");
	auto frame = panel->get<tgui::Slider>("Progress");

	block->setValue(oldBlock);
	frame->setValue(anim);
}

void	newEndFrameCallback(tgui::Gui &gui, std::unique_ptr<Battle::EditableObject> &object, tgui::Panel::Ptr panel)
{
	object->_moves.at(object->_action)[object->_actionBlock].push_back(object->_moves.at(object->_action)[object->_actionBlock][object->_animation]);

	auto oldBlock = object->_actionBlock;

	refreshRightPanel(gui, object, false);

	auto block = panel->get<tgui::SpinButton>("Block");
	auto frame = panel->get<tgui::Slider>("Progress");

	block->setValue(oldBlock);
	frame->setValue(object->_moves.at(object->_action)[object->_actionBlock].size() - 1);
}

void	newAnimBlockCallback(tgui::Gui &gui, std::unique_ptr<Battle::EditableObject> &object, tgui::Panel::Ptr panel)
{
	object->_actionBlock = object->_moves.at(object->_action).size();
	object->_moves.at(object->_action).emplace_back();
	object->_moves.at(object->_action).back().emplace_back();
	refreshRightPanel(gui, object, false);

	auto block = panel->get<tgui::SpinButton>("Block");

	block->setValue(object->_moves.at(object->_action).size() - 1);
}

void	newHurtBoxCallback(std::unique_ptr<Battle::EditableObject> &object, tgui::Panel::Ptr boxes)
{
	object->_moves.at(object->_action)[object->_actionBlock][object->_animation].hurtBoxes.push_back({{-10, -10}, {20, 20}});

	auto &box = object->_moves.at(object->_action)[object->_actionBlock][object->_animation].hurtBoxes.back();
	auto button = tgui::Button::create();
	auto renderer = button->getRenderer();

	renderer->setBackgroundColor({0x00, 0xFF, 0x00, 0x4C});
	renderer->setBackgroundColorDown({0x00, 0xFF, 0x00, 0xA0});
	renderer->setBackgroundColorHover({0x00, 0xFF, 0x00, 0x80});
	renderer->setBackgroundColorDisabled({0x00, 0xFF, 0x00, 0x4C});
	renderer->setBackgroundColorFocused({0x00, 0xFF, 0x00, 0xA0});
	renderer->setBorderColor({0x00, 0xFF, 0x00});
	renderer->setBorderColorDown({0x00, 0xFF, 0x00});
	renderer->setBorderColorHover({0x00, 0xFF, 0x00});
	renderer->setBorderColorDisabled({0x00, 0xFF, 0x00});
	renderer->setBorderColorFocused({0x00, 0xFF, 0x00});
	renderer->setBorders(1);
	button->setSize(box.size.x, box.size.y);
	button->setPosition("&.w / 2 + " + std::to_string(box.pos.x), "&.h / 2 + " + std::to_string(box.pos.y + 300));
	button->connect("MousePressed", [&box](std::weak_ptr<tgui::Button> self){
		selectBox(self.lock(), &box);
		canDrag = true;
	}, std::weak_ptr<tgui::Button>(button));
	boxes->add(button, "HurtBox" + std::to_string(object->_moves.at(object->_action)[object->_actionBlock][object->_animation].hurtBoxes.size() - 1));
	refreshBoxes(boxes, object->_moves.at(object->_action)[object->_actionBlock][object->_animation], object);
	selectBox(button, &box);
}

void	newHitBoxCallback(std::unique_ptr<Battle::EditableObject> &object, tgui::Panel::Ptr boxes)
{
	object->_moves.at(object->_action)[object->_actionBlock][object->_animation].hitBoxes.push_back({{-10, -10}, {20, 20}});

	auto &box = object->_moves.at(object->_action)[object->_actionBlock][object->_animation].hitBoxes.back();
	auto button = tgui::Button::create();
	auto renderer = button->getRenderer();

	renderer->setBackgroundColor({0xFF, 0x00, 0x00, 0x4C});
	renderer->setBackgroundColorDown({0xFF, 0x00, 0x00, 0xA0});
	renderer->setBackgroundColorHover({0xFF, 0x00, 0x00, 0x80});
	renderer->setBackgroundColorDisabled({0xFF, 0x00, 0x00, 0x4C});
	renderer->setBackgroundColorFocused({0xFF, 0x00, 0x00, 0xA0});
	renderer->setBorderColor({0xFF, 0x00, 0x00});
	renderer->setBorderColorDown({0xFF, 0x00, 0x00});
	renderer->setBorderColorHover({0xFF, 0x00, 0x00});
	renderer->setBorderColorDisabled({0xFF, 0x00, 0x00});
	renderer->setBorderColorFocused({0xFF, 0x00, 0x00});
	renderer->setBorders(1);
	button->setSize(box.size.x, box.size.y);
	button->setPosition("&.w / 2 + " + std::to_string(box.pos.x), "&.h / 2 + " + std::to_string(box.pos.y + 300));
	button->connect("MousePressed", [&box](std::weak_ptr<tgui::Button> self){
		selectBox(self.lock(), &box);
		canDrag = true;
	}, std::weak_ptr<tgui::Button>(button));
	boxes->add(button, "HitBox" + std::to_string(object->_moves.at(object->_action)[object->_actionBlock][object->_animation].hitBoxes.size() - 1));
	refreshBoxes(boxes, object->_moves.at(object->_action)[object->_actionBlock][object->_animation], object);
	selectBox(button, &box);
}

void	removeFrameCallback(std::unique_ptr<Battle::EditableObject> &object, tgui::Panel::Ptr boxes)
{
	auto &arr = object->_moves.at(object->_action)[object->_actionBlock];

	if (arr.size() == 1) {
		arr.back() = Battle::FrameData();
		refreshBoxes(boxes, arr.back(), object);
		selectBox(nullptr, nullptr);
		return;
	}
	arr.erase(arr.begin() + object->_animation);
	if (object->_animation == arr.size())
		object->_animation--;
	refreshBoxes(boxes, arr[object->_animation], object);
	selectBox(nullptr, nullptr);
}

void	removeAnimationBlockCallback(std::unique_ptr<Battle::EditableObject> &object)
{
	auto &arr = object->_moves.at(object->_action);

	if (arr.size() == 1) {
		arr.back().clear();
		arr.back().emplace_back();
		return;
	}
	arr.erase(arr.begin() + object->_actionBlock);
	if (object->_actionBlock == arr.size())
		object->_actionBlock--;
}

void	removeActionCallback(tgui::Gui &gui, std::unique_ptr<Battle::EditableObject> &object)
{
	if (object->_action == 0)
		return;

	auto it = object->_moves.find(object->_action);

	object->_moves.erase(it);
	refreshRightPanel(gui, object);
}

void	placeGuiHooks(tgui::Gui &gui, std::unique_ptr<Battle::EditableObject> &object)
{
	auto bar = gui.get<tgui::MenuBar>("main_bar");
	auto panel = gui.get<tgui::Panel>("Panel1");
	auto boxes = gui.get<tgui::Panel>("Boxes");

	logger.debug("Placing hooks");
	placeAnimPanelHooks(gui, panel, boxes, object);

	bar->setMenuEnabled({"New"}, false);
	bar->setMenuEnabled({"Remove"}, false);

	bar->connectMenuItem({"File", "Save (Ctrl + S)"}, saveCallback, std::ref(object));
	bar->connectMenuItem({"File", "Save as (Ctrl + Shift + S)"}, saveAsCallback, std::ref(object));
	bar->connectMenuItem({"File", "Quit (Ctrl + Q)"}, quitCallback);
	bar->connectMenuItem({"File", "New framedata (Ctrl + N)"}, newFileCallback, std::ref(object), bar, std::ref(gui));
	bar->connectMenuItem({"File", "Load framedata (Ctrl + O)"}, openFileCallback, std::ref(object), bar, std::ref(gui));

	bar->connectMenuItem({"New", "Frame (Ctrl + F)"}, newFrameCallback, std::ref(gui), std::ref(object), panel);
	bar->connectMenuItem({"New", "End frame (Ctrl + Shift + F)"}, newEndFrameCallback, std::ref(gui), std::ref(object), panel);
	bar->connectMenuItem({"New", "Animation block (Ctrl + B)"}, newAnimBlockCallback, std::ref(gui), std::ref(object), panel);
	bar->connectMenuItem({"New", "Hurt box (Ctrl + H)"}, newHurtBoxCallback, std::ref(object), boxes);
	bar->connectMenuItem({"New", "Hit box (Ctrl + Shift + H)"}, newHitBoxCallback, std::ref(object), boxes);

	bar->connectMenuItem({"Remove", "Frame (Ctrl + Shift + Del)"}, removeFrameCallback, std::ref(object), boxes);
	bar->connectMenuItem({"Remove", "Animation block (Shift + Del)"}, removeAnimationBlockCallback, std::ref(object));
	bar->connectMenuItem({"Remove", "Action (Ctrl + Del)"}, removeActionCallback, std::ref(gui), std::ref(object));
	bar->connectMenuItem({"Remove", "Selected box (Del)"}, removeBoxCallback, boxes, std::ref(object), panel);

	for (unsigned i = 0; i < resizeButtons.size(); i++) {
		auto &resizeButton = resizeButtons[i];

		resizeButton = tgui::Button::create();
		resizeButton->setVisible(false);
		resizeButton->setSize(8, 8);
		resizeButton->getRenderer()->setBackgroundColor(tgui::Color{0xA0, 0xA0, 0xA0});
		resizeButton->getRenderer()->setBorderColor(tgui::Color::Red);
		resizeButton->connect("MousePressed", [i]{
			canDrag = true;
			dragUp = i < 3;
			dragDown = i > 4;
			dragLeft = i == 0 || i == 3 || i == 5;
			dragRight = i == 2 || i == 4 || i == 7;
		});
		gui.add(resizeButton);
	}
}

void	handleDrag(tgui::Gui &gui, std::unique_ptr<Battle::EditableObject> &object, int mouseX, int mouseY)
{
	static bool bbb = false;

	if (!object)
		return;
	if (!selectedBox && !spriteSelected)
		return;
	if (mouseStart.distance(Battle::Vector2i{mouseX, mouseY}) > 10)
		dragStart = true;
	if (!dragStart)
		return;
	if (!dragLeft && !dragRight && !dragUp && !dragDown) {
		if (spriteSelected) {
			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
			Battle::Vector2i diff = Battle::Vector2i{mouseX, mouseY} - lastMouse;

			diff.y *= -1;
			data.offset += diff;
			boxButton->setPosition("&.w / 2 + " + std::to_string(static_cast<int>(data.offset.x - data.size.x / 2)), "&.h / 2 + " + std::to_string(data.offset.y + 300));
			gui.get<tgui::EditBox>("Offset")->setText("(" + std::to_string(data.offset.x) + "," + std::to_string(data.offset.y) + ")");
			arrangeButtons(&*object);
		} else {
			Battle::Vector2i diff{mouseX, mouseY};

			selectedBox->pos += diff - lastMouse;
			boxButton->setPosition("&.w / 2 + " + std::to_string(selectedBox->pos.x), "&.h / 2 + " + std::to_string(selectedBox->pos.y + 300));
			arrangeButtons(&*object);
		}
		lastMouse = {mouseX, mouseY};
		return;
	}
	if (dragLeft) {
		auto diff = mouseX - lastMouse.x;

		if (spriteSelected) {
			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

			if (static_cast<int>(data.size.x) - diff < 10) {
				data.offset.x += data.size.x - 10;
				lastMouse.x += data.size.x - 10;
				data.size.x = 10;
			} else {
				data.offset.x += diff;
				bbb = !bbb;
				bbb &= diff % 2;
				data.size.x -= diff + copysign(bbb, diff);
				lastMouse.x = mouseX;
			}
		} else {
			if (static_cast<int>(selectedBox->size.x) - diff < 10) {
				selectedBox->pos.x += selectedBox->size.x - 10;
				lastMouse.x += selectedBox->size.x - 10;
				selectedBox->size.x = 10;
			} else {
				selectedBox->pos.x += diff;
				selectedBox->size.x -= diff;
				lastMouse.x = mouseX;
			}
		}
	} else if (dragRight) {
		auto diff = mouseX - lastMouse.x;

		if (spriteSelected) {
			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

			if (static_cast<int>(data.size.x) + diff < 10) {
				lastMouse.x -= data.size.x - 10;
				data.offset.x += (data.size.x - 10) / 2;
				data.size.x = 10;
			} else {
				data.size.x += diff;
				bbb = !bbb;
				bbb &= diff % 2;
				data.offset.x += diff / 2 + copysign(bbb, diff);
				lastMouse.x = mouseX;
			}
		} else {
			if (static_cast<int>(selectedBox->size.x) + diff < 10) {
				lastMouse.x -= selectedBox->size.x - 10;
				selectedBox->size.x = 10;
			} else {
				selectedBox->size.x += diff;
				lastMouse.x = mouseX;
			}
		}
	}
	if (dragUp) {
		auto diff = mouseY - lastMouse.y;

		if (spriteSelected) {
			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

			if (static_cast<int>(data.size.y) - diff < 10) {
				lastMouse.y += data.size.y - 10;
				data.size.y = 10;
			} else {
				data.size.y -= diff;
				lastMouse.y = mouseY;
			}
		} else {
			if (static_cast<int>(selectedBox->size.y) - diff < 10) {
				selectedBox->pos.y += selectedBox->size.y - 10;
				lastMouse.y += selectedBox->size.y - 10;
				selectedBox->size.y = 10;
			} else {
				selectedBox->pos.y += diff;
				selectedBox->size.y -= diff;
				lastMouse.y = mouseY;
			}
		}
	} else if (dragDown) {
		auto diff = mouseY - lastMouse.y;

		if (spriteSelected) {
			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

			if (static_cast<int>(data.size.y) + diff < 10) {
				data.offset.y -= data.size.y - 10;
				lastMouse.y -= data.size.y - 10;
				data.size.y = 10;
			} else {
				data.offset.y -= diff;
				data.size.y += diff;
				lastMouse.y = mouseY;
			}
		} else {
			if (static_cast<int>(selectedBox->size.y) + diff < 10) {
				lastMouse.y -= selectedBox->size.y - 10;
				selectedBox->size.y = 10;
			} else {
				selectedBox->size.y += diff;
				lastMouse.y = mouseY;
			}
		}
	}
	if (spriteSelected) {
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		boxButton->setPosition("&.w / 2 + " + std::to_string(static_cast<int>(data.offset.x - data.size.x / 2)), "&.h / 2 + " + std::to_string(data.offset.y + 300));
		boxButton->setSize(data.size.x, data.size.y);
		gui.get<tgui::EditBox>("Offset")->setText("(" + std::to_string(data.offset.x) + "," + std::to_string(data.offset.y) + ")");
		gui.get<tgui::EditBox>("Size")->setText("(" + std::to_string(data.size.x) + "," + std::to_string(data.size.y) + ")");
		arrangeButtons(&*object);
	} else {
		boxButton->setPosition("&.w / 2 + " + std::to_string(selectedBox->pos.x), "&.h / 2 + " + std::to_string(selectedBox->pos.y + 300));
		boxButton->setSize(selectedBox->size.x, selectedBox->size.y);
		arrangeButtons(&*object);
	}
}

void	handleKeyPress(sf::Event::KeyEvent event, std::unique_ptr<Battle::EditableObject> &object, tgui::Gui &gui)
{
	auto bar = gui.get<tgui::MenuBar>("main_bar");
	auto panel = gui.get<tgui::Panel>("Panel1");
	auto boxes = gui.get<tgui::Panel>("Boxes");

	if (event.code == sf::Keyboard::S) {
		if (event.control && event.shift)
			saveAsCallback(object);
		else if (event.control)
			saveCallback(object);
	}
	if (event.code == sf::Keyboard::Q)
		quitCallback();
	if (event.code == sf::Keyboard::N && event.control)
		newFileCallback(object, bar, gui);
	if (event.code == sf::Keyboard::O && event.control)
		openFileCallback(object, bar, gui);

	if (object) {
		if (event.code == sf::Keyboard::F) {
			if (event.control && event.shift)
				newEndFrameCallback(gui, object, panel);
			else if (event.control)
				newFrameCallback(gui, object, panel);
		}
		if (event.code == sf::Keyboard::B && event.control)
			newAnimBlockCallback(gui, object, panel);
		if (event.code == sf::Keyboard::H) {
			if (event.control && event.shift)
				newHitBoxCallback(object, boxes);
			else if (event.control)
				newHurtBoxCallback(object, boxes);
		}

		if (event.code == sf::Keyboard::Delete) {
			if (event.control && event.shift)
				removeFrameCallback(object, boxes);
			else if (event.control)
				removeActionCallback(gui, object);
			else if (event.shift)
				removeAnimationBlockCallback(object);
			else
				removeBoxCallback(boxes, object, panel);
		}
	}
}

void	run()
{
	Battle::game.screen = std::make_unique<Battle::Screen>("Frame data editor");

	std::unique_ptr<Battle::EditableObject> object;
	tgui::Gui gui{*Battle::game.screen};
	sf::Image icon;
	sf::Event event;
	sf::Texture stage;
	sf::Sprite sprite;

	stage.loadFromFile("assets/stages/14687.png");
	sprite.setTexture(stage, true);
	sprite.setPosition({stage.getSize().x * 1.f / -2.f, stage.getSize().y * 1.f / -1.4f});
	sprite.setScale(1, 1);
	if (icon.loadFromFile("assets/editorIcon.png"))
		Battle::game.screen->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	gui.loadWidgetsFromFile("assets/gui/editor.gui");

	bool dragging = false;
	auto panel = gui.get<tgui::Panel>("Panel1");
	auto progress = panel->get<tgui::Slider>("Progress");
	sf::View view;
	sf::View guiView{
		{
			0, 0,
			static_cast<float>(Battle::game.screen->getSize().x),
			static_cast<float>(Battle::game.screen->getSize().y)
		}
	};

	placeGuiHooks(gui, object);
	view.setCenter(panel->getSize().x / 2, -300);
	view.setSize(Battle::game.screen->getSize().x, Battle::game.screen->getSize().y);
	Battle::game.screen->setView(view);
	gui.setView(guiView);
	while (Battle::game.screen->isOpen()) {
		timer++;
		Battle::game.screen->clear(sf::Color::Cyan);
		Battle::game.screen->draw(sprite);
		if (object) {
			if (timer >= updateTimer || updateAnyway) {
				object->update();
				if (object->_animationCtr == 0)
					Battle::game.soundMgr.play(object->_moves.at(object->_action)[object->_actionBlock][object->_animation].soundHandle);
				updateAnyway = false;
				progress->setValue(object->_animation);
				timer -= updateTimer;
			}
			object->render();
		}

		while (Battle::game.screen->pollEvent(event)) {
			gui.handleEvent(event);
			if (event.type == sf::Event::Closed) {
				quitRequest = true;
				continue;
			}
			if (event.type == sf::Event::KeyPressed)
				handleKeyPress(event.key, object, gui);
			if (event.type == sf::Event::Resized) {
				guiView.setSize(event.size.width, event.size.height);
				guiView.setCenter(event.size.width / 2, event.size.height / 2);
				gui.setView(guiView);

				view.setCenter(panel->getSize().x / 2, -300);
				view.setSize(event.size.width, event.size.height);
				Battle::game.screen->setView(view);
				continue;
			}
			dragging &= sf::Mouse::isButtonPressed(sf::Mouse::Left);
			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
				dragging = true;
				dragStart = false;
				mouseStart = {event.mouseButton.x, event.mouseButton.y};
				lastMouse = mouseStart;
			}
			dragUp &= dragging;
			dragDown &= dragging;
			dragLeft &= dragging;
			dragRight &= dragging;
			canDrag &= dragging;
			if (event.type == sf::Event::MouseMoved && dragging && canDrag)
				handleDrag(gui, object, event.mouseMove.x, event.mouseMove.y);
		}
		gui.draw();
		Battle::game.screen->display();
		if (quitRequest) {
			if (!object) {
				Battle::game.screen->close();
				continue;
			}
			quitRequest = false;
			if (gui.get<tgui::ChildWindow>("QuitConfirm"))
				continue;

			auto window = Utils::openWindowWithFocus(gui, 260, 90);

			window->loadWidgetsFromFile("assets/gui/quitConfirm.gui");
			window->get<tgui::Button>("Yes")->connect("Clicked", [&object](std::weak_ptr<tgui::ChildWindow> self){
				if (loadedPath.empty())
					loadedPath = Utils::saveFileDialog("Save framedata", "assets", {{".*\\.json", "Frame data file"}});
				if (loadedPath.empty()) {
					self.lock()->close();
					return;
				}

				nlohmann::json j = nlohmann::json::array();

				for (auto &[key, value] : object->_moves) {
					j.push_back({
						{"action", key},
						{"framedata", value}
					});
				}

				std::ofstream stream{loadedPath};

				if (stream.fail()) {
					Utils::dispMsg("Saving failed", loadedPath + ": " + strerror(errno), MB_ICONERROR);
					return;
				}
				stream << j.dump(4) << std::endl;
				Battle::game.screen->close();
			}, std::weak_ptr<tgui::ChildWindow>(window));
			window->get<tgui::Button>("No")->connect("Clicked", []{
				Battle::game.screen->close();
			});
			window->get<tgui::Button>("Cancel")->connect("Clicked", [](std::weak_ptr<tgui::ChildWindow> self){
				self.lock()->close();
			}, std::weak_ptr<tgui::ChildWindow>(window));
		}
	}
}

int	main()
{
	logger.info("Starting editor.");
	run();
	logger.info("Goodbye !");
	return EXIT_SUCCESS;
}