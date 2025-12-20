# tests/CMakeLists.txt

include(FetchContent)

# ==============================================================================
# 1. Fetch GoogleTest
# ==============================================================================
message(STATUS "Modulus: Fetching GoogleTest...")

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# Crucial for Windows (MSVC): 
# Forces GTest to use the same runtime library (DLL) as your project.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# ==============================================================================
# 2. Define Test Executable
# ==============================================================================
add_executable(modulus_tests 
    test_main.cpp
    # Add other test files here later, e.g.:
    # core/test_log.cpp
    # platform/test_window.cpp
)

# ==============================================================================
# 3. Linking
# ==============================================================================
target_link_libraries(modulus_tests PRIVATE 
    modulus::modulus    # Link your framework
    GTest::gtest_main   # Link GTest (and its main function)
)

# Ensure tests can include "modulus/core/log.hpp"
target_include_directories(modulus_tests PRIVATE "${PROJECT_SOURCE_DIR}/include")

# ==============================================================================
# 4. Registration (CTest)
# ==============================================================================
include(GoogleTest)

# This magic command parses your C++ files and registers 
# every TEST() macro as a separate test in the IDE.
gtest_discover_tests(modulus_tests)