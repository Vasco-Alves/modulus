#pragma once

// Includes the headers of every Modulus module your target links, so one include is enough:
//   #include <modulus/modulus.hpp>
//
// Each module's CMake target defines MODULUS_HAS_<MODULE>, so modules you don't link are skipped.
// (When you add a public header, add it here too: the tests compile this file to check every header.)

// Core (always there)
#include "modulus/core/file.hpp"
#include "modulus/core/log.hpp"
#include "modulus/core/timer.hpp"

#ifdef MODULUS_HAS_PLATFORM
#include "modulus/platform/input.hpp"
#include "modulus/platform/platform.hpp"
#include "modulus/platform/window.hpp"
#endif

#ifdef MODULUS_HAS_GFX
#include "modulus/gfx/buffer.hpp"
#include "modulus/gfx/buffer_layout.hpp"
#include "modulus/gfx/camera_2d.hpp"
#include "modulus/gfx/color.hpp"
#include "modulus/gfx/font.hpp"
#include "modulus/gfx/graphics.hpp"
#include "modulus/gfx/renderer_2d.hpp"
#include "modulus/gfx/shader.hpp"
#include "modulus/gfx/sprite_sheet.hpp"
#include "modulus/gfx/texture.hpp"
#include "modulus/gfx/vertex_array.hpp"
#endif

#ifdef MODULUS_HAS_AUDIO
#include "modulus/audio/audio.hpp"
#endif
