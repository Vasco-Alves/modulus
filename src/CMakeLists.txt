# ==============================================================================
# 1. Modulus Core (Standalone - No external dependencies)
# ==============================================================================
add_library(modulus_core STATIC
    core/log.cpp
    core/file.cpp
)
add_library(modulus::core ALIAS modulus_core)

target_include_directories(modulus_core
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
# Core doesn't link to anything!

# ==============================================================================
# 2. Modulus Platform (Depends on Core + GLFW)
# ==============================================================================
if(MODULUS_BUILD_PLATFORM)
    add_library(modulus_platform STATIC
        platform/backends/glfw/platform_glfw.cpp
        platform/backends/glfw/input_glfw.cpp
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
endif()

# ==============================================================================
# 3. Modulus Gfx (Depends on Platform + OpenGL)
# ==============================================================================
if(MODULUS_BUILD_GFX)
    find_package(OpenGL REQUIRED)

    add_library(modulus_gfx STATIC
        gfx/gfx.cpp
        gfx/backends/opengl/gfx_opengl.cpp
        gfx/backends/opengl/shader.cpp
        gfx/backends/opengl/buffer.cpp
        gfx/backends/opengl/vertex_array.cpp
    )
    add_library(modulus::gfx ALIAS modulus_gfx)

    target_include_directories(modulus_gfx
        PUBLIC "${PROJECT_SOURCE_DIR}/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    target_link_libraries(modulus_gfx
        PUBLIC  modulus::platform # Needs window handles
        PRIVATE glad
        PRIVATE OpenGL::GL
        PRIVATE glfw
    )
endif()

# ==============================================================================
# 4. Modulus Audio (Depends on miniaudio)
# ==============================================================================
if(MODULUS_BUILD_AUDIO)
    add_library(modulus_audio STATIC
        audio/audio.cpp
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
#     # Define MINIAUDIO_IMPLEMENTATION in this compilation unit
#     target_compile_definitions(modulus_audio PRIVATE MINIAUDIO_IMPLEMENTATION)
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

if(MODULUS_BUILD_GFX)
    target_link_libraries(modulus_all INTERFACE modulus::audio)
endif()
    