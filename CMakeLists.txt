cmake_minimum_required(VERSION 3.13)
project(SoFGV CXX)

set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(_USE_MATH_DEFINES ZLIB_CONST)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-W -Wall -Wextra)
	endif ()
	option(DEBUG_CHECKS "Debug checks" ON)
else ()
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		if (WIN32)
			add_link_options(-mwindows)
		endif()
		add_compile_options("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/src=src" "-ffile-prefix-map=${CMAKE_SOURCE_DIR}/src=src")
	endif ()
	option(DEBUG_CHECKS "Debug checks" OFF)
endif ()
if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
	set(BUILD_STATIC ON)
	set(WEB_BUILD ON)
	# Dirty hack for emscripten
	set(SFML_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/SFML")
	set(FLAC_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/FLAC")
	set(Freetype_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/freetype")
	set(Ogg_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Ogg")
	set(Vorbis_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Vorbis")
	set(TGUI_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/TGUI")

	# Suppress warning in json library
	set(SOFGV_EMSCRIPTEN_TARGET_COMPILE_OPTIONS_DEBUG -g3 -gsource-map -Og)
	set(SOFGV_EMSCRIPTEN_TARGET_COMPILE_OPTIONS_RELEASE -O3)
	set(SOFGV_EMSCRIPTEN_TARGET_LINK_OPTIONS_DEBUG -g3 -gsource-map -sASSERTIONS=2 -sCHECK_NULL_WRITES=1 -sSTACK_OVERFLOW_CHECK=1 -sWEBSOCKET_DEBUG) # -sSAFE_HEAP=1
	set(SOFGV_EMSCRIPTEN_TARGET_LINK_OPTIONS_RELEASE -O3)
	set(SOFGV_EMSCRIPTEN_TARGET_LINK_OPTIONS_ASYNCIFY -sASYNCIFY=1) #-sASYNCIFY_IGNORE_INDIRECT=1)

	if (CMAKE_BUILD_TYPE STREQUAL Debug)
		set(SOFGV_EMSCRIPTEN_TARGET_COMPILE_OPTIONS ${SOFGV_EMSCRIPTEN_TARGET_COMPILE_OPTIONS_DEBUG})
		set(SOFGV_EMSCRIPTEN_TARGET_LINK_OPTIONS ${SOFGV_EMSCRIPTEN_TARGET_LINK_OPTIONS_DEBUG})
	else()
		set(SOFGV_EMSCRIPTEN_TARGET_COMPILE_OPTIONS ${SOFGV_EMSCRIPTEN_TARGET_COMPILE_OPTIONS_RELEASE})
		set(SOFGV_EMSCRIPTEN_TARGET_LINK_OPTIONS ${SOFGV_EMSCRIPTEN_TARGET_LINK_OPTIONS_RELEASE})
	endif()

	set(SOFGV_EMSCRIPTEN_TARGET_COMPILE_OPTIONS
		${SOFGV_EMSCRIPTEN_TARGET_COMPILE_OPTIONS}
		#-pthread
		--use-port=freetype
		-fwasm-exceptions
	)
	set(SOFGV_EMSCRIPTEN_TARGET_LINK_OPTIONS
		${SOFGV_EMSCRIPTEN_TARGET_LINK_OPTIONS}

		${SOFGV_EMSCRIPTEN_TARGET_LINK_OPTIONS_ASYNCIFY}

		#-pthread                            # Enable threading support

		-fwasm-exceptions
		-Wno-limited-postlink-optimizations # warning: running limited binaryen optimizations because DWARF info requested (or indirectly required)
		-Wno-pthreads-mem-growth            # warning: -pthread + ALLOW_MEMORY_GROWTH may run non-wasm code slowly, see https://github.com/WebAssembly/design/issues/1271

		-sNO_EXIT_RUNTIME=1
		-sEXPORTED_RUNTIME_METHODS=HEAPF32
		-sALLOW_MEMORY_GROWTH=1             # Grow the memory arrays at runtime
		-sFETCH=1                           # Enables `emscripten_fetch` API
		-sFORCE_FILESYSTEM=1                # Makes full filesystem support be included
		-sFULL_ES2=1                        # Forces support for all GLES2 features, not just the WebGL-friendly subset
		-sMAX_WEBGL_VERSION=1               # Specifies the highest WebGL version to target
		-sMIN_WEBGL_VERSION=1               # Specifies the lowest WebGL version to target
		-sSTACK_SIZE=8mb                    # Set the total stack size
		#-sUSE_PTHREADS=1                    # Enable threading support
		-sWASM=1                            # Compile code to WebAssembly

		-sGL_EXPLICIT_UNIFORM_LOCATION=1  # TODO P0:
		-sGL_EXPLICIT_UNIFORM_BINDING=1   # TODO P0:

		--bind
		--embed-file ${CMAKE_SOURCE_DIR}/assets/@/assets
		-sTOTAL_MEMORY=512mb

		--shell-file=${CMAKE_SOURCE_DIR}/shell.html
	)

	set(CMAKE_EXECUTABLE_SUFFIX ".html")
	add_compile_options(-Wno-deprecated-literal-operator)
	add_compile_options(${SOFGV_EMSCRIPTEN_TARGET_COMPILE_OPTIONS})
	add_link_options(${SOFGV_EMSCRIPTEN_TARGET_LINK_OPTIONS})
else ()
	set(WEB_BUILD OFF)
endif ()
if (USE_SDL2)
	find_package(SDL2 REQUIRED)

	message("SDL2 include dir is ${SDL2_INCLUDE_DIRS}")
	message("SDL2 library is ${SDL2_LIBRARY}")
	message("SDL2 image library is ${SDL2_IMAGE_LIBRARY}")

	set(EXTRA_LIBRARY_INCLUDES ${SDL2_INCLUDE_DIRS} src/LibCore/SDL2)
	set(SPECIFIC_INCLUDES_PATH src/LibCore/SDL2)
	set(
		INPUT_SRCS
		src/LibCore/SDL2/ControllerInput.cpp
		src/LibCore/SDL2/ControllerInput.hpp
		src/LibCore/SDL2/KeyboardInput.cpp
		src/LibCore/SDL2/KeyboardInput.hpp
	)
	set(
		ASSETS_SRCS
		src/LibCore/SDL2/SoundManager.cpp
		src/LibCore/SDL2/SoundManager.hpp
		src/LibCore/SDL2/TextureManager.cpp
		src/LibCore/SDL2/TextureManager.hpp
	)
	set(
		SCREEN_SRCS
		src/LibCore/SDL2/Color.hpp
		src/LibCore/SDL2/Sprite.hpp
		src/LibCore/SDL2/Screen.cpp
		src/LibCore/SDL2/Screen.hpp
	)

	find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio Network)
	find_package(TGUI 1 REQUIRED)

	set(
		EXTRA_LINK_LIBRARIES
		${SDL2_LIBRARY}
		${SDL2_IMAGE_LIBRARY}
		SFML::Graphics
		SFML::Audio
		SFML::Window
		SFML::System
		SFML::Network
		TGUI::TGUI
	)

	set(NETWORK_CLASSES)
	set(NETWORK_EXTRA_SRC)
	set(VIRTUAL_CONTROLLER OFF)
	add_compile_definitions(USE_SDL=1)
else ()
	if (EMSCRIPTEN)
		set(NETWORK_CLASSES IP_CLASS=sf::IpAddress SOCKET_CLASS=EmWsSocket)
		set(NETWORK_EXTRA_SRC src/LibCore/Resources/Network/EmWsSocket.cpp src/LibCore/Resources/Network/EmWsSocket.hpp)
		if (DEFINED WEBSOCKET_PATH)
			add_compile_definitions(WEBSOCKET_PATH="${WEBSOCKET_PATH}")
		else ()
			add_compile_definitions(WEBSOCKET_PATH="/")
		endif ()
	else ()
		set(NETWORK_CLASSES IP_CLASS=sf::IpAddress SOCKET_CLASS=sf::UdpSocket)
		set(NETWORK_EXTRA_SRC)
	endif ()
	set(
		INPUT_SRCS
		src/LibCore/SFML/ControllerInput.cpp
		src/LibCore/SFML/ControllerInput.hpp
		src/LibCore/SFML/KeyboardInput.cpp
		src/LibCore/SFML/KeyboardInput.hpp
		src/LibCore/SFML/VirtualController.cpp
		src/LibCore/SFML/VirtualController.hpp
	)
	set(
		ASSETS_SRCS
		src/LibCore/SFML/SoundManager.cpp
		src/LibCore/SFML/SoundManager.hpp
		src/LibCore/SFML/TextureManager.cpp
		src/LibCore/SFML/TextureManager.hpp
	)
	set(
		SCREEN_SRCS
		src/LibCore/SFML/Color.hpp
		src/LibCore/SFML/Color.cpp
		src/LibCore/SFML/Sprite.hpp
		src/LibCore/SFML/Screen.cpp
		src/LibCore/SFML/Screen.hpp
		src/LibCore/SFML/Sprite.cpp
	)

	if (BUILD_STATIC)
		set(TGUI_SHARED_LIBS OFF)
		set(SFML_STATIC_LIBRARIES ON)
	else()
		set(TGUI_SHARED_LIBS ON)
		set(SFML_STATIC_LIBRARIES OFF)
	endif()
	find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio Network)
	find_package(TGUI 1 REQUIRED)

	if (NO_TGUI)
		set(
			EXTRA_LINK_LIBRARIES
			SFML::Graphics
			SFML::Audio
			SFML::Network
		)
	else ()
		set(
			EXTRA_LINK_LIBRARIES
			SFML::Graphics
			SFML::Audio
			SFML::Network
			TGUI::TGUI
		)
		add_compile_definitions(USE_TGUI=1)
	endif()
	add_compile_definitions(USE_SFML=1)
	set(EXTRA_LIBRARY_INCLUDES src/LibCore/SFML)
endif ()
if (EMSCRIPTEN)
	set(
		EXTRA_LINK_LIBRARIES
		${EXTRA_LINK_LIBRARIES}
		idbfs.js
		websocket.js
	)
endif ()

if (VIRTUAL_CONTROLLER)
	add_compile_definitions(VIRTUAL_CONTROLLER=1)
endif ()
if (NETWORK_CLASSES)
	find_library(ZLIB_LIBRARY z NAMES zlib z REQUIRED)
	message("zlib library is ${ZLIB_LIBRARY}")
	set(
		EXTRA_LINK_LIBRARIES
		${EXTRA_LINK_LIBRARIES}
		${ZLIB_LIBRARY}
	)
	set(
		NETWORK_SRCS
		${NETWORK_EXTRA_SRC}
		src/LibCore/Inputs/NetworkInput.cpp
		src/LibCore/Inputs/NetworkInput.hpp
		src/LibCore/Resources/Network/ClientConnection.cpp
		src/LibCore/Resources/Network/ClientConnection.hpp
		src/LibCore/Resources/Network/SpectatorConnection.cpp
		src/LibCore/Resources/Network/SpectatorConnection.hpp
		src/LibCore/Resources/Network/Connection.cpp
		src/LibCore/Resources/Network/Connection.hpp
		src/LibCore/Resources/Network/Exceptions.hpp
		src/LibCore/Resources/Network/IConnection.hpp
		src/LibCore/Resources/Network/NetManager.cpp
		src/LibCore/Resources/Network/NetManager.hpp
		src/LibCore/Resources/Network/Packet.cpp
		src/LibCore/Resources/Network/Packet.hpp
		src/LibCore/Resources/Network/RollbackMachine.cpp
		src/LibCore/Resources/Network/RollbackMachine.hpp
		src/LibCore/Resources/Network/ServerConnection.cpp
		src/LibCore/Resources/Network/ServerConnection.hpp
		src/LibCore/Resources/Network/SpectatorInputManager.cpp
		src/LibCore/Resources/Network/SpectatorInputManager.hpp
		src/LibCore/Inputs/SpectatorInput.cpp
		src/LibCore/Inputs/SpectatorInput.hpp
	)
	set(
		NETWORK_EXTRA_SCENES
		src/Game/Scenes/Network/SyncTestInGame.cpp
		src/Game/Scenes/Network/SyncTestInGame.hpp
		src/Game/Scenes/Network/ClientCharacterSelect.cpp
		src/Game/Scenes/Network/ClientCharacterSelect.hpp
		src/Game/Scenes/Network/ServerCharacterSelect.cpp
		src/Game/Scenes/Network/ServerCharacterSelect.hpp
		src/Game/Scenes/Network/NetworkCharacterSelect.cpp
		src/Game/Scenes/Network/NetworkCharacterSelect.hpp
		src/Game/Scenes/Network/SpectatorCharacterSelect.cpp
		src/Game/Scenes/Network/SpectatorCharacterSelect.hpp
		src/Game/Scenes/Network/ClientInGame.cpp
		src/Game/Scenes/Network/ClientInGame.hpp
		src/Game/Scenes/Network/ServerInGame.cpp
		src/Game/Scenes/Network/ServerInGame.hpp
		src/Game/Scenes/Network/NetworkInGame.cpp
		src/Game/Scenes/Network/NetworkInGame.hpp
		src/Game/Scenes/Network/SpectatorInGame.cpp
		src/Game/Scenes/Network/SpectatorInGame.hpp
	)
	add_compile_definitions(HAS_NETWORK=1)
	add_compile_definitions(${NETWORK_CLASSES})
endif ()

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src/LibCore ${EXTRA_LIBRARY_INCLUDES})
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
if (ASAN)
	add_compile_options(-fsanitize=address)
	add_link_options(-fsanitize=address)
endif ()
if (TSAN)
	add_compile_options(-fsanitize=thread)
	add_link_options(-fsanitize=thread)
endif ()
if (MSAN)
	add_compile_options(-fsanitize=memory)
	add_link_options(-fsanitize=memory)
endif ()
if (UBSAN)
	add_compile_options(-fsanitize=undefined)
	add_link_options(-fsanitize=undefined)
endif ()

if (DEBUG_CHECKS)
	add_compile_definitions(_DEBUG=1)
else ()
	add_compile_definitions(NDEBUG=1)
endif ()
if (WIN32)
	set(CRES src/LibCore/version.rc)
	set(GICON src/Game/game.rc src/Game/version.rc)
	set(FDICON src/FrameDataEditor/fdeditor.rc src/FrameDataEditor/version.rc)
else()
	set(CRES)
	set(GICON)
	set(FDICON)
endif()

if (BUILD_STATIC)
	set(LIBRARY_TYPE STATIC)
else ()
	set(LIBRARY_TYPE SHARED)
endif ()

add_library(
	SoFGVCore ${LIBRARY_TYPE}
	${CRES}
	src/LibCore/Data/Vector.hpp
	src/LibCore/Inputs/DelayInput.cpp
	src/LibCore/Inputs/DelayInput.hpp
	src/LibCore/Inputs/IInput.hpp
	src/LibCore/Inputs/InputEnum.hpp
	src/LibCore/Inputs/RemoteInput.cpp
	src/LibCore/Inputs/RemoteInput.hpp
	src/LibCore/Inputs/ReplayInput.cpp
	src/LibCore/Inputs/ReplayInput.hpp
	src/LibCore/Inputs/RollbackInput.cpp
	src/LibCore/Inputs/RollbackInput.hpp
	src/LibCore/Logger.cpp
	src/LibCore/Logger.hpp
	src/LibCore/Objects/Characters/Character.cpp
	src/LibCore/Objects/Characters/Character.hpp
	src/LibCore/Objects/Characters/Projectile.cpp
	src/LibCore/Objects/Characters/Projectile.hpp
	src/LibCore/Objects/Characters/Stickman/Stickman.cpp
	src/LibCore/Objects/Characters/Stickman/Stickman.hpp
	src/LibCore/Objects/Characters/SubObject.cpp
	src/LibCore/Objects/Characters/SubObject.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Butterfly.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Butterfly.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Flower.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Flower.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadow.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/MatterShadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/MatterShadow.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/NeutralShadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/NeutralShadow.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/SpiritShadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/SpiritShadow.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/VoidShadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/VoidShadow.hpp
	src/LibCore/Objects/Characters/VictoriaStar/VictoriaProjectile.cpp
	src/LibCore/Objects/Characters/VictoriaStar/VictoriaProjectile.hpp
	src/LibCore/Objects/Characters/VictoriaStar/VictoriaStar.cpp
	src/LibCore/Objects/Characters/VictoriaStar/VictoriaStar.hpp
	src/LibCore/Objects/FakeObject.cpp
	src/LibCore/Objects/FakeObject.hpp
	src/LibCore/Objects/IObject.hpp
	src/LibCore/Objects/Object.cpp
	src/LibCore/Objects/Object.hpp
	src/LibCore/Objects/Platform.cpp
	src/LibCore/Objects/Platform.hpp
	src/LibCore/Objects/StageObjects/Cloud.cpp
	src/LibCore/Objects/StageObjects/Cloud.hpp
	src/LibCore/Objects/StageObjects/StageObject.cpp
	src/LibCore/Objects/StageObjects/StageObject.hpp
	src/LibCore/Resources/Assets/FileManager.cpp
	src/LibCore/Resources/Assets/FileManager.hpp
	src/LibCore/Resources/Assets/FrameData.cpp
	src/LibCore/Resources/Assets/FrameData.hpp
	src/LibCore/Resources/Assets/MoveListData.cpp
	src/LibCore/Resources/Assets/MoveListData.hpp
	src/LibCore/Resources/Battle/BattleManager.cpp
	src/LibCore/Resources/Battle/BattleManager.hpp
	src/LibCore/Resources/Battle/PracticeBattleManager.cpp
	src/LibCore/Resources/Battle/PracticeBattleManager.hpp
	src/LibCore/Resources/Battle/RandomWrapper.cpp
	src/LibCore/Resources/Battle/RandomWrapper.hpp
	src/LibCore/Resources/Game.cpp
	src/LibCore/Resources/Game.hpp
	src/LibCore/Resources/IScene.hpp
	src/LibCore/Resources/MSVCMacros.hpp
	src/LibCore/Resources/SceneArgument.hpp
	src/LibCore/Resources/SceneManager.cpp
	src/LibCore/Resources/SceneManager.hpp
	src/LibCore/Resources/version.h
	src/LibCore/Utils.cpp
	src/LibCore/Utils.hpp
	src/LibCore/Objects/Particle.cpp
	src/LibCore/Objects/Particle.hpp
	src/LibCore/Objects/ParticleGenerator.cpp
	src/LibCore/Objects/ParticleGenerator.hpp
	src/LibCore/Resources/ReplayData.hpp
	src/LibCore/Objects/Characters/CharacterParams.hpp
	src/LibCore/Objects/CheckUtils.hpp
	src/LibCore/Resources/ObjectFactory.cpp
	src/LibCore/Resources/ObjectFactory.hpp
	src/LibCore/Resources/Assert.hpp
	src/LibCore/Resources/Assert.cpp
	src/LibCore/Resources/Settings.cpp
	src/LibCore/Resources/Settings.hpp
	${INPUT_SRCS}
	${ASSETS_SRCS}
	${SCREEN_SRCS}
	${NETWORK_SRCS}
)
target_compile_definitions(SoFGVCore PRIVATE MYDLL_EXPORTS=1)
if (WIN32)
	enable_language(RC)
	set_target_properties(SoFGVCore PROPERTIES SUFFIX ".dll")
endif ()

if (WIN32)
	target_link_libraries(
		SoFGVCore
		${EXTRA_LINK_LIBRARIES}
		dbghelp
		ws2_32
	)
else ()
	target_link_libraries(
		SoFGVCore
		${EXTRA_LINK_LIBRARIES}
		pthread
	)
endif ()

add_executable(
	SoFGV_bench
	${GICON}
	src/LibCore/benchmark_state_ser.cpp
)
target_link_libraries(SoFGV_bench SoFGVCore ${EXTRA_LINK_LIBRARIES})

message("${GICON}")
add_executable(
	SoFGV
	${GICON}
	src/Game/main.cpp
	src/Game/Scenes/MainMenu/Menu.cpp
	src/Game/Scenes/MainMenu/Menu.hpp
	src/Game/Scenes/MainMenu/MenuItem.cpp
	src/Game/Scenes/MainMenu/MenuItem.hpp
	src/Game/Scenes/MainMenu/TitleScreen.cpp
	src/Game/Scenes/MainMenu/TitleScreen.hpp
	src/Game/Scenes/LoadingScene.cpp
	src/Game/Scenes/LoadingScene.hpp
	src/Game/Scenes/CharacterSelect.cpp
	src/Game/Scenes/CharacterSelect.hpp
	src/Game/Scenes/InGame.cpp
	src/Game/Scenes/InGame.hpp
	src/Game/Scenes/PracticeInGame.cpp
	src/Game/Scenes/PracticeInGame.hpp
	src/Game/Scenes/ReplayInGame.cpp
	src/Game/Scenes/ReplayInGame.hpp
	${NETWORK_EXTRA_SCENES}
	src/LibCore/LibCore.hpp
	src/Game/Scenes/Scenes.hpp
	src/Game/Scenes/SkeletonData.hpp
)
target_link_libraries(SoFGV SoFGVCore pthread ${EXTRA_LINK_LIBRARIES})
target_include_directories(SoFGV PRIVATE src/LibCore src/Game)

if (NOT USE_SDL2)
	if (NOT NO_TGUI)
		message("${FDICON}")
		add_executable(
			FrameDataEditor
			${FDICON}
			src/LibCore/LibCore.hpp
			src/FrameDataEditor/dataEditor.cpp
			src/FrameDataEditor/EditableObject.cpp
			src/FrameDataEditor/EditableObject.hpp
			src/FrameDataEditor/FrameDataEditor.cpp
			src/FrameDataEditor/FrameDataEditor.hpp
			src/FrameDataEditor/PngLoader.cpp
			src/FrameDataEditor/PngLoader.hpp

			src/FrameDataEditor/UI/ColorPlaneWidget.cpp
			src/FrameDataEditor/UI/ColorPlaneWidget.hpp
			src/FrameDataEditor/UI/ColorSliderWidget.cpp
			src/FrameDataEditor/UI/ColorSliderWidget.hpp
			src/FrameDataEditor/UI/ColorWidgets.hpp
			src/FrameDataEditor/UI/LocalizedContainer.hpp
			src/FrameDataEditor/UI/MainWindow.cpp
			src/FrameDataEditor/UI/MainWindow.hpp
			src/FrameDataEditor/UI/PreviewWidget.cpp
			src/FrameDataEditor/UI/PreviewWidget.hpp
			src/FrameDataEditor/UI/SettingsWindow.cpp
			src/FrameDataEditor/UI/SettingsWindow.hpp

			src/FrameDataEditor/Operations/Operation.hpp
			src/FrameDataEditor/Operations/DummyOperation.cpp
			src/FrameDataEditor/Operations/DummyOperation.hpp
			src/FrameDataEditor/Operations/FlagOperation.hpp
			src/FrameDataEditor/Operations/BasicDataOperation.hpp
			src/FrameDataEditor/Operations/EditBoxOperation.cpp
			src/FrameDataEditor/Operations/EditBoxOperation.hpp
			src/FrameDataEditor/Operations/ClearBlockOperation.cpp
			src/FrameDataEditor/Operations/ClearBlockOperation.hpp
			src/FrameDataEditor/Operations/ClearHitOperation.cpp
			src/FrameDataEditor/Operations/ClearHitOperation.hpp
			src/FrameDataEditor/Operations/EditColorOperation.cpp
			src/FrameDataEditor/Operations/EditColorOperation.hpp
			src/FrameDataEditor/Operations/CreateBlockOperation.cpp
			src/FrameDataEditor/Operations/CreateBlockOperation.hpp
			src/FrameDataEditor/Operations/CreateBoxOperation.cpp
			src/FrameDataEditor/Operations/CreateBoxOperation.hpp
			src/FrameDataEditor/Operations/CreateFrameOperation.cpp
			src/FrameDataEditor/Operations/CreateFrameOperation.hpp
			src/FrameDataEditor/Operations/CreateMoveOperation.cpp
			src/FrameDataEditor/Operations/CreateMoveOperation.hpp
			src/FrameDataEditor/Operations/CreatePaletteOperation.cpp
			src/FrameDataEditor/Operations/CreatePaletteOperation.hpp
			src/FrameDataEditor/Operations/PasteAnimDataOperation.cpp
			src/FrameDataEditor/Operations/PasteAnimDataOperation.hpp
			src/FrameDataEditor/Operations/PasteBoxDataOperation.cpp
			src/FrameDataEditor/Operations/PasteBoxDataOperation.hpp
			src/FrameDataEditor/Operations/PasteDataOperation.cpp
			src/FrameDataEditor/Operations/PasteDataOperation.hpp
			src/FrameDataEditor/Operations/RemoveBoxOperation.cpp
			src/FrameDataEditor/Operations/RemoveBoxOperation.hpp
			src/FrameDataEditor/Operations/RemoveFrameOperation.cpp
			src/FrameDataEditor/Operations/RemoveFrameOperation.hpp
			src/FrameDataEditor/Operations/RemovePaletteOperation.cpp
			src/FrameDataEditor/Operations/RemovePaletteOperation.hpp
			src/FrameDataEditor/Operations/EditSoundOperation.cpp
			src/FrameDataEditor/Operations/EditSoundOperation.hpp
			src/FrameDataEditor/Operations/EditSpriteOperation.cpp
			src/FrameDataEditor/Operations/EditSpriteOperation.hpp
			src/FrameDataEditor/Operations/RemoveBlockOperation.cpp
			src/FrameDataEditor/Operations/RemoveBlockOperation.hpp
			src/FrameDataEditor/Operations/RemoveMoveOperation.cpp
			src/FrameDataEditor/Operations/RemoveMoveOperation.hpp
			src/FrameDataEditor/UI/ShortcutsWindow.cpp
			src/FrameDataEditor/UI/ShortcutsWindow.hpp
			src/FrameDataEditor/Operations/EditColorsOperation.cpp
			src/FrameDataEditor/Operations/EditColorsOperation.hpp
			src/FrameDataEditor/Operations/FlattenCollisionBoxesOperation.cpp
			src/FrameDataEditor/Operations/FlattenCollisionBoxesOperation.hpp
			src/FrameDataEditor/Operations/FlattenPropretiesOperation.cpp
			src/FrameDataEditor/Operations/FlattenPropretiesOperation.hpp
			src/FrameDataEditor/Operations/CleanPropertiesOperation.cpp
			src/FrameDataEditor/Operations/CleanPropertiesOperation.hpp
		)
		target_link_libraries(FrameDataEditor SoFGVCore png)
	endif ()

	add_executable(FrameComparer src/FrameComparer/main.cpp)
	target_link_libraries(
		FrameComparer
		SoFGVCore
	)

	add_executable(PaletteExtractor src/PaletteExtractor/main.cpp)
	target_link_libraries(
		PaletteExtractor
		SFML::Graphics
	)

	add_executable(LagSwitch src/LagSwitch/main.cpp)
	target_link_libraries(
		LagSwitch
		SFML::Network
	)
	target_include_directories(LagSwitch PRIVATE src/LibCore)
endif ()