# examples/hello_window/CMakeLists.txt

add_executable(hello_window main.cpp)

# Link against the "Mega Target" which pulls in Platform, Gfx, and Core
target_link_libraries(hello_window PRIVATE modulus::modulus)

# Ensure it can find the headers
target_include_directories(hello_window PRIVATE "${CMAKE_SOURCE_DIR}/include")