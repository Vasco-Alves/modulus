// Gamepads. GLFW sends no events for them, so the main window's update() reads every joystick once
// per frame (read_joysticks), and update_gamepads() turns what it read into the answers of Input's
// gamepad functions. Only read_joysticks() calls GLFW, so the tests can hand update_gamepads()
// made-up gamepads: nobody can plug a real one into CI.

#include <modulus/platform/input.hpp>
#include <modulus/core/log.hpp>
#include "glfw_internal.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <span>
#include <utility>

namespace modulus::platform {

	// How many GamepadButtons there are (Start is the last one)
	static constexpr int button_count = int(GamepadButton::Start) + 1;

	namespace {

		// One gamepad number (see Input): which joystick has it, and what its buttons did this frame
		// and the frame before
		struct Gamepad {
			int joystick = -1;                            // GLFW's joystick ID, or -1 while no gamepad has this number
			GLFWgamepadstate reading = {};                // What GLFW read this frame (the sticks and triggers come from here)
			std::array<bool, button_count> down = {};     // Which buttons are down this frame
			std::array<bool, button_count> was_down = {}; // ...and which were last frame, to tell which just went down
		};

	} // namespace

	// Indexed by gamepad number. A number without a gamepad is all zeros, which reads as untouched.
	static std::array<Gamepad, GLFW_JOYSTICK_LAST + 1> s_gamepads;

	// Sticks and triggers rarely rest exactly at 0 (and worn ones drift), so a little push counts as
	// none: up to a fifth of the way out for a stick, a tenth of the way in for a trigger. And many
	// never quite reach the end, so from 95% on counts as all the way.
	static constexpr float stick_dead_zone = 0.2f;
	static constexpr float trigger_dead_zone = 0.1f;
	static constexpr float full_push = 0.95f;

	// Takes the dead zone off how far something is pushed (from 0 to 1): the dead zone and below
	// become 0, full_push and beyond become 1, and in between is stretched to go smoothly from 0 to 1
	static float remove_dead_zone(float amount, float dead_zone) {
		if (amount <= dead_zone)
			return 0.0f;
		return std::min((amount - dead_zone) / (full_push - dead_zone), 1.0f);
	}

	// Where a stick is pushed, without the dead zone. The dead zone is a circle around the middle
	// (taking it off x and y one at a time would make the stick snap to straight lines), so only the
	// distance from the middle changes, never the direction. The result is never longer than 1:
	// a real stick's corner can read (1, 1), which would make walking diagonally faster.
	static std::pair<float, float> stick_position(float x, float y) {
		float distance = std::sqrt(x * x + y * y);
		float amount = remove_dead_zone(distance, stick_dead_zone);
		if (amount == 0.0f)
			return { 0.0f, 0.0f };
		return { x / distance * amount, y / distance * amount };
	}

	// How far a trigger is pulled, from 0 to 1. GLFW gives -1 (let go) to 1 (all the way), and exactly
	// 0 for a trigger the gamepad doesn't have, which would read as half pulled, so exactly 0 counts as
	// let go. (Real triggers never read exactly 0: XInput moves in steps of 1/127.5 around it.)
	static float trigger_amount(float glfw_value) {
		if (glfw_value == 0.0f)
			return 0.0f;
		return remove_dead_zone((glfw_value + 1.0f) / 2.0f, trigger_dead_zone);
	}

	// Whether a trigger counts as a button that's down: once pulled more than halfway, and until it's
	// let go to 40% or less. (With one line at halfway, a finger resting there would press it again and again.)
	static bool is_trigger_down(float glfw_value, bool was_down) {
		return trigger_amount(glfw_value) > (was_down ? 0.4f : 0.5f);
	}

	// GLFW's number for each button (-1 for the triggers: GLFW has those as axes, not buttons)
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
		case GamepadButton::LeftTrigger:
		case GamepadButton::RightTrigger:    break;
		}
		return -1;
	}

	// Takes a new frame's reading of a gamepad: this frame's buttons become last frame's, and the
	// reading says which are down now
	static void store(Gamepad& gamepad, const GLFWgamepadstate& reading) {
		gamepad.reading = reading;
		gamepad.was_down = gamepad.down;

		for (int i = 0; i < button_count; ++i) {
			int glfw_button = to_glfw_gamepad_button(GamepadButton(i));
			if (glfw_button >= 0)
				gamepad.down[i] = reading.buttons[glfw_button] == GLFW_PRESS;
		}

		const int left = int(GamepadButton::LeftTrigger);
		const int right = int(GamepadButton::RightTrigger);
		gamepad.down[left] = is_trigger_down(reading.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER], gamepad.was_down[left]);
		gamepad.down[right] = is_trigger_down(reading.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER], gamepad.was_down[right]);
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

	// The same, but only while the main window has the focus: like the keyboard, gamepads are for the
	// window you're using. (In the background they stay connected, but read as untouched.)
	static std::span<const Gamepad> find_gamepads_in_use(int gamepad) {
		if (!g_main_window_focused)
			return {};
		return find_gamepads(gamepad);
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

		for (const Gamepad& pad : find_gamepads_in_use(gamepad)) {
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
		for (const Gamepad& pad : find_gamepads_in_use(gamepad)) {
			if (pad.down[i] && !pad.was_down[i])
				return true;
		}
		return false;
	}

	std::pair<float, float> Input::get_gamepad_stick(GamepadStick stick, int gamepad) {
		bool left = stick == GamepadStick::Left;
		int x_axis = left ? GLFW_GAMEPAD_AXIS_LEFT_X : GLFW_GAMEPAD_AXIS_RIGHT_X;
		int y_axis = left ? GLFW_GAMEPAD_AXIS_LEFT_Y : GLFW_GAMEPAD_AXIS_RIGHT_Y;

		// With several gamepads, the stick that's pushed the most
		std::pair<float, float> position = { 0.0f, 0.0f };
		float longest = 0.0f;
		for (const Gamepad& pad : find_gamepads_in_use(gamepad)) {
			auto [x, y] = stick_position(pad.reading.axes[x_axis], pad.reading.axes[y_axis]);
			float length = std::sqrt(x * x + y * y);
			if (length > longest) {
				longest = length;
				position = { x, y };
			}
		}
		return position;
	}

	float Input::get_gamepad_trigger(GamepadTrigger trigger, int gamepad) {
		int axis = trigger == GamepadTrigger::Left ? GLFW_GAMEPAD_AXIS_LEFT_TRIGGER : GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;

		// With several gamepads, the trigger that's pulled the most
		float most = 0.0f;
		for (const Gamepad& pad : find_gamepads_in_use(gamepad))
			most = std::max(most, trigger_amount(pad.reading.axes[axis]));
		return most;
	}

} // namespace modulus::platform
