#pragma once

#include <string>
#include <utility>

namespace modulus::platform {

	// Configuration struct to create a window
	struct WindowConfig {
		std::string title = "Modulus App";
		int width = 1280;
		int height = 720;
		bool vsync = true;
	};

	// The Abstract Interface (User sees this)
	class IWindow {
	public:
		virtual ~IWindow() = default;

		// Core loop functions
		virtual bool update() = 0;       // Polls events. Returns false if window should close.
		virtual void close() = 0;        // Manually signal close

		// Getters
		virtual int width() const = 0;
		virtual int height() const = 0;
		virtual std::pair<int, int> get_size() const = 0;

		// CRITICAL: Needed for the Graphics module later
		// Returns the raw GLFWwindow* (cast to void*)
		virtual void* get_native_handle() const = 0;
	};

} // namespace modulus::platform