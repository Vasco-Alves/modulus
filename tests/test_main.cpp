#include <gtest/gtest.h>

// This test proves the test system itself is working
TEST(SanityCheck, CanBuildAndRun) {
	EXPECT_TRUE(true);
}

// This test proves the assertion macros are working
TEST(SanityCheck, MathWorks) {
	int a = 2;
	int b = 3;
	EXPECT_EQ(a + b, 5);
}

// NOTE: You do NOT need a 'main()' function here.
// We linked against 'GTest::gtest_main' in CMake, which provides a standard main() 
// that runs all the tests automatically.