# examples/hello_console/CMakeLists.txt

add_executable(hello_console main.cpp)

# Link only against the core module
target_link_libraries(hello_console PRIVATE modulus::core)

# Ensure it can find the headers
target_include_directories(hello_console PRIVATE "${CMAKE_SOURCE_DIR}/include")