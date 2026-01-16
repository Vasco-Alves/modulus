#pragma once

namespace modulus::platform {

	enum class Key {
		Unknown = 0,

		// Letters
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

		// Controls
		Space,
		Escape,
		Enter,

		// Arrows
		Right, Left, Down, Up
	};

	enum class MouseButton {
		Left,
		Right,
		Middle
	};

	// The Interface for checking input state
	// Note: We make these static functions for easy access: Input::is_key_down(...)
	struct Input {
		static bool is_key_down(Key key);
		static bool is_mouse_down(MouseButton button);

		// Later we can add:
		// static std::pair<float, float> get_mouse_pos();
	};

} // namespace modulus::platform