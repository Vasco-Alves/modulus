# Simple build script for GLAD
add_library(glad STATIC src/glad.c)

target_include_directories(glad PUBLIC "include")

# GLAD needs to use the system's dynamic loading library (dl) on Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(glad PRIVATE dl)
endif()