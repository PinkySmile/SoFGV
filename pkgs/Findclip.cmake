#
# A CMake module to find clip
#
# Once done this module will define:
#  clip_FOUND - system has clip
#  clip_INCLUDE_DIRS - the clip include directory
#  clip_LIBRARIES - Link these to use clip
#

IF (NOT clip_INCLUDE_DIRS OR NOT clip_LIBRARIES)
    FIND_PATH(clip_INCLUDE_DIRS
        NAMES
            clip.h
        PATHS
            /usr/include/                 # Default Fedora28 system include path
            /usr/local/include/           # Default Fedora28 local include path
            ${CMAKE_MODULE_PATH}/include/ # Expected to contain the path to this file for Windows10
            ${clip_DIR}/include/         # clip root directory (if provided)
            ${clip_DIR}                  # clip root directory (if provided)
    )

    IF (MSVC)     # Windows
        SET(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.a")
    ELSE (MSVC)   # Linux
        SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so;.a")
    ENDIF(MSVC)
    
    FIND_LIBRARY(clip_LIBRARIES
        NAMES
	    clip
        PATHS
            /usr/lib64/                   # Default Fedora28 library path
            /usr/lib/                     # Some more Linux library path
            /usr/lib/x86_64-linux-gnu/    # Some more Linux library path
            /usr/local/lib/               # Some more Linux library path
            /usr/local/lib64/             # Some more Linux library path
            ${clip_DIR}/lib/              # clip root directory (if provided)
            ${clip_DIR}                   # clip root directory (if provided)
            ${CMAKE_MODULE_PATH}/lib/     # Expected to contain the path to this file for Windows10
    )
ENDIF (NOT clip_INCLUDE_DIRS OR NOT clip_LIBRARIES)

IF (clip_INCLUDE_DIRS AND clip_LIBRARIES)
    SET(clip_FOUND TRUE)
ELSE (clip_INCLUDE_DIRS AND clip_LIBRARIES)
    SET(clip_FOUND FALSE)
ENDIF (clip_INCLUDE_DIRS AND clip_LIBRARIES)

IF (clip_FIND_REQUIRED AND NOT clip_FOUND)
    MESSAGE(FATAL_ERROR
            "  clip not found.\n"
            "      Fill CMake variable clip_DIR to the provided directory.\n"
            )
ENDIF (clip_FIND_REQUIRED AND NOT clip_FOUND)
