// The window's frame time. Each test opens a hidden window, and skips itself without a display.

#include <gtest/gtest.h>

#include <modulus/platform/platform.hpp>

#include <chrono>
#include <memory>
#include <thread>

using namespace modulus::platform;
using namespace std::chrono_literals;

namespace {

	std::unique_ptr<IWindow> create_hidden_window() {
		WindowConfig config;
		config.title = "modulus_tests";
		config.visible = false;
		return create_window(config);
	}

} // namespace

TEST(Window, FrameTimeIsZeroUntilTheSecondUpdate) {
	auto window = create_hidden_window();
	if (!window)
		GTEST_SKIP() << "Can't create a window here (no display?)";

	EXPECT_EQ(window->get_frame_time(), 0.0f);
	window->update();
	EXPECT_EQ(window->get_frame_time(), 0.0f); // There was no frame before this one to measure
}

TEST(Window, FrameTimeIsTheTimeBetweenTheLastTwoUpdates) {
	auto window = create_hidden_window();
	if (!window)
		GTEST_SKIP() << "Can't create a window here (no display?)";

	window->update();
	std::this_thread::sleep_for(50ms); // A slow frame
	window->update();

	// At least the 50 ms (give or take rounding). A busy computer can oversleep, so allow much more.
	float frame_time = window->get_frame_time();
	EXPECT_GT(frame_time, 0.049f);
	EXPECT_LT(frame_time, 1.0f);

	// It's how long the last frame took, not a stopwatch: it stays the same until the next update()
	std::this_thread::sleep_for(20ms);
	EXPECT_EQ(window->get_frame_time(), frame_time);

	window->update(); // This frame took those 20 ms
	EXPECT_NE(window->get_frame_time(), frame_time);
	EXPECT_GT(window->get_frame_time(), 0.019f);
}
