cmake_minimum_required(VERSION 3.25)

# Project definition
project(
    Rmt
    VERSION 2.0.0
    DESCRIPTION "RASTER Music Tracker - Atari XL/XE music editor"
    LANGUAGES CXX C
)

# ---------------------------------------------------------------------------
# Version string: derived from the nearest git tag (v2.0-rc1, v2.1, …).
# RMT_BASE_VERSION is the numeric major.minor used by scripts/push.sh.
# RMT_VERSION_FULL is the human-readable string baked into the binary.
# ---------------------------------------------------------------------------
set(RMT_BASE_VERSION "2.1")

find_package(Git QUIET)
if(Git_FOUND)
    # Exact tag on current commit?
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" describe --tags --match "v[0-9]*" --exact-match
        OUTPUT_VARIABLE _RMT_EXACT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
    if(_RMT_EXACT_TAG)
        string(REGEX REPLACE "^v" "" RMT_VERSION_FULL "${_RMT_EXACT_TAG}")
    else()
        # Between tags: nearest tag + "+dev"
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" describe --tags --match "v[0-9]*" --abbrev=0
            OUTPUT_VARIABLE _RMT_NEAREST_TAG
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        )
        if(_RMT_NEAREST_TAG)
            string(REGEX REPLACE "^v" "" _RMT_NEAREST_TAG "${_RMT_NEAREST_TAG}")
            set(RMT_VERSION_FULL "${_RMT_NEAREST_TAG}+dev")
        else()
            set(RMT_VERSION_FULL "${RMT_BASE_VERSION}-dev")
        endif()
    endif()
else()
    set(RMT_VERSION_FULL "${RMT_BASE_VERSION}-dev")
endif()

message(STATUS "RMT version: ${RMT_VERSION_FULL}")

# Platform detection EARLY
if(WIN32)
    set(WINDOWS_PLATFORM TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_CROSSCOMPILING)
    # Also True if cross-compiling for Windows
    set(WINDOWS_PLATFORM TRUE)
else()
    set(WINDOWS_PLATFORM FALSE)
endif()

# Core-only build option (for Phase 2 development on non-Windows)
option(RMT_BUILD_CORE_ONLY "Build only core components without UI (for testing audio/MIDI backends)" OFF)
if(RMT_BUILD_CORE_ONLY)
    message(STATUS "Building core components only (no UI) - for Phase 2 development")
endif()


# Qt5 native GUI frontend: the official build on Linux/POSIX (default ON there).
# Windows keeps the MFC GUI by default; -DRMT_USE_QT=ON selects Qt there too.
if(WINDOWS_PLATFORM)
    option(RMT_USE_QT "Build the Qt5 native GUI frontend instead of MFC" OFF)
else()
    option(RMT_USE_QT "Build the Qt5 native GUI frontend (official build on this platform)" ON)
endif()

# Check if MFC is available
if(NOT WINDOWS_PLATFORM AND NOT CMAKE_CROSSCOMPILING AND NOT RMT_BUILD_CORE_ONLY AND NOT RMT_USE_QT)
    message(FATAL_ERROR
        "The MFC GUI (RMT_USE_QT=OFF) can only be built on Windows with MSVC.\n"
        "\n"
        "To build on this system:\n"
        "  - Build the Qt5 frontend, the official build here: cmake -DRMT_USE_QT=ON (the default)\n"
        "  - Or build core only: cmake -DRMT_BUILD_CORE_ONLY=ON (for testing)\n"
        "See BUILD.md.\n"
    )
endif()

# Set C++ standard globally
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set C standard globally
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Platform: Windows=${WINDOWS_PLATFORM}, CrossCompiling=${CMAKE_CROSSCOMPILING}")

# Compiler-specific settings
if(MSVC)
    # MSVC specific settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive- /MP")
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Ob0 /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /Ob2 /DNDEBUG /GL")
    
    # Warning level
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    endif()
    
    # Link time code generation for Release
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang specific settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unknown-pragmas")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    
    # Link time optimization for Release
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
    endif()
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

# Runtime files next to the programs: the tracker drivers (rmt_driver_v*.obx)
# and the other files RMT loads from <program folder>/resources
file(COPY ${PROJECT_SOURCE_DIR}/rmt/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Generate version header from git tag (used by MfcTypes.cpp and Qt frontend)
configure_file(
    "${PROJECT_SOURCE_DIR}/src/RmtVersion.h.in"
    "${CMAKE_BINARY_DIR}/RmtVersion.h"
    @ONLY
)

# Main source subdirectory
add_subdirectory(src)

# Print configuration summary
message(STATUS "")
message(STATUS "=== RMT CMake Configuration ===")
message(STATUS "Version: ${RMT_VERSION_FULL}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "C Standard: C${CMAKE_C_STANDARD}")
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Target: ${CMAKE_SYSTEM_NAME}")
endif()
message(STATUS "===============================")
