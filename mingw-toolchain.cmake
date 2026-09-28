# MinGW cross-compilation toolchain for x64-w64-mingw32
# This file enables cross-compilation from Linux to Windows 64-bit
# 
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake -B build-mingw
#
# Prerequisites (on Linux):
#   sudo apt install mingw-w64 mingw-w64-x86-64-dev mingw-w64-x86-64-tools

# Set system name to Windows
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# MinGW cross-compiler prefix
set(MINGW_PREFIX "x86_64-w64-mingw32")

# Set C and C++ compiler. The "-posix" variants (Debian/Ubuntu mingw-w64)
# have the posix thread model, needed for std::thread/std::this_thread;
# the plain names default to the "win32" thread model on those systems.
find_program(MINGW_CC NAMES ${MINGW_PREFIX}-gcc-posix ${MINGW_PREFIX}-gcc)
find_program(MINGW_CXX NAMES ${MINGW_PREFIX}-g++-posix ${MINGW_PREFIX}-g++)
set(CMAKE_C_COMPILER ${MINGW_CC})
set(CMAKE_CXX_COMPILER ${MINGW_CXX})
set(CMAKE_RC_COMPILER ${MINGW_PREFIX}-windres)

# Archiver
set(CMAKE_AR ${MINGW_PREFIX}-ar)
set(CMAKE_RANLIB ${MINGW_PREFIX}-ranlib)

# Find libraries in the cross-compile environment
set(CMAKE_FIND_ROOT_PATH /usr/${MINGW_PREFIX})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")

# Default to Release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Tell CMake we're cross-compiling
set(CMAKE_CROSSCOMPILING TRUE)

message(STATUS "MinGW cross-compilation toolchain loaded")
message(STATUS "  Target: Windows 64-bit (${MINGW_PREFIX})")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  CXX Compiler: ${CMAKE_CXX_COMPILER}")
