#pragma once

#include "window.hpp"
#include <memory> // for std::unique_ptr

namespace modulus::platform {

	// Initializes the platform subsystem (e.g. glfwInit)
	bool init();

	// Shuts down the subsystem (e.g. glfwTerminate)
	void shutdown();

	// Factory function: Creates a specific backend window (GLFW) 
	// but returns the generic interface.
	std::unique_ptr<IWindow> create_window(const WindowConfig& config);

} // namespace modulus::platform