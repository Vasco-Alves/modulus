#!/bin/bash

# Define the root project name
PROJECT_NAME="modulus"

echo "🚀 Initializing project structure for '$PROJECT_NAME'..."

# 1. Create the Root Directory
mkdir -p "$PROJECT_NAME"
cd "$PROJECT_NAME" || exit

# 2. Create Top-Level Directories
echo "📂 Creating directories..."
mkdir -p cmake
mkdir -p external
mkdir -p tests

# 3. Create 'include' Directory (Public API)
# This is where the user-facing headers go (The "Stable Mental Model")
mkdir -p include/modulus/core
mkdir -p include/modulus/platform
mkdir -p include/modulus/gfx

# 4. Create 'src' Directory (Private Implementation)
# This is where the backend logic hides
mkdir -p src/core
mkdir -p src/platform/backends/glfw   # Placeholder for the MVP backend
mkdir -p src/gfx/backends/opengl      # Placeholder for the MVP backend

# 5. Create Examples Directory
mkdir -p examples/hello_console
mkdir -p examples/hello_window

# 6. Create File Stubs (Root)
echo "📄 Creating file stubs..."

# .gitignore (Essential for C++ projects)
cat <<EOF > .gitignore
# Build directories
build/
bin/
lib/
out/

# IDE settings
.vscode/
.idea/
*.sln
*.vcxproj
*.vcxproj.user

# OS specifics
.DS_Store
Thumbs.db
EOF

# CMakePresets.json (Modern CMake best practice)
echo "{}" > CMakePresets.json

# Root CMakeLists.txt
echo "# TODO: Root CMake Configuration" > CMakeLists.txt

# README
echo "# Modulus Framework" > README.md

# 7. Create File Stubs (Public API Headers)
# Core
touch include/modulus/core/log.hpp
touch include/modulus/core/types.hpp

# Platform Interface
touch include/modulus/platform/window.hpp
touch include/modulus/platform/input.hpp

# Gfx Interface
touch include/modulus/gfx/context.hpp
touch include/modulus/gfx/color.hpp

# The "Mega Header"
touch include/modulus/modulus.hpp

# 8. Create File Stubs (Private Implementation)
# Core
touch src/core/log.cpp

# Platform
touch src/platform/platform.cpp
touch src/platform/backends/glfw/platform_glfw.cpp

# Gfx
touch src/gfx/gfx.cpp
touch src/gfx/backends/opengl/gfx_opengl.cpp

# 9. Create Example Stubs
touch examples/hello_console/main.cpp
touch examples/hello_window/main.cpp

# 10. Create Test Stub
touch tests/test_main.cpp

echo "✅ Project '$PROJECT_NAME' structure created successfully!"
echo "   Next steps:"
echo "   1. Populate cmake/ with modules (e.g. FindGLFW.cmake)"
echo "   2. Configure the root CMakeLists.txt"
