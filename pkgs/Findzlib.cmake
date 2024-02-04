#
# A CMake module to find zlib
#
# Once done this module will define:
#  ZLIB_FOUND - system has zlib
#  ZLIB_LIBRARY - Link this to use zlib
#

IF (NOT ZLIB_INCLUDE_DIRS OR NOT ZLIB_LIBRARY)
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

        FIND_LIBRARY(ZLIB_LIBRARY
                NAMES
                z
                zlib
                PATHS
                ${ZLIB_DIR}/lib/zlib/         # ZLIB root directory (if provided)
                ${ZLIB_DIR}/lib/              # ZLIB root directory (if provided)
                ${ZLIB_DIR}                   # ZLIB root directory (if provided)
        )

        FIND_PATH(ZLIB_INCLUDE_DIRS
                NAMES
                zlib.h
                PATHS
                ${ZLIB_DIR}/include/zlib         # ZLIB root directory (if provided)
                ${ZLIB_DIR}/include/             # ZLIB root directory (if provided)
                ${ZLIB_DIR}                      # ZLIB root directory (if provided)
        )
ENDIF (NOT ZLIB_INCLUDE_DIRS OR NOT ZLIB_LIBRARY)

IF (ZLIB_INCLUDE_DIRS AND ZLIB_LIBRARY)
        SET(ZLIB_FOUND TRUE)
ELSE (ZLIB_INCLUDE_DIRS AND ZLIB_LIBRARY)
        SET(ZLIB_FOUND FALSE)
ENDIF (ZLIB_INCLUDE_DIRS AND ZLIB_LIBRARY)

IF (zlib_FIND_REQUIRED AND NOT ZLIB_FOUND)
        MESSAGE(FATAL_ERROR
                "  zlib not found.\n"
                "      Fill CMake variable ZLIB_DIR to the provided directory.\n"
                )
ENDIF (zlib_FIND_REQUIRED AND NOT ZLIB_FOUND)
