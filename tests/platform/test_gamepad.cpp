// The gamepad logic, with made-up gamepads: nobody can plug a real one into CI.
// Each frame, the main window's update() asks GLFW about every joystick (read_joysticks) and hands
// what it heard to update_gamepads(), which doesn't call GLFW. Most tests here call update_gamepads()
// themselves, with joysticks they make up, and check what Input says: they need no window and run
// everywhere. The last few open a hidden window to check the real wiring.

#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

#include <modulus/platform/input.hpp>
#include <modulus/platform/platform.hpp>

#include "platform/backends/glfw/glfw_internal.hpp" // Modulus' private GLFW backend

#include <memory>

using namespace modulus::platform;

namespace {

	// A gamepad nobody touches: buttons up, sticks in the middle, triggers let go (-1 in GLFW)
	JoystickReading untouched_gamepad() {
		JoystickReading joystick;
		joystick.is_gamepad = true;
		joystick.name = "Test Gamepad";
		joystick.state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] = -1.0f;
		joystick.state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] = -1.0f;
		return joystick;
	}

	class GamepadTest : public ::testing::Test {
	protected:
		void TearDown() override {
			forget_gamepads(); // The next test starts with none
		}

		// One frame, as the main window's update() would run it, with the made-up joysticks
		void next_frame() { update_gamepads(joysticks); }

		void plug_in(int jid) { joysticks[jid] = untouched_gamepad(); }
		void unplug(int jid) { joysticks[jid] = {}; }
		void hold(int jid, int glfw_button) { joysticks[jid].state.buttons[glfw_button] = GLFW_PRESS; }
		void release(int jid, int glfw_button) { joysticks[jid].state.buttons[glfw_button] = GLFW_RELEASE; }

		JoystickReadings joysticks = {}; // By GLFW joystick ID. Nothing plugged in yet.
	};

	std::unique_ptr<IWindow> create_hidden_window() {
		WindowConfig config;
		config.title = "modulus_tests";
		config.visible = false;
		return create_window(config);
	}

} // namespace

TEST_F(GamepadTest, NothingConnectedReadsUntouched) {
	next_frame();
	EXPECT_FALSE(Input::is_gamepad_connected());
	EXPECT_FALSE(Input::is_gamepad_connected(0));
	EXPECT_FALSE(Input::is_gamepad_down(GamepadButton::South));
	EXPECT_FALSE(Input::is_gamepad_pressed(GamepadButton::South));
}

TEST_F(GamepadTest, ConnectsAtTheNextFrame) {
	plug_in(3);
	EXPECT_FALSE(Input::is_gamepad_connected()); // Nothing changes until the next frame

	next_frame();
	EXPECT_TRUE(Input::is_gamepad_connected());
	EXPECT_TRUE(Input::is_gamepad_connected(0)); // The first gamepad is number 0, whatever GLFW calls it
	EXPECT_FALSE(Input::is_gamepad_connected(1));
}

TEST_F(GamepadTest, ButtonPressLastsOneFrame) {
	plug_in(0);
	next_frame();

	hold(0, GLFW_GAMEPAD_BUTTON_A);
	EXPECT_FALSE(Input::is_gamepad_down(GamepadButton::South)); // Nothing changes until the next frame
	next_frame();
	EXPECT_TRUE(Input::is_gamepad_down(GamepadButton::South));
	EXPECT_TRUE(Input::is_gamepad_pressed(GamepadButton::South));
	EXPECT_FALSE(Input::is_gamepad_down(GamepadButton::East));

	next_frame(); // Still held
	EXPECT_TRUE(Input::is_gamepad_down(GamepadButton::South));
	EXPECT_FALSE(Input::is_gamepad_pressed(GamepadButton::South));

	release(0, GLFW_GAMEPAD_BUTTON_A);
	next_frame();
	EXPECT_FALSE(Input::is_gamepad_down(GamepadButton::South));
	EXPECT_FALSE(Input::is_gamepad_pressed(GamepadButton::South));
}

TEST_F(GamepadTest, EachButtonIsTheRightGlfwButton) {
	struct Pair { GamepadButton button; int glfw_button; };
	const Pair pairs[] = {
		{ GamepadButton::South,           GLFW_GAMEPAD_BUTTON_A },
		{ GamepadButton::East,            GLFW_GAMEPAD_BUTTON_B },
		{ GamepadButton::West,            GLFW_GAMEPAD_BUTTON_X },
		{ GamepadButton::North,           GLFW_GAMEPAD_BUTTON_Y },
		{ GamepadButton::DpadLeft,        GLFW_GAMEPAD_BUTTON_DPAD_LEFT },
		{ GamepadButton::DpadRight,       GLFW_GAMEPAD_BUTTON_DPAD_RIGHT },
		{ GamepadButton::DpadUp,          GLFW_GAMEPAD_BUTTON_DPAD_UP },
		{ GamepadButton::DpadDown,        GLFW_GAMEPAD_BUTTON_DPAD_DOWN },
		{ GamepadButton::LeftBumper,      GLFW_GAMEPAD_BUTTON_LEFT_BUMPER },
		{ GamepadButton::RightBumper,     GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER },
		{ GamepadButton::LeftStickClick,  GLFW_GAMEPAD_BUTTON_LEFT_THUMB },
		{ GamepadButton::RightStickClick, GLFW_GAMEPAD_BUTTON_RIGHT_THUMB },
		{ GamepadButton::Back,            GLFW_GAMEPAD_BUTTON_BACK },
		{ GamepadButton::Start,           GLFW_GAMEPAD_BUTTON_START },
	};

	plug_in(0);
	next_frame();

	for (const Pair& pair : pairs) {
		hold(0, pair.glfw_button);
		next_frame();

		// That button is down, and no other
		for (int b = 0; b <= int(GamepadButton::Start); ++b) {
			GamepadButton button = GamepadButton(b);
			EXPECT_EQ(Input::is_gamepad_down(button), button == pair.button)
				<< "GLFW button " << pair.glfw_button << ", GamepadButton " << b;
		}

		release(0, pair.glfw_button);
	}

	// GLFW's Guide button (the logo in the middle) isn't one of ours
	hold(0, GLFW_GAMEPAD_BUTTON_GUIDE);
	next_frame();
	for (int b = 0; b <= int(GamepadButton::Start); ++b)
		EXPECT_FALSE(Input::is_gamepad_down(GamepadButton(b))) << "GamepadButton " << b;
}

TEST_F(GamepadTest, ButtonsHeldWhileConnectingAreDownButNotPressed) {
	plug_in(0);
	hold(0, GLFW_GAMEPAD_BUTTON_A);
	next_frame();
	EXPECT_TRUE(Input::is_gamepad_down(GamepadButton::South));
	EXPECT_FALSE(Input::is_gamepad_pressed(GamepadButton::South)); // Nobody saw it go down

	release(0, GLFW_GAMEPAD_BUTTON_A);
	next_frame();
	hold(0, GLFW_GAMEPAD_BUTTON_A);
	next_frame();
	EXPECT_TRUE(Input::is_gamepad_pressed(GamepadButton::South)); // This time it went down
}

TEST_F(GamepadTest, UnpluggingReleasesEverything) {
	plug_in(0);
	next_frame();
	hold(0, GLFW_GAMEPAD_BUTTON_A);
	next_frame();

	unplug(0);
	next_frame();
	EXPECT_FALSE(Input::is_gamepad_connected());
	EXPECT_FALSE(Input::is_gamepad_down(GamepadButton::South));
	EXPECT_FALSE(Input::is_gamepad_pressed(GamepadButton::South));
}

TEST_F(GamepadTest, NumbersSkipJoysticksThatArentGamepads) {
	// Joystick 0 is plugged in, but GLFW doesn't know which button is which on it (a flight stick,
	// say), so it isn't a gamepad. Joystick 1 is.
	joysticks[0].is_gamepad = false;
	plug_in(1);
	hold(1, GLFW_GAMEPAD_BUTTON_A);
	next_frame();

	EXPECT_TRUE(Input::is_gamepad_down(GamepadButton::South, 0)); // Joystick 1 is gamepad 0
	EXPECT_FALSE(Input::is_gamepad_connected(1));
}

TEST_F(GamepadTest, NumbersStayWhenAnotherGamepadLeaves) {
	plug_in(0);
	plug_in(1);
	hold(1, GLFW_GAMEPAD_BUTTON_B);
	next_frame(); // Joysticks 0 and 1 are gamepads 0 and 1

	unplug(0);
	next_frame();
	EXPECT_FALSE(Input::is_gamepad_connected(0));
	EXPECT_TRUE(Input::is_gamepad_down(GamepadButton::East, 1)); // Still number 1
	EXPECT_FALSE(Input::is_gamepad_pressed(GamepadButton::East, 1));

	plug_in(2);
	hold(2, GLFW_GAMEPAD_BUTTON_X);
	next_frame(); // A new gamepad takes the free number: 0
	EXPECT_TRUE(Input::is_gamepad_down(GamepadButton::West, 0));
	EXPECT_FALSE(Input::is_gamepad_pressed(GamepadButton::West, 0));
	EXPECT_TRUE(Input::is_gamepad_down(GamepadButton::East, 1));
}

TEST_F(GamepadTest, WithoutANumberAnyGamepadCounts) {
	plug_in(0);
	plug_in(1);
	next_frame();

	hold(1, GLFW_GAMEPAD_BUTTON_B); // Only on gamepad 1
	next_frame();
	EXPECT_TRUE(Input::is_gamepad_connected());
	EXPECT_TRUE(Input::is_gamepad_down(GamepadButton::East));
	EXPECT_TRUE(Input::is_gamepad_pressed(GamepadButton::East));
	EXPECT_FALSE(Input::is_gamepad_down(GamepadButton::East, 0));

	// Gamepad 0 presses it too, while gamepad 1 keeps holding it: it went down on one of them
	hold(0, GLFW_GAMEPAD_BUTTON_B);
	next_frame();
	EXPECT_TRUE(Input::is_gamepad_pressed(GamepadButton::East));
	EXPECT_FALSE(Input::is_gamepad_pressed(GamepadButton::East, 1));
}

TEST_F(GamepadTest, NumbersOutOfRangeReadNothing) {
	plug_in(0);
	next_frame();
	hold(0, GLFW_GAMEPAD_BUTTON_A);
	next_frame();

	for (int number : { -2, 16, 99 }) {
		EXPECT_FALSE(Input::is_gamepad_connected(number)) << number;
		EXPECT_FALSE(Input::is_gamepad_down(GamepadButton::South, number)) << number;
		EXPECT_FALSE(Input::is_gamepad_pressed(GamepadButton::South, number)) << number;
	}
}

// ---- The real wiring, with a hidden window ----

TEST(GamepadWindow, ReadingEveryJoystickRaisesNoGlfwErrors) {
	auto window = create_hidden_window();
	if (!window)
		GTEST_SKIP() << "Can't create a window here (no display?)";

	glfwGetError(nullptr); // Forget anything that happened while the window was created
	window->update();      // Asks GLFW about all 16 joysticks (and reads any real ones plugged in)
	window->update();
	EXPECT_EQ(glfwGetError(nullptr), GLFW_NO_ERROR);
}

TEST(GamepadWindow, ClosingTheMainWindowForgetsGamepads) {
	auto window = create_hidden_window();
	if (!window)
		GTEST_SKIP() << "Can't create a window here (no display?)";

	JoystickReadings joysticks = {};
	joysticks[0] = untouched_gamepad();
	update_gamepads(joysticks); // As the window's update() would, with a made-up gamepad
	EXPECT_TRUE(Input::is_gamepad_connected());

	window.reset();
	EXPECT_FALSE(Input::is_gamepad_connected());
}
