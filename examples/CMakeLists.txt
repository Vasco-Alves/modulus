# Each example copies the assets it uses next to it (see hello_window), and only those: the
# examples are usually built into the same folder, and copying the same files from two examples
# at once (as a parallel build does) can fail.

# Always build the console example (it only needs Core)
add_subdirectory(hello_console)

# Only build the window example if Platform, Gfx and Audio are enabled
if(MODULUS_BUILD_PLATFORM AND MODULUS_BUILD_GFX AND MODULUS_BUILD_AUDIO)
    add_subdirectory(hello_window)
endif()

# The 2D examples need Platform and Gfx
if(MODULUS_BUILD_PLATFORM AND MODULUS_BUILD_GFX)
    add_subdirectory(hello_2d)
    add_subdirectory(hello_text)

    # This one plays a sound too
    if(MODULUS_BUILD_AUDIO)
        add_subdirectory(hello_sprites)
    endif()
endif()