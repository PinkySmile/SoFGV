cmake_minimum_required(VERSION 3.13)
project(SoFGV CXX)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(_USE_MATH_DEFINES=1)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/pkgs)
find_package(SFML REQUIRED)
find_package(TGUI REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src/LibCore ${SFML_INCLUDE_DIRS} ${TGUI_INCLUDE_DIRS})


message("SFML include dir is ${SFML_INCLUDE_DIRS}")
message("SFML graphics library is ${SFML_GRAPHICS_LIBRARY}")
message("SFML audio library is ${SFML_AUDIO_LIBRARY}")
message("SFML system library is ${SFML_SYSTEM_LIBRARY}")
message("SFML window library is ${SFML_WINDOW_LIBRARY}")
message("SFML network library is ${SFML_NETWORK_LIBRARY}")
message("TGUI include dir is ${TGUI_INCLUDE_DIRS}")
message("TGUI library is ${TGUI_LIBRARIES}")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
if (ASAN)
	add_compile_options(-fsanitize=address)
	link_libraries(asan)
endif ()
if (CMAKE_BUILD_TYPE STREQUAL Debug)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-W -Wall -Wextra)
	endif ()
	add_compile_definitions(_DEBUG=1)
else ()
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		if (WIN32)
			add_link_options(-mwindows)
		endif()
	endif ()
	add_compile_definitions(NDEBUG=1)
endif ()

if (WIN32)
	set(GICON src/Game/game.rc)
	set(FDICON src/FrameDataEditor/fdeditor.rc)
else()
	set(GICON)
	set(FDICON)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	if ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
		set(TGUI_LIB "${TGUI_DIR}/lib/tgui-d.lib")
		set(SFML_GRAPHICS_LIB "${SFML_DIR}/lib/sfml-graphics-d.lib")
		set(SFML_AUDIO_LIB "${SFML_DIR}/lib/sfml-audio-d.lib")
		set(SFML_SYSTEM_LIB "${SFML_DIR}/lib/sfml-system-d.lib")
		set(SFML_WINDOW_LIB "${SFML_DIR}/lib/sfml-window-d.lib")
		set(SFML_NETWORK_LIB "${SFML_DIR}/lib/sfml-network-d.lib")
	else ()
		set(TGUI_LIB "${TGUI_LIBRARIES}")
		set(SFML_GRAPHICS_LIB "${SFML_GRAPHICS_LIBRARY}")
		set(SFML_AUDIO_LIB "${SFML_AUDIO_LIBRARY}")
		set(SFML_SYSTEM_LIB "${SFML_SYSTEM_LIBRARY}")
		set(SFML_WINDOW_LIB "${SFML_WINDOW_LIBRARY}")
		set(SFML_NETWORK_LIB "${SFML_NETWORK_LIBRARY}")
	endif ()
else ()
	set(TGUI_LIB "${TGUI_LIBRARIES}")
	set(SFML_GRAPHICS_LIB "${SFML_GRAPHICS_LIBRARY}")
	set(SFML_AUDIO_LIB "${SFML_AUDIO_LIBRARY}")
	set(SFML_SYSTEM_LIB "${SFML_SYSTEM_LIBRARY}")
	set(SFML_WINDOW_LIB "${SFML_WINDOW_LIBRARY}")
	set(SFML_NETWORK_LIB "${SFML_NETWORK_LIBRARY}")
endif ()

add_library(
	SoFGVCore SHARED
	src/LibCore/Data/Color.hpp
	src/LibCore/Data/Vector.hpp
	src/LibCore/Inputs/ControllerInput.cpp
	src/LibCore/Inputs/ControllerInput.hpp
	src/LibCore/Inputs/IInput.hpp
	src/LibCore/Inputs/InputEnum.hpp
	src/LibCore/Inputs/KeyboardInput.cpp
	src/LibCore/Inputs/KeyboardInput.hpp
	src/LibCore/Inputs/RollbackInput.cpp
	src/LibCore/Inputs/RollbackInput.hpp
	src/LibCore/Logger.cpp
	src/LibCore/Logger.hpp
	src/LibCore/Objects/Character.cpp
	src/LibCore/Objects/Character.hpp
	src/LibCore/Objects/Object.cpp
	src/LibCore/Objects/Object.hpp
	src/LibCore/Objects/Platform.cpp
	src/LibCore/Objects/Platform.hpp
	src/LibCore/Objects/Characters/Projectile.cpp
	src/LibCore/Objects/Characters/Projectile.hpp
	src/LibCore/Objects/FakeObject.cpp
	src/LibCore/Objects/FakeObject.hpp
	src/LibCore/Objects/IObject.hpp
	src/LibCore/Resources/Battle/BattleManager.cpp
	src/LibCore/Resources/Battle/BattleManager.hpp
	src/LibCore/Resources/Assets/FrameData.cpp
	src/LibCore/Resources/Assets/FrameData.hpp
	src/LibCore/Resources/Game.cpp
	src/LibCore/Resources/Game.hpp
	src/LibCore/Resources/Assets/MoveListData.cpp
	src/LibCore/Resources/Assets/MoveListData.hpp
	src/LibCore/Resources/Screen.cpp
	src/LibCore/Resources/Screen.hpp
	src/LibCore/Resources/Assets/SoundManager.cpp
	src/LibCore/Resources/Assets/SoundManager.hpp
	src/LibCore/Resources/Assets/Sprite.hpp
	src/LibCore/Resources/Assets/TextureManager.cpp
	src/LibCore/Resources/Assets/TextureManager.hpp
	src/LibCore/Resources/Battle/PracticeBattleManager.cpp
	src/LibCore/Resources/Battle/PracticeBattleManager.hpp
	src/LibCore/Resources/Battle/RandomWrapper.cpp
	src/LibCore/Resources/Battle/RandomWrapper.hpp
	src/LibCore/Objects/Characters/Stickman/Stickman.cpp
	src/LibCore/Objects/Characters/Stickman/Stickman.hpp
	src/LibCore/Utils.cpp
	src/LibCore/Utils.hpp
	src/LibCore/Resources/SceneManager.cpp
	src/LibCore/Resources/SceneManager.hpp
	src/LibCore/Inputs/ReplayInput.cpp
	src/LibCore/Inputs/ReplayInput.hpp
	src/LibCore/Resources/version.h
	src/LibCore/Objects/StageObjects/StageObject.cpp
	src/LibCore/Objects/StageObjects/StageObject.hpp
	src/LibCore/Objects/StageObjects/Cloud.hpp
	src/LibCore/Objects/StageObjects/Cloud.cpp
	src/LibCore/Resources/Network/NetManager.cpp
	src/LibCore/Resources/Network/NetManager.hpp
	src/LibCore/Resources/Network/Packet.cpp
	src/LibCore/Resources/Network/Packet.hpp
	src/LibCore/Resources/Network/Connection.cpp
	src/LibCore/Resources/Network/Connection.hpp
	src/LibCore/Resources/Network/Exceptions.hpp
	src/LibCore/Resources/Network/ServerConnection.cpp
	src/LibCore/Resources/Network/ServerConnection.hpp
	src/LibCore/Resources/Network/ClientConnection.cpp
	src/LibCore/Resources/Network/ClientConnection.hpp
	src/LibCore/Resources/Network/IConnection.hpp
	src/LibCore/Resources/IScene.hpp
	src/LibCore/Resources/Network/RollbackMachine.cpp
	src/LibCore/Resources/Network/RollbackMachine.hpp
	src/LibCore/Inputs/RemoteInput.cpp
	src/LibCore/Inputs/RemoteInput.hpp
	src/LibCore/Inputs/NetworkInput.cpp
	src/LibCore/Inputs/NetworkInput.hpp
	src/LibCore/Inputs/DelayInput.cpp
	src/LibCore/Inputs/DelayInput.hpp
	src/LibCore/Inputs/VirtualController.cpp
	src/LibCore/Inputs/VirtualController.hpp
	src/LibCore/Resources/SceneArgument.hpp
        src/LibCore/Resources/MSVCMacros.hpp
	src/LibCore/Objects/Characters/VictoriaStar/VictoriaStar.cpp
	src/LibCore/Objects/Characters/VictoriaStar/VictoriaStar.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadow.hpp
	src/LibCore/Objects/Characters/SubObject.hpp
	src/LibCore/Objects/Characters/SubObject.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Butterfly.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Butterfly.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/NeutralShadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/NeutralShadow.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/MatterShadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/MatterShadow.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/SpiritShadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/SpiritShadow.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/VoidShadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/VoidShadow.hpp
)
target_compile_definitions(SoFGVCore PRIVATE MYDLL_EXPORTS=1)
if (WIN32)
	enable_language(RC)
	set_target_properties(SoFGVCore PROPERTIES SUFFIX ".dll")
endif ()

if (WIN32)
	target_link_libraries(
		SoFGVCore
		dbghelp
		${SFML_GRAPHICS_LIB}
		${SFML_AUDIO_LIB}
		${SFML_SYSTEM_LIB}
		${SFML_WINDOW_LIB}
		${SFML_NETWORK_LIB}
		${TGUI_LIB}
		ws2_32
	)
else ()
	target_link_libraries(
		SoFGVCore
		${SFML_GRAPHICS_LIB}
		${SFML_AUDIO_LIB}
		${SFML_SYSTEM_LIB}
		${SFML_WINDOW_LIB}
		${SFML_NETWORK_LIB}
		${TGUI_LIB}
		pthread
		png
		xcb
	)
endif ()

message("${GICON}")
add_executable(
	SoFGV
	${GICON}
	src/Game/main.cpp
	src/Game/Scenes/Menu.cpp
	src/Game/Scenes/Menu.hpp
	src/Game/Scenes/MenuItem.cpp
	src/Game/Scenes/MenuItem.hpp
	src/Game/Scenes/LoadingScene.cpp
	src/Game/Scenes/LoadingScene.hpp
	src/Game/Scenes/CharacterSelect.cpp
	src/Game/Scenes/CharacterSelect.hpp
	src/Game/Scenes/InGame.cpp
	src/Game/Scenes/InGame.hpp
	src/Game/Scenes/TitleScreen.cpp
	src/Game/Scenes/TitleScreen.hpp
	src/Game/Scenes/PracticeInGame.cpp
	src/Game/Scenes/PracticeInGame.hpp
	src/Game/Scenes/ReplayInGame.cpp
	src/Game/Scenes/ReplayInGame.hpp
	src/Game/Scenes/Network/SyncTestInGame.cpp
	src/Game/Scenes/Network/SyncTestInGame.hpp
	src/Game/Scenes/Network/ClientCharacterSelect.cpp
	src/Game/Scenes/Network/ClientCharacterSelect.hpp
	src/Game/Scenes/Network/ServerCharacterSelect.cpp
	src/Game/Scenes/Network/ServerCharacterSelect.hpp
	src/Game/Scenes/Network/NetworkCharacterSelect.cpp
	src/Game/Scenes/Network/NetworkCharacterSelect.hpp
	src/Game/Scenes/Network/ClientInGame.cpp
	src/Game/Scenes/Network/ClientInGame.hpp
	src/Game/Scenes/Network/ServerInGame.cpp
	src/Game/Scenes/Network/ServerInGame.hpp
	src/Game/Scenes/Network/NetworkInGame.cpp
	src/Game/Scenes/Network/NetworkInGame.hpp
	src/LibCore/LibCore.hpp
	src/Game/Scenes/Scenes.hpp
)
target_link_libraries(SoFGV SoFGVCore)
target_include_directories(SoFGV PRIVATE src/LibCore src/Game)

message("${FDICON}")
add_executable(
	FrameDataEditor
	${FDICON}
	src/FrameDataEditor/dataEditor.cpp
	src/FrameDataEditor/EditableObject.cpp
	src/FrameDataEditor/EditableObject.hpp
	src/LibCore/LibCore.hpp
)
target_link_libraries(FrameDataEditor SoFGVCore)

add_executable(PaletteExtractor src/PaletteExtractor/main.cpp)
target_link_libraries(
	PaletteExtractor
	${SFML_GRAPHICS_LIB}
	${SFML_SYSTEM_LIB}
	${SFML_WINDOW_LIB}
)

add_executable(LagSwitch src/LagSwitch/main.cpp)
target_link_libraries(
	LagSwitch
	${SFML_SYSTEM_LIB}
	${SFML_NETWORK_LIB}
)