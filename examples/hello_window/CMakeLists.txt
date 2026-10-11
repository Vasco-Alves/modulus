add_executable(hello_window main.cpp)

# target_link_libraries(hello_window PRIVATE modulus::modulus)
target_link_libraries(hello_window PRIVATE
    modulus::core       # For Logging, File I/O
    modulus::platform   # For Window, Input
    modulus::gfx        # For Rendering
    modulus::audio      # For Sound
)

# Copy the assets folder next to the program on every build, so that
# File::app_path("assets/...") finds it and edited shaders are picked up.
# $<TARGET_FILE_DIR:hello_window> is the folder where hello_window.exe ends up.
add_custom_target(hello_window_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:hello_window>/assets"
    COMMENT "Copying assets next to hello_window"
)
add_dependencies(hello_window hello_window_assets)
