#pragma once

#include <modulus/core/log.hpp>
#include <modulus/platform/platform.hpp>

namespace modulus {

	class Context {
	public:
		Context() {
			// Initialize Core/Logging
			// (Logging is static, but maybe we init other core things here later)

			// Initialize Platform
			if (!platform::init()) {
				// In a real engine, we might throw an exception here or have a status flag
				m_initialized = false;
			} else {
				m_initialized = true;
			}
		}

		~Context() {
			if (m_initialized) {
				// Shut down systems in reverse order
				platform::shutdown();
			}
		}

		// Helper to check if everything started okay
		bool is_valid() const {
			return m_initialized;
		}

	private:
		bool m_initialized = false;
	};

} // namespace modulus