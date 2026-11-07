// Tests the frame-by-frame input logic by sending the window fake events.
// GLFW can't inject real events, but setting a callback returns the previous one, which is
// Modulus' own handler. Calling it is exactly what GLFW does when a real key goes down.
// The InputTest tests open a hidden window, and skip themselves without a display.

#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

#include <modulus/platform/input.hpp>
#include <modulus/platform/platform.hpp>

#include <memory>
#include <string>

using namespace modulus::platform;

namespace {

	class InputTest : public ::testing::Test {
	protected:
		void SetUp() override {
			open_window();
			if (!window)
				GTEST_SKIP() << "Can't create a window here (no display?)";

			window->update(); // Start a frame
		}

		// Opens a hidden window. With no other window open, it's the main window, which Input reads.
		void open_window() {
			WindowConfig config;
			config.title = "modulus_tests";
			config.visible = false;

			window = create_window(config);
			handle = window ? static_cast<GLFWwindow*>(window->get_native_handle()) : nullptr;
		}

		void send_key(int glfw_key, int action) {
			GLFWkeyfun handler = glfwSetKeyCallback(handle, nullptr);
			glfwSetKeyCallback(handle, handler);
			ASSERT_NE(handler, nullptr);
			handler(handle, glfw_key, 0, action, 0);
		}

		void press_key(int glfw_key) { send_key(glfw_key, GLFW_PRESS); }
		void repeat_key(int glfw_key) { send_key(glfw_key, GLFW_REPEAT); } // What GLFW sends while a key is held

		// Types one character, given as its Unicode number (what GLFW passes on after the keyboard layout)
		void type(unsigned int codepoint) {
			GLFWcharfun handler = glfwSetCharCallback(handle, nullptr);
			glfwSetCharCallback(handle, handler);
			ASSERT_NE(handler, nullptr);
			handler(handle, codepoint);
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

TEST_F(InputTest, HeldKeyRepeats) {
	press_key(GLFW_KEY_BACKSPACE);
	EXPECT_TRUE(Input::is_key_pressed(Key::Backspace));
	EXPECT_TRUE(Input::is_key_repeated(Key::Backspace)); // Going down counts too
	EXPECT_FALSE(Input::is_key_repeated(Key::Enter));

	window->update();
	repeat_key(GLFW_KEY_BACKSPACE); // Held long enough to repeat
	EXPECT_TRUE(Input::is_key_repeated(Key::Backspace));
	EXPECT_FALSE(Input::is_key_pressed(Key::Backspace)); // It didn't go down again

	window->update(); // A frame without a repeat
	EXPECT_FALSE(Input::is_key_repeated(Key::Backspace));
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

// (The expected text is written byte by byte, as UTF-8 stores it)
TEST_F(InputTest, TypedTextIsUtf8) {
	type('H');     // 1 byte
	type(0xE9);    // e with an acute accent: 2 bytes
	type(0x20AC);  // The euro sign: 3 bytes
	type(0x1F600); // A smiling face: 4 bytes
	EXPECT_EQ(Input::get_typed_text(), "H\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80");
}

TEST_F(InputTest, TypedTextLastsOneFrame) {
	type('h');
	type('i');
	EXPECT_EQ(Input::get_typed_text(), "hi");
	EXPECT_EQ(Input::get_typed_text(), "hi"); // Reading it doesn't use it up

	window->update(); // The next frame
	EXPECT_EQ(Input::get_typed_text(), "");
}

TEST_F(InputTest, CodePointsThatArentCharactersAreSkipped) {
	type(0xD800);   // The first half of a UTF-16 pair
	type(0xDFFF);   // A second half
	type(0x110000); // Past the last character Unicode has
	type('a');
	EXPECT_EQ(Input::get_typed_text(), "a");
}

TEST_F(InputTest, ClipboardHoldsText) {
	// This is the computer's real clipboard: the text on it is put back afterwards, but anything else
	// (like a picture) is lost. On Linux the text then belongs to this test, so it's gone once the
	// test ends, unless a clipboard manager keeps it. (Under the virtual display of the CI, the
	// clipboard starts empty.)
	std::string before = Input::get_clipboard_text();

	Input::set_clipboard_text("Ol\xC3\xA1, clipboard"); // In UTF-8, with an a with an acute accent
	EXPECT_EQ(Input::get_clipboard_text(), "Ol\xC3\xA1, clipboard");

	Input::set_clipboard_text(before);
}

TEST_F(InputTest, ReadingTheClipboardPutsTheErrorCallbackBack) {
	// The clipboard functions swap GLFW's error callback for a moment: the usual one must come back
	GLFWerrorfun usual = glfwSetErrorCallback(nullptr);
	glfwSetErrorCallback(usual);

	Input::get_clipboard_text();
	EXPECT_EQ(glfwSetErrorCallback(usual), usual);
}

TEST_F(InputTest, AClipboardWithoutTextLogsNoError) {
	// Under the virtual display of the CI, a newly started GLFW finds no text on the clipboard. GLFW
	// reports that as an error, but for Input it's normal: nothing to paste. (Where the clipboard
	// holds text, this only checks that reading it logs no error.)
	testing::internal::CaptureStderr();
	Input::get_clipboard_text();
	std::string errors = testing::internal::GetCapturedStderr();

	EXPECT_EQ(errors.find("[ERROR]"), std::string::npos) << errors;
}

TEST_F(InputTest, ANewMainWindowStartsWithNothingPressed) {
	press_key(GLFW_KEY_SPACE);
	repeat_key(GLFW_KEY_BACKSPACE);
	click(GLFW_MOUSE_BUTTON_LEFT);
	scroll(1.0);
	type('a');

	// Close the main window, then open another one, which becomes the new main window
	window.reset();
	open_window();
	ASSERT_NE(window, nullptr);

	// Nothing has happened in it yet, even before its first update()
	EXPECT_FALSE(Input::is_key_pressed(Key::Space));
	EXPECT_FALSE(Input::is_key_repeated(Key::Backspace));
	EXPECT_FALSE(Input::is_mouse_pressed(MouseButton::Left));
	EXPECT_EQ(Input::get_mouse_scroll(), 0.0f);
	EXPECT_EQ(Input::get_typed_text(), "");
}

// GLFW only runs while a window is open. Without one, Input must not call it: GLFW would report an
// error. (glfwGetError() works without GLFW running, and returns the last error, if any. The
// clipboard functions read GLFW's error themselves and log it, so for them the log is checked.)
TEST(InputWithoutAWindow, ReadsNothingAndCallsNoGlfw) {
	glfwGetError(nullptr); // Forget any earlier error

	EXPECT_FALSE(Input::is_key_repeated(Key::Space));
	EXPECT_EQ(Input::get_typed_text(), "");

	testing::internal::CaptureStderr();
	EXPECT_EQ(Input::get_clipboard_text(), "");
	Input::set_clipboard_text("ignored");
	std::string errors = testing::internal::GetCapturedStderr();
	EXPECT_EQ(errors.find("[ERROR]"), std::string::npos) << errors;

	EXPECT_FALSE(Input::is_gamepad_connected());
	EXPECT_FALSE(Input::is_gamepad_down(GamepadButton::South));
	EXPECT_FALSE(Input::is_gamepad_pressed(GamepadButton::South));
	EXPECT_EQ(Input::get_gamepad_stick(GamepadStick::Left), std::make_pair(0.0f, 0.0f));
	EXPECT_EQ(Input::get_gamepad_trigger(GamepadTrigger::Left), 0.0f);

	EXPECT_EQ(glfwGetError(nullptr), GLFW_NO_ERROR);
}
