cmake_minimum_required(VERSION 3.13)
project(SoFGV CXX)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(_USE_MATH_DEFINES=1)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/pkgs)
find_package(SFML REQUIRED)
find_package(TGUI REQUIRED)
include_directories(include ${SFML_INCLUDE_DIRS} ${TGUI_INCLUDE_DIRS} src/LibCore)

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
	src/LibCore/Inputs/RemoteInput.cpp
	src/LibCore/Inputs/RemoteInput.hpp
	src/LibCore/Logger.cpp
	src/LibCore/Logger.hpp
	src/LibCore/Objects/Character.cpp
	src/LibCore/Objects/Character.hpp
	src/LibCore/Objects/Object.cpp
	src/LibCore/Objects/Object.hpp
	src/LibCore/Objects/Platform.cpp
	src/LibCore/Objects/Platform.hpp
	src/LibCore/Objects/Projectile.cpp
	src/LibCore/Objects/Projectile.hpp
	src/LibCore/Objects/FakeObject.cpp
	src/LibCore/Objects/FakeObject.hpp
	src/LibCore/Objects/IObject.hpp
	src/LibCore/Resources/BattleManager.cpp
	src/LibCore/Resources/BattleManager.hpp
	src/LibCore/Resources/FrameData.cpp
	src/LibCore/Resources/FrameData.hpp
	src/LibCore/Resources/Game.cpp
	src/LibCore/Resources/Game.hpp
	src/LibCore/Resources/MoveListData.cpp
	src/LibCore/Resources/MoveListData.hpp
	src/LibCore/Resources/Screen.cpp
	src/LibCore/Resources/Screen.hpp
	src/LibCore/Resources/SoundManager.cpp
	src/LibCore/Resources/SoundManager.hpp
	src/LibCore/Resources/Sprite.hpp
	src/LibCore/Resources/TextureManager.cpp
	src/LibCore/Resources/TextureManager.hpp
	src/LibCore/Scenes/CharacterSelect.cpp
	src/LibCore/Scenes/CharacterSelect.hpp
	src/LibCore/Scenes/IScene.hpp
	src/LibCore/Scenes/InGame.cpp
	src/LibCore/Scenes/InGame.hpp
	src/LibCore/Scenes/TitleScreen.cpp
	src/LibCore/Scenes/TitleScreen.hpp
	src/LibCore/Resources/PracticeBattleManager.cpp
	src/LibCore/Resources/PracticeBattleManager.hpp
	src/LibCore/Scenes/PracticeInGame.cpp
	src/LibCore/Scenes/PracticeInGame.hpp
	src/LibCore/Objects/Characters/Stickman.cpp
	src/LibCore/Objects/Characters/Stickman.hpp
	src/LibCore/Utils.cpp
	src/LibCore/Utils.hpp
	src/LibCore/Inputs/ReplayInput.cpp
	src/LibCore/Inputs/ReplayInput.hpp
	src/LibCore/Resources/version.h
	src/LibCore/Scenes/ReplayInGame.cpp
	src/LibCore/Scenes/ReplayInGame.hpp
        src/LibCore/Objects/StageObjects/StageObject.cpp
	src/LibCore/Objects/StageObjects/StageObject.hpp
	src/LibCore/Objects/StageObjects/Cloud.hpp
	src/LibCore/Objects/StageObjects/Cloud.cpp
	src/LibCore/Scenes/Network/SyncTestInGame.cpp src/LibCore/Scenes/Network/SyncTestInGame.hpp src/LibCore/Scenes/Network/RollbackMachine.cpp src/LibCore/Scenes/Network/RollbackMachine.hpp)
target_compile_definitions(SoFGVCore PRIVATE MYDLL_EXPORTS=1)

if (WIN32)
	target_link_libraries(
		SoFGVCore
		dbghelp
		${SFML_GRAPHICS_LIB}
		${SFML_AUDIO_LIB}
		${SFML_SYSTEM_LIB}
		${SFML_WINDOW_LIB}
		${TGUI_LIB}
		${SFML_NETWORK_LIB}
		ws2_32
	)
else ()
	target_link_libraries(
		SoFGVCore
		${SFML_GRAPHICS_LIB}
		${SFML_AUDIO_LIB}
		${SFML_SYSTEM_LIB}
		${SFML_WINDOW_LIB}
		${TGUI_LIB}
		pthread
		png
		xcb
		${SFML_NETWORK_LIB}
	)
endif ()

add_executable(
	SoFGV
	${GICON}
	src/Game/main.cpp
	src/LibCore/LibCore.hpp)
target_link_libraries(SoFGV SoFGVCore)
target_include_directories(SoFGV PRIVATE src/LibCore)

add_executable(
	FrameDataEditor
	${FDICON}
	src/FrameDataEditor/dataEditor.cpp
	src/FrameDataEditor/EditableObject.cpp
	src/FrameDataEditor/EditableObject.hpp
	src/LibCore/LibCore.hpp)
target_link_libraries(FrameDataEditor SoFGVCore)

add_executable(PaletteExtractor src/PaletteExtractor/main.cpp)
target_link_libraries(
	PaletteExtractor
	${SFML_GRAPHICS_LIB}
	${SFML_SYSTEM_LIB}
	${SFML_WINDOW_LIB}
)
