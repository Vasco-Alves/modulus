add_executable(hello_window main.cpp)

# target_link_libraries(hello_window PRIVATE modulus::modulus)
target_link_libraries(hello_window PRIVATE
    modulus::core       # For Logging, File I/O
    modulus::platform   # For Window, Input
    modulus::gfx        # For Rendering
    modulus::audio      # For Sound
)

# Copy the assets this program uses next to it on every build, so that
# File::app_path("assets/...") finds them and edited shaders are picked up.
# $<TARGET_FILE_DIR:hello_window> is the folder where hello_window.exe ends up.
add_custom_target(hello_window_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/assets/shaders"
            "$<TARGET_FILE_DIR:hello_window>/assets/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/assets/test.mp3"
            "$<TARGET_FILE_DIR:hello_window>/assets/test.mp3"
    COMMENT "Copying assets next to hello_window"
)
add_dependencies(hello_window hello_window_assets)
