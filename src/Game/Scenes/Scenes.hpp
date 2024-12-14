//
// Created by PinkySmile on 11/02/23.
//

#ifndef SOFGV_SCENES_HPP
#define SOFGV_SCENES_HPP

#include "LoadingScene.hpp"
#include "CharacterSelect.hpp"
#include "InGame.hpp"
#include "MainMenu/TitleScreen.hpp"
#include "PracticeInGame.hpp"
#include "ReplayInGame.hpp"
#ifdef HAS_NETWORK
#include "Network/SyncTestInGame.hpp"
#include "Network/ClientCharacterSelect.hpp"
#include "Network/ServerCharacterSelect.hpp"
#include "Network/ClientInGame.hpp"
#include "Network/ServerInGame.hpp"
#endif

#endif //SOFGV_SCENES_HPP
