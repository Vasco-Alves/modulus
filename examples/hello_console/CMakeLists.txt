# examples/hello_console/CMakeLists.txt

add_executable(hello_console main.cpp)

# Link only against the core module (this also gives us its headers)
target_link_libraries(hello_console PRIVATE modulus::core)