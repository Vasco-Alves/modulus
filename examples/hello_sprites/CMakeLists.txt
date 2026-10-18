add_executable(hello_sprites main.cpp)

target_link_libraries(hello_sprites PRIVATE
    modulus::core       # For Timer
    modulus::platform   # For the window and input
    modulus::gfx        # For the 2D renderer and sprite sheets
)
