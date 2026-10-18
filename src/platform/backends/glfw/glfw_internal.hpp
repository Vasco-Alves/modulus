#pragma once

#include <GLFW/glfw3.h>

namespace modulus::platform {
	// A global pointer to the main window handle.
	// Defined in platform_glfw.cpp, used in input_glfw.cpp
	extern GLFWwindow* g_main_window_handle;
	
	// Global scroll accumulator
	extern float g_scroll_delta_y;

	// Which keys went down during the current frame (indexed by GLFW key code).
	// Filled by the key callback, cleared at the start of every frame.
	extern bool g_keys_pressed[GLFW_KEY_LAST + 1];
}
