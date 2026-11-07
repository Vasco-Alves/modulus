// Gamepads. GLFW sends no events for them, so the main window's update() reads every joystick once
// per frame (read_joysticks), and update_gamepads() turns what it read into the answers of Input's
// gamepad functions. Only read_joysticks() calls GLFW, so the tests can hand update_gamepads()
// made-up gamepads: nobody can plug a real one into CI.

#include <modulus/platform/input.hpp>
#include <modulus/core/log.hpp>
#include "glfw_internal.hpp"

#include <array>
#include <span>

namespace modulus::platform {

	// How many GamepadButtons there are (Start is the last one)
	static constexpr int button_count = int(GamepadButton::Start) + 1;

	namespace {

		// One gamepad number (see Input): which joystick has it, and what its buttons did this frame
		// and the frame before
		struct Gamepad {
			int joystick = -1;                            // GLFW's joystick ID, or -1 while no gamepad has this number
			std::array<bool, button_count> down = {};     // Which buttons are down this frame
			std::array<bool, button_count> was_down = {}; // ...and which were last frame, to tell which just went down
		};

	} // namespace

	// Indexed by gamepad number. A number without a gamepad is all zeros, which reads as untouched.
	static std::array<Gamepad, GLFW_JOYSTICK_LAST + 1> s_gamepads;

	// GLFW's number for each button
	static int to_glfw_gamepad_button(GamepadButton button) {
		switch (button) {
		case GamepadButton::South:           return GLFW_GAMEPAD_BUTTON_A;
		case GamepadButton::East:            return GLFW_GAMEPAD_BUTTON_B;
		case GamepadButton::West:            return GLFW_GAMEPAD_BUTTON_X;
		case GamepadButton::North:           return GLFW_GAMEPAD_BUTTON_Y;
		case GamepadButton::DpadLeft:        return GLFW_GAMEPAD_BUTTON_DPAD_LEFT;
		case GamepadButton::DpadRight:       return GLFW_GAMEPAD_BUTTON_DPAD_RIGHT;
		case GamepadButton::DpadUp:          return GLFW_GAMEPAD_BUTTON_DPAD_UP;
		case GamepadButton::DpadDown:        return GLFW_GAMEPAD_BUTTON_DPAD_DOWN;
		case GamepadButton::LeftBumper:      return GLFW_GAMEPAD_BUTTON_LEFT_BUMPER;
		case GamepadButton::RightBumper:     return GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER;
		case GamepadButton::LeftStickClick:  return GLFW_GAMEPAD_BUTTON_LEFT_THUMB;
		case GamepadButton::RightStickClick: return GLFW_GAMEPAD_BUTTON_RIGHT_THUMB;
		case GamepadButton::Back:            return GLFW_GAMEPAD_BUTTON_BACK;
		case GamepadButton::Start:           return GLFW_GAMEPAD_BUTTON_START;
		}
		return -1;
	}

	// Takes a new frame's reading of a gamepad: this frame's buttons become last frame's, and the
	// reading says which are down now
	static void store(Gamepad& gamepad, const GLFWgamepadstate& reading) {
		gamepad.was_down = gamepad.down;

		for (int i = 0; i < button_count; ++i)
			gamepad.down[i] = reading.buttons[to_glfw_gamepad_button(GamepadButton(i))] == GLFW_PRESS;
	}

	// Whether a GLFW joystick has a gamepad number already
	static bool has_number(int joystick) {
		for (const Gamepad& gamepad : s_gamepads) {
			if (gamepad.joystick == joystick)
				return true;
		}
		return false;
	}

	// =========================================================================
	// Asking GLFW (the only part that calls it)
	// =========================================================================

	JoystickReadings read_joysticks() {
		JoystickReadings joysticks;

		for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid) {
			JoystickReading& joystick = joysticks[jid];

			// GLFW reads the joystick right now (not during glfwPollEvents). It says no when nothing is
			// plugged in here, or when it doesn't know which button is which on this joystick.
			joystick.is_gamepad = glfwGetGamepadState(jid, &joystick.state) == GLFW_TRUE;

			// The name is only for the log, when the gamepad gets its number. (Asking for it makes
			// GLFW read the device again, so not every frame.)
			if (joystick.is_gamepad && !has_number(jid)) {
				const char* name = glfwGetGamepadName(jid);
				joystick.name = name ? name : "no name";
			}
		}

		return joysticks;
	}

	// =========================================================================
	// Keeping track of the gamepads (no GLFW calls: the tests drive this part)
	// =========================================================================

	void update_gamepads(const JoystickReadings& joysticks) {
		// A gamepad that was unplugged frees its number
		for (int number = 0; number < int(s_gamepads.size()); ++number) {
			Gamepad& gamepad = s_gamepads[number];
			if (gamepad.joystick != -1 && !joysticks[gamepad.joystick].is_gamepad) {
				MOD_INFO("Gamepad {0} disconnected", number);
				gamepad = {};
			}
		}

		// The others take this frame's reading
		for (Gamepad& gamepad : s_gamepads) {
			if (gamepad.joystick != -1)
				store(gamepad, joysticks[gamepad.joystick].state);
		}

		// A new gamepad gets the lowest free number, and keeps it until it's unplugged (so unplugging
		// player 1's gamepad doesn't make player 2's gamepad number 0). There always is a free one:
		// there are as many numbers as joysticks.
		for (int joystick = 0; joystick < int(joysticks.size()); ++joystick) {
			if (!joysticks[joystick].is_gamepad || has_number(joystick))
				continue;

			for (int number = 0; number < int(s_gamepads.size()); ++number) {
				Gamepad& gamepad = s_gamepads[number];
				if (gamepad.joystick != -1)
					continue;

				gamepad.joystick = joystick;
				store(gamepad, joysticks[joystick].state);
				gamepad.was_down = gamepad.down; // Buttons it already holds weren't seen going down: not "pressed"
				MOD_INFO("Gamepad {0} connected: {1}", number, joysticks[joystick].name);
				break;
			}
		}
	}

	void forget_gamepads() {
		s_gamepads = {};
	}

	// =========================================================================
	// Input's gamepad functions
	// =========================================================================

	// The gamepads a question is about: every number for any_gamepad, otherwise just that one
	// (none for a number no gamepad can have)
	static std::span<const Gamepad> find_gamepads(int gamepad) {
		if (gamepad == Input::any_gamepad)
			return s_gamepads;
		if (gamepad < 0 || gamepad >= int(s_gamepads.size()))
			return {};
		return std::span<const Gamepad>(s_gamepads).subspan(size_t(gamepad), 1);
	}

	// A button's place in Gamepad::down (-1 for a value that isn't a GamepadButton, only possible with a cast)
	static int button_index(GamepadButton button) {
		int index = int(button);
		return index >= 0 && index < button_count ? index : -1;
	}

	bool Input::is_gamepad_connected(int gamepad) {
		for (const Gamepad& pad : find_gamepads(gamepad)) {
			if (pad.joystick != -1)
				return true;
		}
		return false;
	}

	bool Input::is_gamepad_down(GamepadButton button, int gamepad) {
		int i = button_index(button);
		if (i < 0)
			return false;

		for (const Gamepad& pad : find_gamepads(gamepad)) {
			if (pad.down[i])
				return true;
		}
		return false;
	}

	bool Input::is_gamepad_pressed(GamepadButton button, int gamepad) {
		int i = button_index(button);
		if (i < 0)
			return false;

		// Gamepads send no events, so "went down" means: down this frame, but not last frame
		for (const Gamepad& pad : find_gamepads(gamepad)) {
			if (pad.down[i] && !pad.was_down[i])
				return true;
		}
		return false;
	}

} // namespace modulus::platform
