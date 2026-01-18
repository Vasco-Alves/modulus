include(FetchContent)

# ==============================================================================
# 1. GLFW (For Platform Module)
# ==============================================================================
if(MODULUS_BUILD_PLATFORM)
    message(STATUS "Modulus: Fetching GLFW...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.3.8
    )

    # Configure GLFW to be lightweight (no docs, no tests, no installation)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(glfw)
endif()

# ==============================================================================
# 2. GLAD (For Gfx Module) - MANUAL DOWNLOAD
# ==============================================================================
# Expected Structure:
# modulus/
#   external/
#     glad/
#       include/
#         glad/
#           glad.h
#         KHR/
#           khrplatform.h
#       src/
#         glad.c
#       CMakeLists.txt

if(MODULUS_BUILD_GFX)
    # Check if the user has actually put the files there
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/glad/include/glad/glad.h")
        message(STATUS "Modulus: Found GLAD")
        add_subdirectory(glad) 
    else()
        message(WARNING "Modulus: GLAD not found in external/glad! Gfx module may fail to build.")
        message(WARNING "  -> Please download GLAD and place it in external/glad/")
    endif()
endif()

# ==============================================================================
# 3. miniaudio (For Audio Module) - MANUAL DOWNLOAD
# ==============================================================================
if(MODULUS_BUILD_AUDIO)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/miniaudio/miniaudio.h")
        message(STATUS "Modulus: Found miniaudio")
        
        # Create a "fake" library that just points to the include folder
        add_library(miniaudio INTERFACE)
        target_include_directories(miniaudio INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/miniaudio")
        
    else()
        message(WARNING "Modulus: miniaudio.h not found in external/miniaudio/")
    endif()
endif()

# ==============================================================================
# 4. GLM (Mathematics)
# ==============================================================================
if(MODULUS_BUILD_GFX)
    message(STATUS "Modulus: Fetching GLM...")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        0.9.9.8
    )

    # MANUAL POPULATION
    # This downloads the files but DOES NOT run GLM's CMakeLists.txt
    FetchContent_GetProperties(glm)
    if(NOT glm_POPULATED)
        FetchContent_Populate(glm)
        
        # We manually create the "glm" library target here.
        # "INTERFACE" means "There is nothing to compile, just headers to include."
        add_library(glm INTERFACE)
        target_include_directories(glm INTERFACE ${glm_SOURCE_DIR})
    endif()
endif()
