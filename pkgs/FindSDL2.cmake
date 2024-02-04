#
# A CMake module to find SDL2
#
# Once done this module will define:
#  SDL2_FOUND - system has SDL2
#  SDL2_LIBRARY
#  SDL2_IMAGE_LIBRARY
#  - Link these to use SDL2
#

if (NOSDL2)
        SET(SDL2_FOUND FALSE)
else ()
        IF (NOT SDL2_INCLUDE_DIRS OR NOT SDL2_LIBRARY OR NOT SDL2_IMAGE_LIBRARY)
                IF (MSVC)      # Visual Studio
                        SET(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
                        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.dll.lib;.dll.a;.a")
                ELSEIF (MINGW) # Windows
                        SET(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
                        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dll.a;.dll;.a;.lib")
                ELSE (MSVC)    # Linux
                        SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
                        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so;.a")
                ENDIF(MSVC)

                FIND_LIBRARY(SDL2_LIBRARY
                        NAMES
                        SDL2
                        PATHS
                        ${SDL2_DIR}/lib/              # SDL2 root directory (if provided)
                        ${SDL2_DIR}                   # SDL2 root directory (if provided)
                )

                FIND_LIBRARY(SDL2_IMAGE_LIBRARY
                        NAMES
                        SDL2_image
                        PATHS
                        ${SDL2_DIR}/lib/              # SDL2 root directory (if provided)
                        ${SDL2_DIR}                   # SDL2 root directory (if provided)
                )

                FIND_PATH(SDL2_INCLUDE_DIRS
                        NAMES
                        SDL2/SDL.h
                        PATHS
                        ${SDL2_DIR}/include/             # SDL2 root directory (if provided)
                        ${SDL2_DIR}                      # SDL2 root directory (if provided)
                )
        ENDIF (NOT SDL2_INCLUDE_DIRS OR NOT SDL2_LIBRARY OR NOT SDL2_IMAGE_LIBRARY)

        IF (SDL2_INCLUDE_DIRS AND SDL2_LIBRARY AND SDL2_IMAGE_LIBRARY)
                SET(SDL2_FOUND TRUE)
        ELSE ()
                SET(SDL2_FOUND FALSE)
        ENDIF ()
endif ()

IF (SDL2_FIND_REQUIRED AND NOT SDL2_FOUND)
        MESSAGE(FATAL_ERROR
                "  SDL2 not found.\n"
                "      Fill CMake variable SDL2_DIR to the provided directory.\n"
                )
ENDIF (SDL2_FIND_REQUIRED AND NOT SDL2_FOUND)