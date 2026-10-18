include(FetchContent)

# ==============================================================================
# 1. GLFW (For Platform Module)
# ==============================================================================
if(MODULUS_BUILD_PLATFORM)
    # Did the parent project already provide GLFW (for example with find_package(glfw3) through vcpkg)?
    # Both that and the download below create a target named "glfw", which src/ links directly.
    if(TARGET glfw)
        message(STATUS "Modulus: Using existing GLFW from parent project.")
    else()
        # FALLBACK: We are standalone, so download it.
        message(STATUS "Modulus: Fetching GLFW...")
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG        3.3.8
        )

        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

        FetchContent_MakeAvailable(glfw)
    endif()
endif()

# ==============================================================================
# 2. GLAD (For Gfx Module) - MODULUS OWNS THIS
# ==============================================================================
if(MODULUS_BUILD_GFX)
    # Check if we have already built it (prevents double-add errors)
    if(NOT TARGET glad)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/glad/include/glad/glad.h")
            message(STATUS "Modulus: Found local GLAD")
            add_subdirectory(glad) 
        else()
            message(FATAL_ERROR "Modulus: GLAD not found in external/glad! Please download it.")
        endif()
    endif()
endif()

# ==============================================================================
# 3. miniaudio (For Audio Module)
# ==============================================================================
if(MODULUS_BUILD_AUDIO)
    if(NOT TARGET miniaudio)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/miniaudio/miniaudio.h")
            message(STATUS "Modulus: Found miniaudio")
            
            add_library(miniaudio INTERFACE)
            target_include_directories(miniaudio INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/miniaudio")
            
        else()
            message(WARNING "Modulus: miniaudio.h not found!")
        endif()
    endif()
endif()

# ==============================================================================
# 4. GLM (Mathematics)
# ==============================================================================
if(MODULUS_BUILD_GFX)
    # Did the parent project already provide GLM (for example with find_package(glm) through vcpkg)?
    # Both that and the download below create a target named "glm::glm", which src/ links directly.
    # (Renaming it with an alias here wouldn't work: an alias of a target that came from
    # find_package() is only visible in this folder, not in src/.)
    if(TARGET glm::glm)
        message(STATUS "Modulus: Using existing GLM from parent project.")
    else()
        # FALLBACK: Download it.
        message(STATUS "Modulus: Fetching GLM...")
        FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG        1.0.1
        )

        # GLM is header-only; don't compile it into a separate library.
        set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)

        FetchContent_MakeAvailable(glm)
    endif()
endif()