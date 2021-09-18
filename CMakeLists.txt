cmake_minimum_required(VERSION 3.13)
project(Battle)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

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
	sfml-window
	sfml-system
	sfml-graphics
)
