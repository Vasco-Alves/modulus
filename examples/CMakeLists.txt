# Always build the console example (it only needs Core)
add_subdirectory(hello_console)

# Only build the window example if Platform, Gfx and Audio are enabled
if(MODULUS_BUILD_PLATFORM AND MODULUS_BUILD_GFX AND MODULUS_BUILD_AUDIO)
    add_subdirectory(hello_window)
endif()