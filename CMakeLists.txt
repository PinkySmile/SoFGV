cmake_minimum_required(VERSION 3.13)
project(Battle CXX)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(_USE_MATH_DEFINES=1)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/pkgs)
find_package(SFML REQUIRED)
find_package(TGUI REQUIRED)
if (WIN32)
	find_package(GGPO REQUIRED)
endif()
find_package(clip REQUIRED)
include_directories(include ${SFML_INCLUDE_DIRS} ${TGUI_INCLUDE_DIRS} ${clip_INCLUDE_DIRS})

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

add_executable(
	Battle
	src/main.cpp
	src/Data/Color.hpp
	src/Data/Vector.hpp
	src/Inputs/ControllerInput.cpp
	src/Inputs/ControllerInput.hpp
	src/Inputs/IInput.hpp
	src/Inputs/InputEnum.hpp
	src/Inputs/KeyboardInput.cpp
	src/Inputs/KeyboardInput.hpp
	src/Inputs/RemoteInput.cpp
	src/Inputs/RemoteInput.hpp
	src/Logger.cpp
	src/Logger.hpp
	src/Objects/Character.cpp
	src/Objects/Character.hpp
	src/Objects/Object.cpp
	src/Objects/Object.hpp
	src/Objects/Platform.cpp
	src/Objects/Platform.hpp
	src/Objects/Projectile.cpp
	src/Objects/Projectile.hpp
	src/Objects/FakeObject.cpp
	src/Objects/FakeObject.hpp
	src/Objects/IObject.hpp
	src/Resources/BattleManager.cpp
	src/Resources/BattleManager.hpp
	src/Resources/FrameData.cpp
	src/Resources/FrameData.hpp
	src/Resources/Game.cpp
	src/Resources/Game.hpp
	src/Resources/MoveListData.cpp
	src/Resources/MoveListData.hpp
	src/Resources/NetManager.cpp
	src/Resources/NetManager.hpp
	src/Resources/Screen.cpp
	src/Resources/Screen.hpp
	src/Resources/SoundManager.cpp
	src/Resources/SoundManager.hpp
	src/Resources/Sprite.hpp
	src/Resources/TextureManager.cpp
	src/Resources/TextureManager.hpp
	src/Scenes/CharacterSelect.cpp
	src/Scenes/CharacterSelect.hpp
	src/Scenes/IScene.hpp
	src/Scenes/InGame.cpp
	src/Scenes/InGame.hpp
	src/Scenes/NetplayCharacterSelect.cpp
	src/Scenes/NetplayCharacterSelect.hpp
	src/Scenes/NetplayInGame.cpp
	src/Scenes/NetplayInGame.hpp
	src/Scenes/NetplayScene.hpp
	src/Scenes/TitleScreen.cpp
	src/Scenes/TitleScreen.hpp
	src/Resources/PracticeBattleManager.cpp
	src/Resources/PracticeBattleManager.hpp
	src/Scenes/PracticeInGame.cpp
	src/Scenes/PracticeInGame.hpp
	src/Objects/Characters/Stickman.cpp
	src/Objects/Characters/Stickman.hpp
	src/Utils.cpp
	src/Utils.hpp
	src/Inputs/ReplayInput.cpp src/Inputs/ReplayInput.hpp src/Resources/version.h)

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

if (WIN32)
	target_link_libraries(
		Battle
		dbghelp
		${SFML_GRAPHICS_LIB}
		${SFML_AUDIO_LIB}
		${SFML_SYSTEM_LIB}
		${SFML_WINDOW_LIB}
		${SFML_NETWORK_LIB}
		${GGPO_LIBRARIES}
		${clip_LIBRARIES}
		${TGUI_LIB}
		ws2_32
	)
else ()
	target_link_libraries(
		Battle
		${SFML_GRAPHICS_LIB}
		${SFML_AUDIO_LIB}
		${SFML_SYSTEM_LIB}
		${SFML_WINDOW_LIB}
		${SFML_NETWORK_LIB}
		${clip_LIBRARIES}
		${TGUI_LIB}
		pthread
		png
		xcb
	)
endif ()

add_executable(
	FrameDataEditor
	src/dataEditor.cpp
	src/Data/Color.hpp
	src/Data/Vector.hpp
	src/FrameDataEditor/EditableObject.cpp
	src/FrameDataEditor/EditableObject.hpp
	src/Inputs/ControllerInput.cpp
	src/Inputs/ControllerInput.hpp
	src/Inputs/IInput.hpp
	src/Inputs/InputEnum.hpp
	src/Inputs/KeyboardInput.cpp
	src/Inputs/KeyboardInput.hpp
	src/Inputs/RemoteInput.cpp
	src/Inputs/RemoteInput.hpp
	src/Logger.cpp
	src/Logger.hpp
	src/Objects/Character.cpp
	src/Objects/Character.hpp
	src/Objects/Object.cpp
	src/Objects/Object.hpp
	src/Objects/Platform.cpp
	src/Objects/Platform.hpp
	src/Objects/Projectile.cpp
	src/Objects/Projectile.hpp
	src/Objects/FakeObject.cpp
	src/Objects/FakeObject.hpp
	src/Objects/IObject.hpp
	src/Resources/BattleManager.cpp
	src/Resources/BattleManager.hpp
	src/Resources/FrameData.cpp
	src/Resources/FrameData.hpp
	src/Resources/Game.cpp
	src/Resources/Game.hpp
	src/Resources/MoveListData.cpp
	src/Resources/MoveListData.hpp
	src/Resources/NetManager.cpp
	src/Resources/NetManager.hpp
	src/Resources/Screen.cpp
	src/Resources/Screen.hpp
	src/Resources/SoundManager.cpp
	src/Resources/SoundManager.hpp
	src/Resources/Sprite.hpp
	src/Resources/TextureManager.cpp
	src/Resources/TextureManager.hpp
	src/Scenes/CharacterSelect.cpp
	src/Scenes/CharacterSelect.hpp
	src/Scenes/IScene.hpp
	src/Scenes/InGame.cpp
	src/Scenes/InGame.hpp
	src/Scenes/NetplayCharacterSelect.cpp
	src/Scenes/NetplayCharacterSelect.hpp
	src/Scenes/NetplayInGame.cpp
	src/Scenes/NetplayInGame.hpp
	src/Scenes/NetplayScene.hpp
	src/Scenes/TitleScreen.cpp
	src/Scenes/TitleScreen.hpp
	src/Utils.cpp
	src/Utils.hpp
	src/Resources/PracticeBattleManager.cpp
	src/Resources/PracticeBattleManager.hpp
	src/Scenes/PracticeInGame.cpp
	src/Scenes/PracticeInGame.hpp
	src/Objects/Characters/Stickman.cpp
	src/Objects/Characters/Stickman.hpp
	src/Inputs/ReplayInput.cpp
	src/Inputs/ReplayInput.hpp
	src/Resources/version.h
)

if (WIN32)
	target_link_libraries(
		FrameDataEditor
		${SFML_GRAPHICS_LIB}
		${SFML_SYSTEM_LIB}
		${SFML_WINDOW_LIB}
		${SFML_AUDIO_LIB}
		${SFML_NETWORK_LIB}
		${TGUI_LIB}
		${GGPO_LIBRARIES}
		${clip_LIBRARIES}
		ws2_32
		dbghelp
	)
else()
	target_link_libraries(
		FrameDataEditor
		${SFML_GRAPHICS_LIB}
		${SFML_SYSTEM_LIB}
		${SFML_WINDOW_LIB}
		${SFML_AUDIO_LIB}
		${SFML_NETWORK_LIB}
		${TGUI_LIB}
		${clip_LIBRARIES}
		pthread
		png
		xcb
	)
endif()