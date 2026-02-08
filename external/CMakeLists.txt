include(FetchContent)

# ==============================================================================
# 1. GLFW (For Platform Module)
# ==============================================================================
if(MODULUS_BUILD_PLATFORM)
    # CHECK: Did the parent project (Engine + vcpkg) already provide GLFW?
    if(TARGET glfw OR TARGET glfw3::glfw)
        message(STATUS "Modulus: Using existing GLFW from parent project.")
        
        # Compatibility: Modulus code expects a target named "glfw".
        # If vcpkg provided "glfw3::glfw", we create an alias.
        if(TARGET glfw3::glfw AND NOT TARGET glfw)
            add_library(glfw ALIAS glfw3::glfw)
        endif()

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
    # CHECK: Did vcpkg provide GLM? (Target is usually 'glm::glm')
    if(TARGET glm OR TARGET glm::glm)
        message(STATUS "Modulus: Using existing GLM from parent project.")

        # Compatibility: Modulus code expects "glm", vcpkg provides "glm::glm"
        if(TARGET glm::glm AND NOT TARGET glm)
            add_library(glm ALIAS glm::glm)
        endif()

    else()
        # FALLBACK: Download it.
        message(STATUS "Modulus: Fetching GLM...")
        FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG        0.9.9.8
        )

        FetchContent_GetProperties(glm)
        if(NOT glm_POPULATED)
            FetchContent_Populate(glm)
            
            add_library(glm INTERFACE)
            target_include_directories(glm INTERFACE ${glm_SOURCE_DIR})
        endif()
    endif()
endif()