#
# A CMake module to find GGPO
#
# Once done this module will define:
#  GGPO_FOUND - system has GGPO
#  GGPO_LIBRARIES - Link these to use GGPO
#

IF (NOT GGPO_LIBRARIES)
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

        FIND_LIBRARY(GGPO_LIBRARIES
                NAMES
                GGPO
                PATHS
                ${GGPO_DIR}/lib/              # GGPO root directory (if provided)
                ${GGPO_DIR}                   # GGPO root directory (if provided)
                /usr/lib64/                   # Default Fedora28 library path
                /usr/lib/                     # Some more Linux library path
                /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
                /usr/local/lib/               # Some more Linux library path
                /usr/local/lib64/             # Some more Linux library path
        )
ENDIF (NOT GGPO_LIBRARIES)

IF (GGPO_LIBRARIES)
        SET(GGPO_FOUND TRUE)
ELSE (GGPO_LIBRARIES)
        SET(GGPO_FOUND FALSE)
ENDIF (GGPO_LIBRARIES)

IF (GGPO_FIND_REQUIRED AND NOT GGPO_FOUND)
        MESSAGE(FATAL_ERROR
                "  GGPO not found.\n"
                "      Fill CMake variable GGPO_DIR to the provided directory.\n"
                )
ENDIF (GGPO_FIND_REQUIRED AND NOT GGPO_FOUND)
