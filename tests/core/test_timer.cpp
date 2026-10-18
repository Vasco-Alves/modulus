#include <gtest/gtest.h>
#include <modulus/core/timer.hpp>

#include <chrono>
#include <thread>

using modulus::core::Timer;

TEST(Timer, NeverGoesBackwards) {
	Timer timer;
	float previous = timer.elapsed();
	EXPECT_GE(previous, 0.0f);

	for (int i = 0; i < 1000; ++i) {
		float now = timer.elapsed();
		ASSERT_GE(now, previous);
		previous = now;
	}
}

TEST(Timer, MeasuresTimeAndResets) {
	Timer timer;
	std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Waits at least this long
	float before_reset = timer.elapsed_ms();
	EXPECT_GE(before_reset, 19.0f);

	timer.reset();
	EXPECT_LT(timer.elapsed_ms(), before_reset);
}
