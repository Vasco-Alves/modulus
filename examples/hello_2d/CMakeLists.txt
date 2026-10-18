add_executable(hello_2d main.cpp)

target_link_libraries(hello_2d PRIVATE
    modulus::core       # For Timer
    modulus::platform   # For the window and input
    modulus::gfx        # For the 2D renderer
)
