//
// Created by andgel on 21/01/23
//

#include "VirtualController.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{

	VirtualController::VirtualController()
	{
		this->_stickBack.textureHandle = game->textureMgr.load("assets/icons/inputs/stick_back.png");
		this->_stickTop.textureHandle = game->textureMgr.load("assets/icons/inputs/stick_top.png");
	}

	VirtualController::~VirtualController()
	{
		game->textureMgr.remove(this->_stickBack.textureHandle);
		game->textureMgr.remove(this->_stickTop.textureHandle);
	}

	void VirtualController::consumeEvent(const sf::Event &event)
	{
		RollbackInput::consumeEvent(event);
	}

	void VirtualController::render()
	{

	}
}