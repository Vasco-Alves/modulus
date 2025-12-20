# Modulus Framework

**Modulus** is a unified, modular C++ framework designed to be the "standard way" to build multimedia applications, tools, and engines.

It acts as a consistent facade over the fragmented C++ ecosystem, allowing developers to write code once (`modulus::platform::create_window`) and switch backends (GLFW, SDL, Windows) simply by changing a configuration line.

## Core Philosophy

- **Stable Mental Model:** Learn one API, use it everywhere. No need to memorize the quirks of 5 different libraries.
- **Pay for what you use:** Need only Audio? Link `modulus::audio`. Need a full engine? Link `modulus::modulus`.
- **Modern C++:** Built with C++20, enforcing strict standards and modern practices.

## Architecture (MVP)

The framework is divided into independent modules:

- **Core:** Logging, Types, Assertions. (Zero dependencies)
- **Platform:** Window creation, Input, Events. (Backend: **GLFW**)
- **Gfx:** Rendering context, frame control. (Backend: **OpenGL 4.6**)
- *(Planned)* **Audio:** Spatial audio and mixing. (Backend: **miniaudio**)

## Getting Started

### Prerequisites

- CMake 3.20+
- C++20 compliant compiler (MSVC, GCC, Clang)

### 1. Clone the Repository

```bash
git clone https://github.com/Vasco-Alves/modulus.git
cd modulus
```

### 2. Setup Dependencies

Modulus uses **FetchContent** for most dependencies (GLFW, miniaudio), so they will download automatically during the build.

**Manual Step: OpenGL Loader (GLAD)**
Because GLAD is a generated loader, you must download it manually:

1. Go to [glad.dav1d.de](https://glad.dav1d.de/)
2. Select **C/C++**, **OpenGL**, **API 4.6**, **Profile: Core**.
3. **Important:** Check "Generate a loader".
4. Download and extract into: `external/glad/`

Structure should look like:

```plaintext
modulus/external/glad/
├── include/
│   └── glad/glad.h
├── src/
│   └── glad.c
└── CMakeLists.txt (Provided in repo)
```

### 3. Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 4. Run Example

```bash
./bin/hello_window
```
