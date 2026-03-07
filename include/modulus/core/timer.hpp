#pragma once

#include <chrono>

namespace modulus::core {

	class Timer {
	public:
		Timer() {
			reset();
		}

		// Restarts the timer from zero
		void reset() {
			m_start = std::chrono::high_resolution_clock::now();
		}

		// Returns the elapsed time in seconds
		float elapsed() const {
			return std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - m_start).count();
		}

		// Returns the elapsed time in milliseconds
		float elapsed_ms() const {
			return elapsed() * 1000.0f;
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
	};

} // namespace modulus::core