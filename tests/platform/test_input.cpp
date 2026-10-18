// Tests the frame-by-frame input logic by sending the window fake events.
// GLFW can't inject real events, but setting a callback returns the previous one, which is
// Modulus' own handler. Calling it is exactly what GLFW does when a real key goes down.

#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

#include <modulus/platform/input.hpp>
#include <modulus/platform/platform.hpp>

#include <memory>

using namespace modulus::platform;

namespace {

	class InputTest : public ::testing::Test {
	protected:
		void SetUp() override {
			WindowConfig config;
			config.title = "modulus_tests";
			config.visible = false;

			window = create_window(config);
			if (!window)
				GTEST_SKIP() << "Can't create a window here (no display?)";

			handle = static_cast<GLFWwindow*>(window->get_native_handle());
			window->update(); // Start a frame
		}

		void press_key(int glfw_key) {
			GLFWkeyfun handler = glfwSetKeyCallback(handle, nullptr);
			glfwSetKeyCallback(handle, handler);
			ASSERT_NE(handler, nullptr);
			handler(handle, glfw_key, 0, GLFW_PRESS, 0);
		}

		void click(int glfw_button) {
			GLFWmousebuttonfun handler = glfwSetMouseButtonCallback(handle, nullptr);
			glfwSetMouseButtonCallback(handle, handler);
			ASSERT_NE(handler, nullptr);
			handler(handle, glfw_button, GLFW_PRESS, 0);
		}

		void scroll(double amount) {
			GLFWscrollfun handler = glfwSetScrollCallback(handle, nullptr);
			glfwSetScrollCallback(handle, handler);
			ASSERT_NE(handler, nullptr);
			handler(handle, 0.0, amount);
		}

		std::unique_ptr<IWindow> window;
		GLFWwindow* handle = nullptr;
	};

} // namespace

TEST_F(InputTest, KeyPressLastsOneFrame) {
	press_key(GLFW_KEY_SPACE);
	EXPECT_TRUE(Input::is_key_pressed(Key::Space));
	EXPECT_FALSE(Input::is_key_pressed(Key::Enter));

	window->update(); // The next frame
	EXPECT_FALSE(Input::is_key_pressed(Key::Space));
}

TEST_F(InputTest, ModifierAndNavigationKeys) {
	press_key(GLFW_KEY_LEFT_SHIFT);
	press_key(GLFW_KEY_RIGHT_CONTROL);
	press_key(GLFW_KEY_DELETE);

	EXPECT_TRUE(Input::is_key_pressed(Key::LeftShift));
	EXPECT_TRUE(Input::is_key_pressed(Key::RightControl));
	EXPECT_TRUE(Input::is_key_pressed(Key::Delete));
	EXPECT_FALSE(Input::is_key_pressed(Key::LeftAlt));
}

TEST_F(InputTest, MouseClickLastsOneFrame) {
	click(GLFW_MOUSE_BUTTON_LEFT);
	EXPECT_TRUE(Input::is_mouse_pressed(MouseButton::Left));
	EXPECT_FALSE(Input::is_mouse_pressed(MouseButton::Right));

	window->update();
	EXPECT_FALSE(Input::is_mouse_pressed(MouseButton::Left));
}

TEST_F(InputTest, ScrollAddsUpWithinAFrame) {
	scroll(1.0);
	scroll(2.0);
	EXPECT_FLOAT_EQ(Input::get_mouse_scroll(), 3.0f);

	window->update();
	EXPECT_FLOAT_EQ(Input::get_mouse_scroll(), 0.0f);
}
