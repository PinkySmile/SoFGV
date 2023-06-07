#
# A CMake module to find TGUI
#
# Once done this module will define:
#  TGUI_FOUND - system has TGUI
#  TGUI_LIBRARIES - Link these to use TGUI
#

IF (NOT TGUI_INCLUDE_DIRS OR NOT TGUI_LIBRARIES)
        IF (BUILD_STATIC)
                SET(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
                SET(CMAKE_FIND_LIBRARY_SUFFIXES "-s.a")
        ELSEIF (MSVC)      # Visual Studio
                SET(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
                SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.dll.lib;.dll.a;.a")
        ELSEIF (MINGW) # Windows
                SET(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
                SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a;.dll;.a;.lib")
        ELSE (MSVC)    # Linux
                SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
                SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so;.dylib;.a")
        ENDIF (BUILD_STATIC)

        FIND_LIBRARY(TGUI_LIBRARIES
                NAMES
                tgui
                PATHS
                ${TGUI_DIR}/lib/              # TGUI root directory (if provided)
                ${TGUI_DIR}                   # TGUI root directory (if provided)
        )

        FIND_PATH(TGUI_INCLUDE_DIRS
                NAMES
                TGUI/TGUI.hpp
                PATHS
                ${TGUI_DIR}/include/             # TGUI root directory (if provided)
                ${TGUI_DIR}                      # TGUI root directory (if provided)
        )
ENDIF (NOT TGUI_INCLUDE_DIRS OR NOT TGUI_LIBRARIES)

IF (TGUI_INCLUDE_DIRS AND TGUI_LIBRARIES)
        SET(TGUI_FOUND TRUE)
ELSE (TGUI_INCLUDE_DIRS AND TGUI_LIBRARIES)
        SET(TGUI_FOUND FALSE)
ENDIF (TGUI_INCLUDE_DIRS AND TGUI_LIBRARIES)

IF (TGUI_FIND_REQUIRED AND NOT TGUI_FOUND)
        MESSAGE(FATAL_ERROR
                "  TGUI not found.\n"
                "      Fill CMake variable TGUI_DIR to the provided directory.\n"
                )
ENDIF (TGUI_FIND_REQUIRED AND NOT TGUI_FOUND)
