#pragma once

#include <chrono>

namespace modulus::core {

	// Uses steady_clock: it only ever moves forward. (The wall clock can jump when the
	// system time changes, and high_resolution_clock is the wall clock on some compilers.)
	class Timer {
	public:
		Timer() {
			reset();
		}

		// Restarts the timer from zero
		void reset() {
			m_start = std::chrono::steady_clock::now();
		}

		// Returns the elapsed time in seconds
		float elapsed() const {
			return std::chrono::duration<float>(std::chrono::steady_clock::now() - m_start).count();
		}

		// Returns the elapsed time in milliseconds
		float elapsed_ms() const {
			return elapsed() * 1000.0f;
		}

	private:
		std::chrono::steady_clock::time_point m_start;
	};

} // namespace modulus::core