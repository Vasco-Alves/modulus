#pragma once

#include <string>
#include <utility>

namespace modulus::platform {

	enum class Key {
		Unknown = 0,

		// Controls
		Escape, Enter, Space, Tab, Backspace,

		// Arrows
		Left, Right, Up, Down,

		// Function keys
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

		// Numbers (top row)
		D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,

		// Letters
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

		// Modifiers
		LeftShift, RightShift, LeftControl, RightControl, LeftAlt, RightAlt,

		// Editing and navigation
		Insert, Delete, Home, End, PageUp, PageDown,
	};

	enum class MouseButton {
		Left, Right, Middle
	};

	// The Interface for checking input state. It reads from the first window created (the main
	// window): keys, mouse buttons, scroll and typing change once per frame, when that window's
	// update() is called, while the mouse position and the clipboard are read the moment you ask.
	// With several windows, update the others right after the main one, before reading Input: every
	// update() collects the events of all windows, and the main window's next update() forgets the
	// presses and typing collected since its last one.
	// Note: We make these static functions for easy access: Input::is_key_down(...)
	struct Input {
		static bool is_key_down(Key key);     // True every frame while the key is held (e.g. movement)
		static bool is_key_pressed(Key key);  // True only in the frame the key went down (e.g. jump)

		// True in the frame the key went down, and again in each frame where the held key repeats, as
		// set in the keyboard settings (e.g. Backspace in a text field). Several repeats in one frame count once.
		static bool is_key_repeated(Key key);

		static bool is_mouse_down(MouseButton button);    // True every frame while the button is held (e.g. dragging)
		static bool is_mouse_pressed(MouseButton button); // True only in the frame the button went down (e.g. clicking)

		static std::pair<float, float> get_mouse_position();
		static float get_mouse_scroll(); // Returns scroll delta since last frame

		// --- Typing text (e.g. a name) ---

		// The text typed during this frame, as UTF-8 (like all text in Modulus): usually nothing, or one
		// character. It follows the keyboard layout, Shift and accent keys, and a held key types again
		// each time it repeats. Enter, Backspace, Tab and Escape aren't text: read them as keys.
		static std::string get_typed_text();

		// The clipboard, for copying and pasting text (e.g. Ctrl+V into a name field). Read it only when
		// pasting, not every frame: on Linux, reading it means asking the program that holds it.
		static std::string get_clipboard_text();                 // "" if it holds no text (e.g. a picture)
		static void set_clipboard_text(const std::string& text); // Puts a copy of the text on it
	};

} // namespace modulus::platform