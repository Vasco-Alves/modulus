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
#       CMakeLists.txt (We will create this below)

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
# 3. miniaudio (For Audio Module)
# ==============================================================================
# We fetch the single header file. 
# Since it's not a library with a CMakeLists.txt, we define the target ourselves here.

if(MODULUS_BUILD_AUDIO)
    message(STATUS "Modulus: Fetching miniaudio...")
    FetchContent_Declare(
        miniaudio
        URL https://raw.githubusercontent.com/mackron/miniaudio/0.11.11/miniaudio.h
        DOWNLOAD_NO_EXTRACT TRUE
    )
    FetchContent_MakeAvailable(miniaudio)

    # Define a clean target for the rest of Modulus to use
    add_library(miniaudio INTERFACE)
    target_include_directories(miniaudio INTERFACE "${miniaudio_SOURCE_DIR}")
    # Note: miniaudio implementation must be #defined in one .cpp file later.
endif()