#include <iostream>
#include <memory>
#include <TGUI/TGUI.hpp>
#include "Logger.hpp"
#include "Resources/Screen.hpp"
#include "Resources/Game.hpp"
#include "Objects/ACharacter.hpp"
#include "FrameDataEditor/EditableObject.hpp"
#include "Utils.hpp"

auto c = std::make_shared<bool>(false);
Battle::Logger logger("./editor.log");
float updateTimer = 0;//std::numeric_limits<float>::infinity();
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
tgui::Color normalColor;
tgui::Button::Ptr boxButton;
Battle::Vector2i mouseStart;
Battle::Vector2i lastMouse;
Battle::Box *selectedBox;
Battle::Box startValues;
std::array<tgui::Button::Ptr, 8> resizeButtons;

void	arrangeButtons(Battle::EditableObject *object)
{
	auto *data = object ? &object->_moves.at(object->_action)[object->_actionBlock][object->_animation] : nullptr;
	Battle::Box box = spriteSelected ? Battle::Box{{static_cast<int>(data->offset.x - data->size.x / 2), data->offset.y}, data->size} : *selectedBox;

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
		resizeButton->setPosition("boxes.w / 2 + " + std::to_string(pos.x), "boxes.h / 2 + " + std::to_string(pos.y));
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
	button->setPosition("&.w / 2 + " + std::to_string(static_cast<int>(data.offset.x - data.size.x / 2)), "&.h / 2 + " + std::to_string(data.offset.y));
	button->connect("MousePressed", [&object](std::weak_ptr<tgui::Button> self){
		selectSprite(self.lock(), object);
		canDrag = true;
	}, std::weak_ptr<tgui::Button>(button));
	panel->add(button, "SpriteBox");

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
		button->setPosition("&.w / 2 + " + std::to_string(data.collisionBox->pos.x), "&.h / 2 + " + std::to_string(data.collisionBox->pos.y));
		button->connect("MousePressed", [&data](std::weak_ptr<tgui::Button> self){
			selectBox(self.lock(), data.collisionBox);
			canDrag = true;
		}, std::weak_ptr<tgui::Button>(button));
		panel->add(button, "CollisionBox");
	}
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
		button->setPosition("&.w / 2 + " + std::to_string(box.pos.x), "&.h / 2 + " + std::to_string(box.pos.y));
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
		button->setPosition("&.w / 2 + " + std::to_string(box.pos.x), "&.h / 2 + " + std::to_string(box.pos.y));
		button->connect("MousePressed", [&box](std::weak_ptr<tgui::Button> self){
			selectBox(self.lock(), &box);
			canDrag = true;
		}, std::weak_ptr<tgui::Button>(button));
		panel->add(button, "HitBox" + std::to_string(i));
		i++;
	}
}

void	refreshFrameDataPanel(tgui::Panel::Ptr panel, tgui::Panel::Ptr boxes, std::unique_ptr<Battle::EditableObject> &object)
{
	auto progress = panel->get<tgui::Slider>("Progress");
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
	auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

	logger.debug("Soft refresh");
	*c = true;
	dFlags->setText(std::to_string(data.dFlag.flags));
	oFlags->setText(std::to_string(data.oFlag.flags));
	progress->setMinimum(0);
	progress->setMaximum(object->_moves.at(object->_action)[object->_actionBlock].size() - 1);
	progress->setValue(object->_animation);
	sprite->setText(data.spritePath);
	duration->setText(std::to_string(data.duration));
	marker->setText(std::to_string(data.specialMarker));
	subObj->setText(std::to_string(data.subObjectSpawn));
	pushBack->setText(std::to_string(data.pushBack));
	pushBlock->setText(std::to_string(data.pushBlock));
	blockStun->setText(std::to_string(data.blockStun));
	hitStun->setText(std::to_string(data.hitStun));
	spiritLimit->setText(std::to_string(data.spiritLimit));
	voidLimit->setText(std::to_string(data.voidLimit));
	matterLimit->setText(std::to_string(data.matterLimit));
	prorate->setText(std::to_string(data.prorate));

	auto newBounds = "(" + std::to_string(data.textureBounds.pos.x) + "," + std::to_string(data.textureBounds.pos.y) + "," + std::to_string(data.textureBounds.size.x) + "," + std::to_string(data.textureBounds.size.y) + ")";
	auto newSize = "(" + std::to_string(data.size.x) + "," + std::to_string(data.size.y) + ")";
	auto newOffset = "(" + std::to_string(data.offset.x) + "," + std::to_string(data.offset.y) + ")";

	offset->setText(newOffset);
	bounds->setText(newBounds);
	size->setText(newSize);
	rotation->setValue(data.rotation * 180 / M_PI);
	collisionBox->setChecked(data.collisionBox != nullptr);
	selectBox(nullptr, nullptr);
	refreshBoxes(boxes, data, object);
	*c = false;
}

void	refreshRightPanel(tgui::Gui &gui, std::unique_ptr<Battle::EditableObject> &object)
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
	panel->setEnabled(true);
	step->onPress.emit(&*step, "Step");
	action->setText("0");
	action->onReturnKeyPress.emit(&*action, "0");
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

void	placeAnimPanelHooks(tgui::Panel::Ptr panel, tgui::Panel::Ptr boxes, std::unique_ptr<Battle::EditableObject> &object)
{
	auto panWeak = std::weak_ptr<tgui::Panel>(panel);
	auto animPanel = panel->get<tgui::Panel>("AnimPanel");
	auto action = panel->get<tgui::EditBox>("Action");
	auto block = panel->get<tgui::SpinButton>("Block");
	auto blockLabel = panel->get<tgui::Label>("Label1");
	auto frameLabel = panel->get<tgui::Label>("Label2");
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
	auto flash = panel->get<tgui::CheckBox>("Flash");

	boxes->connect("Clicked", []{
		if (!dragStart && !canDrag)
			selectBox(nullptr, nullptr);
	});
	action->connect("ReturnKeyPressed", [&object, block](const std::string &t){
		if (t.empty())
			return;
		object->_action = std::stoul(t);
		if (object->_moves[object->_action].empty()) {
			object->_moves[object->_action].emplace_back();
			object->_moves[object->_action][0].emplace_back();
		}
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
	offset->connect("TextChanged", [boxes, &object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 1);
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.offset.x = std::stoul(x);
		data.offset.y = std::stoul(y);
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

		data.size.x = std::stoul(x);
		data.size.y = std::stoul(y);
		refreshBoxes(boxes, data, object);
		if (spriteSelected)
			arrangeButtons(&*object);
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

		data.textureBounds.pos.x = std::stoul(x);
		data.textureBounds.pos.y = std::stoul(y);
		data.textureBounds.size.x = std::stoul(w);
		data.textureBounds.size.y = std::stoul(h);
	});
	rotation->connect("ValueChanged", [&object](float f){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.rotation = f / 180 * M_PI;
	});
	duration->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.duration = std::stoul(t);
	});
	marker->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.specialMarker = std::stoul(t);
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

		data.pushBack = std::stoul(t);
	});
	pushBlock->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.pushBlock = std::stoul(t);
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

		data.prorate = std::stoul(t);
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
	oFlags->connect("TextChanged", [&object, grab, aub, ub, voidElem, spiritElem, matterElem, lowHit, highHit, autoHitPos, canCH, hitSwitch, cancelable, jab, resetHit, resetSpeed, restand](std::string t){
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
		data.dFlag.dashSpeed = b;
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
	dFlags->connect("TextChanged", [flash, invulnerable, invulnerableArmor, superArmor, grabInvul, voidBlock, spiritBlock, matterBlock, neutralBlock, airborne, canBlock, highBlock, lowBlock, dashSpeed, resetRotation, counterHit, &object](std::string t){
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
		flash->setChecked(data.dFlag.flash);
		if (!g)
			*c = false;
	});
}

void	placeGuiHooks(tgui::Gui &gui, std::unique_ptr<Battle::EditableObject> &object)
{
	auto bar = gui.get<tgui::MenuBar>("main_bar");
	auto panel = gui.get<tgui::Panel>("Panel1");
	auto boxes = panel->get<tgui::Panel>("Boxes");

	logger.debug("Placing hooks");
	placeAnimPanelHooks(panel, gui.get<tgui::Panel>("Boxes"), object);
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
		refreshRightPanel(gui, object);
	});
	bar->connectMenuItem({"File", "Load framedata"}, [&gui, &object]{
		auto path = Utils::openFileDialog("Open framedata", "assets", {{".*\\.json", "Frame data file"}});

		if (path.empty())
			return;
		try {
			object = std::make_unique<Battle::EditableObject>(path);
			loadedPath = path;
			refreshRightPanel(gui, object);
		} catch (std::exception &e) {
			Utils::dispMsg("Error", e.what(), MB_ICONERROR);
		}
	});

	for (unsigned i = 0; i < resizeButtons.size(); i++) {
		auto &resizeButton = resizeButtons[i];

		resizeButton = tgui::Button::create();
		resizeButton->setVisible(false);
		resizeButton->setSize(8, 8);
		resizeButton->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
		resizeButton->getRenderer()->setBorderColor(tgui::Color::Red);
		resizeButton->connect("MousePressed", [i]{
			startValues = *selectedBox;
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
			Battle::Vector2i diff{mouseX, mouseY};

			data.offset += diff - lastMouse;
			boxButton->setPosition("&.w / 2 + " + std::to_string(static_cast<int>(data.offset.x - data.size.x / 2)), "&.h / 2 + " + std::to_string(data.offset.y));
			gui.get<tgui::EditBox>("Offset")->setText("(" + std::to_string(data.offset.x) + "," + std::to_string(data.offset.y) + ")");
			arrangeButtons(&*object);
		} else {
			Battle::Vector2i diff{mouseX, mouseY};

			selectedBox->pos += diff - lastMouse;
			boxButton->setPosition("&.w / 2 + " + std::to_string(selectedBox->pos.x), "&.h / 2 + " + std::to_string(selectedBox->pos.y));
			arrangeButtons(&*object);
		}
		lastMouse = {mouseX, mouseY};
		return;
	}
}

void	run()
{
	Battle::game.screen = std::make_unique<Battle::Screen>("Frame data editor");

	std::unique_ptr<Battle::EditableObject> object;
	tgui::Gui gui{*Battle::game.screen};
	sf::Image icon;
	sf::Event event;

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
	view.setCenter(panel->getSize().x / 2, 0);
	view.setSize(Battle::game.screen->getSize().x, Battle::game.screen->getSize().y);
	Battle::game.screen->setView(view);
	gui.setView(guiView);
	while (Battle::game.screen->isOpen()) {
		timer++;
		Battle::game.screen->clear(sf::Color::Cyan);
		if (object) {
			if (timer >= updateTimer || updateAnyway) {
				object->update();
				updateAnyway = false;
				progress->setValue(object->_animation);
				timer -= updateTimer;
			}
			object->render();
		}

		while (Battle::game.screen->pollEvent(event)) {
			gui.handleEvent(event);
			if (event.type == sf::Event::Closed)
				Battle::game.screen->close();
			if (event.type == sf::Event::Resized) {
				guiView.setSize(event.size.width, event.size.height);
				guiView.setCenter(event.size.width / 2, event.size.height / 2);
				gui.setView(guiView);

				view.setCenter(panel->getSize().x / 2, 0);
				view.setSize(event.size.width, event.size.height);
				Battle::game.screen->setView(view);
			}
			dragging &= sf::Mouse::isButtonPressed(sf::Mouse::Left);
			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
				dragging = true;
				dragStart = false;
				mouseStart = {event.mouseButton.x, event.mouseButton.y};
				lastMouse = mouseStart;
			}
			canDrag &= dragging;
			if (event.type == sf::Event::MouseMoved && dragging && canDrag)
				handleDrag(gui, object, event.mouseMove.x, event.mouseMove.y);
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