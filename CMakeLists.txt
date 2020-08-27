cmake_minimum_required(VERSION 3.13)
project(Battle)

set(CMAKE_CXX_STANDARD 17)

add_executable(
	Battle
	src/main.cpp
	src/Logger.cpp
	src/Logger.hpp
	src/Screen.cpp
	src/Screen.hpp
	src/ICharacter.hpp
	src/AttackType.hpp
	src/IHitObject.hpp
	src/Directions.hpp
	src/IProjectile.hpp
	src/ACharacter.hpp
	src/Point.hpp
	src/Quadra.hpp
#	src/Action.hpp
	src/ACharacter.cpp
)

target_link_libraries(
	Battle
	sfml-window
	sfml-system
	sfml-graphics
)
