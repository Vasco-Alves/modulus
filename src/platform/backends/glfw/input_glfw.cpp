#include <modulus/platform/input.hpp>
#include "glfw_internal.hpp"

namespace modulus::platform {

	// Helper function to convert modulus::platform::Key to GLFW key codes
	static int to_glfw_key(Key key) {
		switch (key) {
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

			// Controls
		case Key::Space: return GLFW_KEY_SPACE;
		case Key::Escape: return GLFW_KEY_ESCAPE;
		case Key::Enter: return GLFW_KEY_ENTER;

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

} // namespace modulus::platform