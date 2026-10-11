cmake_minimum_required(VERSION 3.20)

# ==============================================================================
# Project Metadata
# ==============================================================================
project(modulus 
    VERSION 0.1.1 
    DESCRIPTION "A unified, modular C++ framework for multimedia applications"
    LANGUAGES C CXX
)

# ==============================================================================
# 1. Smart Detection: Standalone vs. Submodule
# ==============================================================================
# If the folder we are in is the "Root" of the CMake build, we are Standalone.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MODULUS_IS_TOP_LEVEL ON)
else()
    set(MODULUS_IS_TOP_LEVEL OFF)
endif()

# ==============================================================================
# Global Compiler Settings
# ==============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# Output Directory Layout
# ==============================================================================
# Only force output directories if we are the Top Level project.
# Otherwise, we respect the Engine's output preferences.
if(MODULUS_IS_TOP_LEVEL)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# ==============================================================================
# Module Configuration
# ==============================================================================

# Core is always built. The other modules are optional.
option(MODULUS_BUILD_PLATFORM "Build the Platform module" ON)
option(MODULUS_BUILD_GFX      "Build the Graphics module" ON)
option(MODULUS_BUILD_AUDIO    "Build the Audio module"    ON)

# Dynamic Options:
# If standalone, default Examples to ON. 
# If submodule (inside Engine), default Examples to OFF to save build time.
option(MODULUS_BUILD_EXAMPLES "Build the example applications" ${MODULUS_IS_TOP_LEVEL})
option(MODULUS_BUILD_TESTS    "Build unit tests"               OFF)

message(STATUS "-------------------------------------------------")
message(STATUS "Modulus Configuration (Top Level: ${MODULUS_IS_TOP_LEVEL})")
message(STATUS "  Platform Module: ${MODULUS_BUILD_PLATFORM}")
message(STATUS "  Graphics Module: ${MODULUS_BUILD_GFX}")
message(STATUS "  Audio Module:    ${MODULUS_BUILD_AUDIO}")
message(STATUS "  Examples:        ${MODULUS_BUILD_EXAMPLES}")
message(STATUS "-------------------------------------------------")

# ==============================================================================
# Dependencies
# ==============================================================================
# Adds only the libraries the enabled modules need (GLFW, GLAD, GLM, miniaudio).
add_subdirectory(external)

# ==============================================================================
# Build Targets
# ==============================================================================

add_subdirectory(src)

if(MODULUS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(MODULUS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()