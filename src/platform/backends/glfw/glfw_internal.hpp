#pragma once

#include <GLFW/glfw3.h>

#include <string>

namespace modulus::platform {
	// A global pointer to the main window handle.
	// Defined in platform_glfw.cpp, used in input_glfw.cpp
	extern GLFWwindow* g_main_window_handle;
	
	// Global scroll accumulator
	extern float g_scroll_delta_y;

	// Which keys went down during the current frame (indexed by GLFW key code).
	// Filled by the key callback, cleared at the start of every frame.
	extern bool g_keys_pressed[GLFW_KEY_LAST + 1];

	// Which keys went down or repeated during the current frame (a held key repeats, at the speed
	// set in the keyboard settings). Filled by the key callback, cleared at the start of every frame.
	extern bool g_keys_repeated[GLFW_KEY_LAST + 1];

	// The same for mouse buttons (indexed by GLFW button code)
	extern bool g_mouse_buttons_pressed[GLFW_MOUSE_BUTTON_LAST + 1];

	// The text typed during the current frame, as UTF-8.
	// Filled by the character callback, cleared at the start of every frame.
	extern std::string g_typed_text;

	// Logs an error from GLFW. It's GLFW's error callback (platform_glfw.cpp).
	void log_glfw_error(int error, const char* description);
}
