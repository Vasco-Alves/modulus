# ==============================================================================
# 1. Modulus Core (Standalone - No external dependencies)
# ==============================================================================
add_library(modulus_core STATIC
    "core/log.cpp"
    "core/file.cpp"
)
add_library(modulus::core ALIAS modulus_core)

target_include_directories(modulus_core
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
# Core doesn't link to anything!

# The headers use C++20 (<format>), so anything that links Modulus is compiled as C++20 too
target_compile_features(modulus_core PUBLIC cxx_std_20)
modulus_enable_warnings(modulus_core)

# ==============================================================================
# 2. Modulus Platform (Depends on Core + GLFW)
# ==============================================================================
if(MODULUS_BUILD_PLATFORM)
    add_library(modulus_platform STATIC
        "platform/backends/glfw/platform_glfw.cpp"
        "platform/backends/glfw/input_glfw.cpp"
    )
    add_library(modulus::platform ALIAS modulus_platform)

    target_include_directories(modulus_platform
        PUBLIC "${PROJECT_SOURCE_DIR}/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    # Links to Core for logging/types, and GLFW for implementation
    target_link_libraries(modulus_platform
        PUBLIC  modulus::core
        PRIVATE glfw
    )
    target_compile_definitions(modulus_platform PUBLIC MODULUS_HAS_PLATFORM) # Lets modulus/modulus.hpp include it
    modulus_enable_warnings(modulus_platform)
endif()

# ==============================================================================
# 3. Modulus Gfx (Depends on Platform + OpenGL)
# ==============================================================================
if(MODULUS_BUILD_GFX)
    find_package(OpenGL REQUIRED)

    add_library(modulus_gfx STATIC
        "gfx/backends/opengl/gfx_opengl.cpp"
        "gfx/backends/opengl/shader.cpp"
        "gfx/backends/opengl/buffer.cpp"
        "gfx/backends/opengl/vertex_array.cpp"
        "gfx/backends/opengl/texture.cpp"
    )
    add_library(modulus::gfx ALIAS modulus_gfx)

    target_include_directories(modulus_gfx
        PUBLIC "${PROJECT_SOURCE_DIR}/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../external/stb_image"
    )

    target_link_libraries(modulus_gfx
        PUBLIC  modulus::platform # Draws into a platform window (IWindow)
        PUBLIC  glm::glm          # Public because shader.hpp uses glm::mat4
        PRIVATE glad
        PRIVATE OpenGL::GL
    )
    target_compile_definitions(modulus_gfx PUBLIC MODULUS_HAS_GFX) # Lets modulus/modulus.hpp include it
    modulus_enable_warnings(modulus_gfx)
endif()

# ==============================================================================
# 4. Modulus Audio (Depends on miniaudio)
# ==============================================================================
if(MODULUS_BUILD_AUDIO)
    add_library(modulus_audio STATIC
        "audio/audio.cpp"
    )

    add_library(modulus::audio ALIAS modulus_audio)

    target_include_directories(modulus_audio
        PUBLIC "${PROJECT_SOURCE_DIR}/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    target_link_libraries(modulus_audio
        PUBLIC modulus::core   # Needs logging
        PRIVATE miniaudio      # The library we defined in external/CMakeLists.txt
    )
    target_compile_definitions(modulus_audio PUBLIC MODULUS_HAS_AUDIO) # Lets modulus/modulus.hpp include it
    modulus_enable_warnings(modulus_audio)
endif()


# ==============================================================================
# 5. The "Mega" Target (modulus::modulus)
# ==============================================================================
# This is an INTERFACE library. It doesn't compile code itself, it just groups the others together.
add_library(modulus_all INTERFACE)
add_library(modulus::modulus ALIAS modulus_all)

target_link_libraries(modulus_all INTERFACE
    modulus::core
)

if(MODULUS_BUILD_PLATFORM)
    target_link_libraries(modulus_all INTERFACE modulus::platform)
endif()

if(MODULUS_BUILD_GFX)
    target_link_libraries(modulus_all INTERFACE modulus::gfx)
endif()

if(MODULUS_BUILD_AUDIO)
    target_link_libraries(modulus_all INTERFACE modulus::audio)
endif()
    