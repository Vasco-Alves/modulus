# ==============================================================================
# 1. Modulus Core (Standalone - No external dependencies)
# ==============================================================================
add_library(modulus_core STATIC
    core/log.cpp
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
        platform/platform.cpp
        platform/backends/glfw/platform_glfw.cpp
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
    )
    add_library(modulus::gfx ALIAS modulus_gfx)

    target_include_directories(modulus_gfx
        PUBLIC "${PROJECT_SOURCE_DIR}/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    target_link_libraries(modulus_gfx
        PUBLIC  modulus::platform # Needs window handles
        PRIVATE OpenGL::GL
    )
endif()

# ==============================================================================
# 4. The "Mega" Target (modulus::modulus)
# ==============================================================================
# This is an INTERFACE library. It doesn't compile code itself, 
# it just groups the others together.
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