#include <modulus/platform/input.hpp>
#include "glfw_internal.hpp"

namespace modulus::platform {

	// Helper function to convert modulus::platform::Key to GLFW key codes
	static int to_glfw_key(Key key) {
		switch (key) {
			// Controls
		case Key::Escape: return GLFW_KEY_ESCAPE;
		case Key::Space: return GLFW_KEY_SPACE;
		case Key::Enter: return GLFW_KEY_ENTER;

			// Letters
		case Key::A: return GLFW_KEY_A;
		case Key::B: return GLFW_KEY_B;
		case Key::C: return GLFW_KEY_C;
		case Key::D: return GLFW_KEY_D;
		case Key::E: return GLFW_KEY_E;
		case Key::F: return GLFW_KEY_F;
		case Key::G: return GLFW_KEY_G;
		case Key::H: return GLFW_KEY_H;
		case Key::I: return GLFW_KEY_I;
		case Key::J: return GLFW_KEY_J;
		case Key::K: return GLFW_KEY_K;
		case Key::L: return GLFW_KEY_L;
		case Key::M: return GLFW_KEY_M;
		case Key::N: return GLFW_KEY_N;
		case Key::O: return GLFW_KEY_O;
		case Key::P: return GLFW_KEY_P;
		case Key::Q: return GLFW_KEY_Q;
		case Key::R: return GLFW_KEY_R;
		case Key::S: return GLFW_KEY_S;
		case Key::T: return GLFW_KEY_T;
		case Key::U: return GLFW_KEY_U;
		case Key::V: return GLFW_KEY_V;
		case Key::W: return GLFW_KEY_W;
		case Key::X: return GLFW_KEY_X;
		case Key::Y: return GLFW_KEY_Y;
		case Key::Z: return GLFW_KEY_Z;

		case Key::F1: return GLFW_KEY_F1;
		case Key::F2: return GLFW_KEY_F2;
		case Key::F3: return GLFW_KEY_F3;
		case Key::F4: return GLFW_KEY_F4;
		case Key::F5: return GLFW_KEY_F5;
		case Key::F6: return GLFW_KEY_F6;
		case Key::F7: return GLFW_KEY_F7;
		case Key::F8: return GLFW_KEY_F8;
		case Key::F9: return GLFW_KEY_F9;
		case Key::F10: return GLFW_KEY_F10;
		case Key::F11: return GLFW_KEY_F11;
		case Key::F12: return GLFW_KEY_F12;

		case Key::D0: return GLFW_KEY_0;
		case Key::D1: return GLFW_KEY_1;
		case Key::D2: return GLFW_KEY_2;
		case Key::D3: return GLFW_KEY_3;
		case Key::D4: return GLFW_KEY_4;
		case Key::D5: return GLFW_KEY_5;
		case Key::D6: return GLFW_KEY_6;
		case Key::D7: return GLFW_KEY_7;
		case Key::D8: return GLFW_KEY_8;
		case Key::D9: return GLFW_KEY_9;

		case Key::Unknown:
		default: return GLFW_KEY_UNKNOWN;
		}
	}

	bool Input::is_key_down(Key key) {
		if (!g_mainWindowHandle)
			return false;

		int glfw_key = to_glfw_key(key);
		if (glfw_key == GLFW_KEY_UNKNOWN)
			return false;

		// The actual GLFW call
		int state = glfwGetKey(g_mainWindowHandle, glfw_key);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::is_mouse_down(MouseButton button) {
		if (!g_mainWindowHandle)
			return false;

		int glfw_button;
		switch (button) {
		case MouseButton::Left:
			glfw_button = GLFW_MOUSE_BUTTON_LEFT;
			break;

		case MouseButton::Right:
			glfw_button = GLFW_MOUSE_BUTTON_RIGHT;
			break;

		case MouseButton::Middle:
			glfw_button = GLFW_MOUSE_BUTTON_MIDDLE;
			break;

		default:
			return false;
		}

		// The actual GLFW call
		int state = glfwGetMouseButton(g_mainWindowHandle, glfw_button);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> Input::get_mouse_position() {
		if (!g_mainWindowHandle) return { 0.0f, 0.0f };

		double x, y;
		glfwGetCursorPos(g_mainWindowHandle, &x, &y);
		return { (float)x, (float)y };
	}

	float Input::get_mouse_scroll() {
		return g_scrollDeltaY;
	}

} // namespace modulus::platform
