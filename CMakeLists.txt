cmake_minimum_required(VERSION 3.13)
project(Battle)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(
	Battle
	src/main.cpp
	src/Logger.cpp
	include/Logger.hpp
	src/Screen.cpp
	include/Screen.hpp
	include/ICharacter.hpp include/AttackType.hpp include/IHitObject.hpp include/Directions.hpp include/IProjectile.hpp include/ACharacter.hpp include/Point.hpp include/Quadra.hpp include/Action.hpp)

target_link_libraries(
	Battle
	sfml-window
	sfml-system
	sfml-graphics
)