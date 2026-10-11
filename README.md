# Modulus Framework

**Modulus** is a modular C++20 framework that wraps well-known open-source libraries behind one simple, consistent API.

Instead of spending the start of every project gluing libraries together (windows, graphics, audio, ...), you link the Modulus modules you need and start writing your app.

> **Status:** early prototype. Each module has one backend for now, and APIs will change.

## Core Philosophy

- **Stable Mental Model:** Learn one API, use it everywhere. No need to memorize the quirks of 5 different libraries.
- **Pay for what you use:** Every module is its own CMake target. A console app links only `modulus::core`. Need everything? Link `modulus::modulus`.
- **Readable first:** The code is meant to be easy to read and learn from.

## Modules

| Module   | CMake target        | What it does                              | Built on                          |
| -------- | ------------------- | ----------------------------------------- | --------------------------------- |
| Core     | `modulus::core`     | Logging, files and asset paths, timer     | No external libraries             |
| Platform | `modulus::platform` | Window, keyboard and mouse input          | GLFW                              |
| Gfx      | `modulus::gfx`      | Shaders, buffers, vertex arrays, textures | OpenGL 3.3 (GLAD), GLM, stb_image |
| Audio    | `modulus::audio`    | Playing sound files                       | miniaudio                         |

## Getting Started

### Prerequisites

- CMake 3.20+
- C++20 compiler (MSVC, GCC, Clang)
- Internet connection for the first build

### Dependencies

There is nothing to install by hand:

- **GLFW** and **GLM** are downloaded automatically by CMake (FetchContent).
- **GLAD**, **miniaudio** and **stb_image** are already included in `external/`.

If your own project already provides GLFW or GLM (for example through vcpkg), Modulus uses those instead of downloading them.

### Build

```bash
git clone https://github.com/Vasco-Alves/modulus.git
cd modulus
cmake -S . -B build
cmake --build build
```

In Visual Studio you can also just open the folder: it picks up `CMakePresets.json` automatically.

### Run the Examples

The programs are in `build/bin` (or `build/bin/Debug` with the Visual Studio generator, the default on Windows). You can start them from any folder:

```bash
./build/bin/hello_console
./build/bin/hello_window
```

## Using Modulus in Your Project

Add Modulus as a subfolder (for example a git submodule) and link only the modules you need:

```cmake
add_subdirectory(external/modulus)

target_link_libraries(my_app PRIVATE
    modulus::core
    modulus::audio
)
```

Your project must use C++20 too, because the Modulus headers use `<format>`.

### Loading Assets

A relative path like `"assets/player.png"` is normally looked up in the folder the program was *started from*, which changes depending on how it is launched. `File::app_path` looks next to the program instead:

```cpp
#include <modulus/core/file.hpp>

auto text = modulus::core::File::read_text(modulus::core::File::app_path("assets/hello.txt"));
```

Then let CMake copy your assets next to the program on every build. [examples/hello_window/CMakeLists.txt](examples/hello_window/CMakeLists.txt) shows how.

### Build Options

| Option                   | Default                                       | Description                       |
| ------------------------ | --------------------------------------------- | --------------------------------- |
| `MODULUS_BUILD_PLATFORM` | `ON`                                          | Window and input                  |
| `MODULUS_BUILD_GFX`      | `ON`                                          | Graphics (needs Platform)         |
| `MODULUS_BUILD_AUDIO`    | `ON`                                          | Audio                             |
| `MODULUS_BUILD_EXAMPLES` | `ON` on its own, `OFF` inside another project | Example programs                  |
| `MODULUS_BUILD_TESTS`    | `OFF`                                         | Unit tests (downloads GoogleTest) |

Core is always built.

## License

MIT. See [LICENSE.txt](LICENSE.txt).
