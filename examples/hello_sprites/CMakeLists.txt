add_executable(hello_sprites main.cpp)

target_link_libraries(hello_sprites PRIVATE
    modulus::core       # For File
    modulus::platform   # For the window and input
    modulus::gfx        # For the 2D renderer and sprite sheets
    modulus::audio      # For the coin sound
)

# Copy the sounds next to the program on every build, so that
# File::app_path("assets/sounds/...") finds them
add_custom_target(hello_sprites_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/assets/sounds"
            "$<TARGET_FILE_DIR:hello_sprites>/assets/sounds"
    COMMENT "Copying sounds next to hello_sprites"
)
add_dependencies(hello_sprites hello_sprites_assets)
