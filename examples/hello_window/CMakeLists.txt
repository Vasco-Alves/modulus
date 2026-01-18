add_executable(hello_window main.cpp)

# target_link_libraries(hello_window PRIVATE modulus::modulus)
target_link_libraries(hello_window PRIVATE 
    modulus::core       # For Logging, File I/O
    modulus::platform   # For Window, Input
    modulus::gfx        # For Rendering
    modulus::audio      # For Sound
)

# Ensure it can find the headers
target_include_directories(hello_window PRIVATE "${CMAKE_SOURCE_DIR}/include")

# Copy assets folder to build destination
file(COPY "${CMAKE_SOURCE_DIR}/assets" DESTINATION "${CMAKE_BINARY_DIR}/bin")