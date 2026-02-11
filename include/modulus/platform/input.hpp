#pragma once

#include <utility>

namespace modulus::platform {

	enum class Key {
		Unknown = 0,

		// Controls
		Escape, Enter, Space, Tab, Backspace,

		// Arrows
		Left, Right, Up, Down,

		// Letters
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	};

	enum class MouseButton {
		Left, Right, Middle
	};

	// The Interface for checking input state
	// Note: We make these static functions for easy access: Input::is_key_down(...)
	struct Input {
		static bool is_key_down(Key key);
		static bool is_mouse_down(MouseButton button);

		static std::pair<float, float> get_mouse_position();
		static float get_mouse_scroll(); // Returns scroll delta since last frame
	};

} // namespace modulus::platform