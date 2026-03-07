#pragma once

#include "modulus/core/log.hpp"
#include "modulus/platform/platform.hpp"

// We will need these to init them automatically if requested
#include "modulus/gfx/graphics.hpp" 
#include "modulus/audio/audio.hpp"

#include <cstdint>

namespace modulus {

	// A bitmask to select which modules to initialize
	enum class InitFlags : uint32_t {
		None = 0,
		CoreOnly = 1 << 0, // 1
		Platform = 1 << 1, // 2
		Graphics = 1 << 2, // 4
		Audio = 1 << 3, // 8

		// Helper for games
		Everything = CoreOnly | Platform | Graphics | Audio
	};

	// Helper operator to check flags easily
	inline bool operator&(InitFlags a, InitFlags b) {
		return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
	}

	class Context {
	public:
		Context(InitFlags flags = InitFlags::Everything) : m_flags(flags) {

			// 1. Core is always initialized if they create a context
			m_initialized = true;

			// 2. Opt-in Platform (GLFW)
			if (flags & InitFlags::Platform) {
				if (!platform::init()) m_initialized = false;
			}

			// 3. Opt-in Graphics (OpenGL)
			if (flags & InitFlags::Platform) {
				if (!platform::init()) m_initialized = false;
			}

			// 4. Opt-in Audio (MiniAudio)
			if (m_initialized && (flags & InitFlags::Audio)) {
				if (!audio::init()) m_initialized = false;
			}

			// NOTE: Graphics module must intentionally be skipped here.
			// Graphics MUST be initialized manually after Window creation.
		}

		~Context() {
			if (m_initialized) {
				// Shut down in exact reverse order
				if (m_flags & InitFlags::Audio)    audio::shutdown();
				// if (m_flags & InitFlags::Graphics) gfx::shutdown();
				if (m_flags & InitFlags::Platform) platform::shutdown();
			}
		}

		bool is_valid() const { return m_initialized; }

	private:
		bool m_initialized = false;
		InitFlags m_flags;
	};

} // namespace modulus