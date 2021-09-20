#
# A CMake module to find TGUI
#
# Once done this module will define:
#  TGUI_FOUND - system has TGUI
#  TGUI_LIBRARIES - Link these to use TGUI
#

IF (NOT TGUI_INCLUDE_DIRS OR NOT TGUI_LIBRARIES)
        IF (MSVC)      # Visual Studio
                SET(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
                SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.dll.lib;.dll.a;.a")
        ELSEIF (MINGW) # Windows
                SET(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
                SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a;.dll;.a;.lib")
        ELSE (MSVC)    # Linux
                SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
                SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so;.dylib;.a")
        ENDIF(MSVC)

        FIND_LIBRARY(TGUI_LIBRARIES
                NAMES
                tgui
                PATHS
                ${TGUI_DIR}/lib/              # TGUI root directory (if provided)
                ${TGUI_DIR}                   # TGUI root directory (if provided)
                /usr/lib64/                   # Default Fedora28 library path
                /usr/lib/                     # Some more Linux library path
                /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
                /usr/local/lib/               # Some more Linux library path
                /usr/local/lib64/             # Some more Linux library path
        )

        FIND_PATH(TGUI_INCLUDE_DIRS
                NAMES
                TGUI/TGUI.hpp
                PATHS
                ${TGUI_DIR}/include/             # TGUI root directory (if provided)
                ${TGUI_DIR}                      # TGUI root directory (if provided)
                /usr/include/                    # Default Fedora28 system include path
                /usr/local/include/              # Default Fedora28 local include path
                ${CMAKE_MODULE_PATH}/include/    # Expected to contain the path to this file for Windows10
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
