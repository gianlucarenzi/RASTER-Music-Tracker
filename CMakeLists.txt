cmake_minimum_required(VERSION 3.25)

# Project definition
project(
    Rmt
    VERSION 1.35.0
    DESCRIPTION "RASTER Music Tracker - Atari XL/XE music editor"
    LANGUAGES CXX C
)

# Platform detection EARLY
if(WIN32)
    set(WINDOWS_PLATFORM TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_CROSSCOMPILING)
    # Also True if cross-compiling for Windows
    set(WINDOWS_PLATFORM TRUE)
else()
    set(WINDOWS_PLATFORM FALSE)
endif()

# Check if MFC is available
if(NOT WINDOWS_PLATFORM AND NOT CMAKE_CROSSCOMPILING)
    message(FATAL_ERROR 
        "RASTER Music Tracker v1.35 currently requires Windows/MFC.\n"
        "This is Phase 1 of the cross-platform migration.\n"
        "\n"
        "To build on this system:\n"
        "  - Use MinGW cross-compile from Linux: cmake -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake\n"
        "  - Or use Windows with MSVC/MinGW native\n"
        "\n"
        "Phase 2 will add native Linux/macOS support (wxWidgets, PortAudio, RtMidi)."
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
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_VERSION VERSION_GREATER_EQUAL 11)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
    endif()
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

# Main source subdirectory
add_subdirectory(src)

# Print configuration summary
message(STATUS "")
message(STATUS "=== RMT CMake Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "C Standard: C${CMAKE_C_STANDARD}")
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Target: ${CMAKE_SYSTEM_NAME}")
endif()
message(STATUS "===============================")
