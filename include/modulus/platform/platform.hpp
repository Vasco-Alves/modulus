#pragma once

#include "window.hpp"

#include <memory>

namespace modulus::platform {

	// Factory function: Creates a specific backend window but returns the generic interface.
	// Returns nullptr if the window could not be created.
	//
	// There is no init() or shutdown(): the platform backend (GLFW) starts
	// with the first window and stops when the last window is destroyed.
	std::unique_ptr<IWindow> create_window(const WindowConfig& config);

} // namespace modulus::platform