#pragma once

#include <GLFW/glfw3.h>

#include <array>
#include <string>

namespace modulus::platform {
	// A global pointer to the main window handle.
	// Defined in platform_glfw.cpp, used in input_glfw.cpp
	extern GLFWwindow* g_main_window_handle;

	// Whether the main window has the keyboard focus. Gamepads read as untouched while it doesn't.
	// Kept up to date by the main window's focus callback (platform_glfw.cpp).
	extern bool g_main_window_focused;

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

	// ---- Gamepads (gamepad_glfw.cpp) ----

	// What GLFW said about one joystick in one frame
	struct JoystickReading {
		bool is_gamepad = false;     // Plugged in, and GLFW knows which button is which on it (it has a "gamepad mapping")
		GLFWgamepadstate state = {}; // Its buttons and axes, as GLFW gives them (axes from -1 to 1)
		std::string name;            // e.g. "PS4 Controller". Only read when it's about to get a gamepad number.
	};

	// Every joystick GLFW can have, by GLFW joystick ID
	using JoystickReadings = std::array<JoystickReading, GLFW_JOYSTICK_LAST + 1>;

	// Asks GLFW about every joystick. Called by the main window's update() (GLFW must be running).
	JoystickReadings read_joysticks();

	// A new frame for the gamepads: unplugged ones free their number, new ones get the lowest free
	// one, and last frame's buttons are kept to tell which just went down. It doesn't call GLFW, so
	// the tests can hand it made-up joysticks. Only the main window's update() calls it.
	void update_gamepads(const JoystickReadings& joysticks);

	// Forgets every gamepad. Called when the main window closes: without one, Input reads none.
	void forget_gamepads();
}
