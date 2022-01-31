cmake_minimum_required(VERSION 3.13)
project(Battle)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(_USE_MATH_DEFINES=1)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/pkgs)
find_package(SFML REQUIRED)
find_package(TGUI REQUIRED)
include_directories(include ${SFML_INCLUDE_DIRS} ${TGUI_INCLUDE_DIRS})

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
	src/Logger.cpp
	src/Logger.hpp
	src/Data/Vector.hpp
	src/Resources/Screen.cpp
	src/Resources/Screen.hpp
	src/Resources/BattleManager.hpp
	src/Resources/BattleManager.cpp
	src/Resources/TextureManager.hpp
	src/Resources/TextureManager.cpp
	src/Inputs/InputEnum.hpp
	src/Objects/ACharacter.hpp
	src/Objects/ACharacter.cpp
	src/Objects/AObject.hpp
	src/Objects/AObject.cpp
	src/Objects/IObject.hpp
	src/Resources/Game.hpp
	src/Resources/Game.cpp
	src/Resources/Sprite.hpp
	src/Data/Color.hpp
	src/Resources/FrameData.cpp
	src/Resources/FrameData.hpp
	src/Scenes/TitleScreen.cpp
	src/Scenes/TitleScreen.hpp
	src/Scenes/IScene.hpp
	src/Scenes/InGame.cpp
	src/Scenes/InGame.hpp
	src/Inputs/IInput.hpp
	src/Inputs/KeyboardInput.cpp
	src/Inputs/KeyboardInput.hpp
	src/Inputs/ControllerInput.cpp
	src/Inputs/ControllerInput.hpp
	src/Inputs/RemoteInput.cpp
	src/Inputs/RemoteInput.hpp
	src/Inputs/NetworkInput.cpp
	src/Inputs/NetworkInput.hpp
	src/Scenes/NetplayGame.cpp
	src/Scenes/NetplayGame.hpp
	src/Scenes/CharacterSelect.cpp
	src/Scenes/CharacterSelect.hpp
)

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

target_link_libraries(
	Battle
	dbghelp
	${SFML_GRAPHICS_LIB}
	${SFML_AUDIO_LIB}
	${SFML_SYSTEM_LIB}
	${SFML_WINDOW_LIB}
	${SFML_NETWORK_LIB}
)

add_executable(
	FrameDataEditor
	src/dataEditor.cpp
	src/Logger.cpp
	src/Logger.hpp
	src/Data/Vector.hpp
	src/Resources/Screen.cpp
	src/Resources/Screen.hpp
	src/Resources/TextureManager.hpp
	src/Resources/TextureManager.cpp
	src/Inputs/InputEnum.hpp
	src/Objects/AObject.hpp
	src/Objects/AObject.cpp
	src/Objects/IObject.hpp
	src/Resources/Game.hpp
	src/Resources/Game.cpp
	src/Resources/Sprite.hpp
	src/Data/Color.hpp
	src/FrameDataEditor/EditableObject.cpp
	src/FrameDataEditor/EditableObject.hpp
	src/Objects/ACharacter.hpp
	src/Objects/ACharacter.cpp
	src/Resources/BattleManager.hpp
	src/Resources/BattleManager.cpp
	src/Resources/FrameData.cpp
	src/Resources/FrameData.hpp
	src/Utils.cpp
	src/Utils.hpp
)

target_link_libraries(
	FrameDataEditor
	${SFML_GRAPHICS_LIB}
	${SFML_SYSTEM_LIB}
	${SFML_WINDOW_LIB}
	${TGUI_LIB}
)