#pragma once

#include <GLFW/glfw3.h>

namespace modulus::platform {
	// A global pointer to the main window handle.
	// Defined in platform_glfw.cpp, used in input_glfw.cpp
	extern GLFWwindow* g_mainWindowHandle;
}
