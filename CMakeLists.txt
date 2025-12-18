cmake_minimum_required(VERSION 3.20)

# ==============================================================================
# Project Metadata
# ==============================================================================
project(modulus 
    VERSION 0.1.0 
    DESCRIPTION "A unified, modular C++ framework for multimedia applications"
    LANGUAGES C CXX
)

# ==============================================================================
# Global Compiler Settings
# ==============================================================================

# Use C++20 (Required for concepts and modern features)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Disable compiler-specific extensions (MSVC/GCC quirks) to ensure portability
set(CMAKE_CXX_EXTENSIONS OFF) 

# Generate 'compile_commands.json' for Intellisense (VS Code / Clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# Output Directory Layout
# ==============================================================================

# Forces all built binaries and libraries into a clean folder structure
# build/bin/ -> executables
# build/lib/ -> static libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ==============================================================================
# Module Configuration (The "Pay for what you use" Switches)
# ==============================================================================

# Core is always required, but we list it for clarity
option(MODULUS_BUILD_CORE     "Build the Core module (Logging, Types)" ON)

# Optional Modules
option(MODULUS_BUILD_PLATFORM "Build the Platform module (Windowing, Input)" ON)
option(MODULUS_BUILD_GFX      "Build the Graphics module (OpenGL/Vulkan)"    ON)
# option(MODULUS_BUILD_AUDIO  "Build the Audio module"                       OFF) # Future

# Build Targets
option(MODULUS_BUILD_EXAMPLES "Build the example applications" ON)
option(MODULUS_BUILD_TESTS    "Build unit tests"               ON)

message(STATUS "-------------------------------------------------")
message(STATUS "Modulus Configuration:")
message(STATUS "  Platform Module: ${MODULUS_BUILD_PLATFORM}")
message(STATUS "  Graphics Module: ${MODULUS_BUILD_GFX}")
message(STATUS "  Examples:        ${MODULUS_BUILD_EXAMPLES}")
message(STATUS "  Tests:           ${MODULUS_BUILD_TESTS}")
message(STATUS "-------------------------------------------------")

# ==============================================================================
# Dependencies
# ==============================================================================

# Add local cmake modules (if you add custom FindX.cmake scripts later)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 1. External Dependencies
# This runs first so libraries (like GLFW) are available when we build 'src'
add_subdirectory(external)

# ==============================================================================
# Build Targets
# ==============================================================================

# 2. Main Library Source
add_subdirectory(src)

# 3. Examples (Optional)
if(MODULUS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# 4. Unit Tests (Optional)
if(MODULUS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()