add_executable(hello_text main.cpp)

target_link_libraries(hello_text PRIVATE
    modulus::core       # For File and Timer
    modulus::platform   # For the window and input
    modulus::gfx        # For the 2D renderer and fonts
)

# Copy the fonts next to the program on every build, so that
# File::app_path("assets/fonts/...") finds them
add_custom_target(hello_text_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/assets/fonts"
            "$<TARGET_FILE_DIR:hello_text>/assets/fonts"
    COMMENT "Copying fonts next to hello_text"
)
add_dependencies(hello_text hello_text_assets)
