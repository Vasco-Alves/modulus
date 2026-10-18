# tests/CMakeLists.txt

include(FetchContent)

# ==============================================================================
# 1. Fetch GoogleTest
# ==============================================================================
message(STATUS "Modulus: Fetching GoogleTest...")

# Give downloaded files the time they were extracted, so a new download always rebuilds
# (CMake warns when this isn't chosen)
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

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
    test_headers.cpp
    core/test_file.cpp
    core/test_log.cpp
    core/test_timer.cpp
)

if(MODULUS_BUILD_PLATFORM)
    target_sources(modulus_tests PRIVATE
        platform/test_input.cpp   # Opens a hidden window. Skips itself without a display.
    )

    # The platform tests talk to GLFW directly, to send the window fake input events
    target_link_libraries(modulus_tests PRIVATE glfw)
endif()

if(MODULUS_BUILD_GFX)
    target_sources(modulus_tests PRIVATE
        gfx/test_buffer_layout.cpp
        gfx/test_camera_2d.cpp
        gfx/test_font.cpp          # Needs OpenGL too (for the font texture)
        gfx/test_opengl.cpp   # Opens a hidden window. Skips itself when OpenGL 3.3 isn't available.
        gfx/test_render_target.cpp # The same
        gfx/test_renderer_2d.cpp   # The same
        gfx/test_sprite_sheet.cpp  # The same
    )

    # The OpenGL tests check the GPU state directly, so they also use GLAD
    target_link_libraries(modulus_tests PRIVATE glad)
endif()

if(MODULUS_BUILD_AUDIO)
    target_sources(modulus_tests PRIVATE
        audio/test_audio.cpp   # Opens the default audio device (and only plays silence). Skips itself without one.
    )
endif()

# ==============================================================================
# 3. Linking
# ==============================================================================
target_link_libraries(modulus_tests PRIVATE
    modulus::modulus    # Link your framework
    GTest::gtest_main   # Link GTest (and its main function)
)
modulus_enable_warnings(modulus_tests)

# The TrueType font the tests load (the same one the examples use)
target_compile_definitions(modulus_tests PRIVATE MODULUS_TEST_FONT="${PROJECT_SOURCE_DIR}/assets/fonts/Lato-Regular.ttf")

# ==============================================================================
# 4. Registration (CTest)
# ==============================================================================
include(GoogleTest)

# This magic command parses your C++ files and registers
# every TEST() macro as a separate test in the IDE.
gtest_discover_tests(modulus_tests)
