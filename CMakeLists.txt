cmake_minimum_required(VERSION 3.13)
project(Battle)

set(CMAKE_CXX_STANDARD 17)

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
	src/InputEnum.hpp
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
)

target_link_libraries(
	Battle
	${SFML_GRAPHICS_LIBRARY}
	${SFML_AUDIO_LIBRARY}
	${SFML_SYSTEM_LIBRARY}
	${SFML_WINDOW_LIBRARY}
)

add_executable(
	FrameDataEditor
	src/dataEditor.cpp
	src/Logger.cpp
	src/Logger.hpp
	src/Data/Vector.hpp
	src/Resources/Screen.cpp
	src/Resources/Screen.hpp
	src/Resources/BattleManager.hpp
	src/Resources/BattleManager.cpp
	src/Resources/TextureManager.hpp
	src/Resources/TextureManager.cpp
	src/InputEnum.hpp
	src/Objects/ACharacter.hpp
	src/Objects/ACharacter.cpp
	src/Objects/AObject.hpp
	src/Objects/AObject.cpp
	src/Objects/IObject.hpp
	src/Resources/Game.hpp
	src/Resources/Game.cpp
	src/Resources/Sprite.hpp
	src/Data/Color.hpp
	src/FrameDataEditor/EditableObject.cpp
	src/FrameDataEditor/EditableObject.hpp
	src/Resources/FrameData.cpp
	src/Resources/FrameData.hpp
	src/Utils.cpp
	src/Utils.hpp
)

target_link_libraries(
	FrameDataEditor
	${SFML_GRAPHICS_LIBRARY}
	${SFML_SYSTEM_LIBRARY}
	${SFML_WINDOW_LIBRARY}
	${TGUI_LIBRARIES}
)
