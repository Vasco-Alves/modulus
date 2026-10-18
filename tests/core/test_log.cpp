#include <gtest/gtest.h>
#include <modulus/core/log.hpp>

#include <string>

// CaptureStdout() and CaptureStderr() collect everything the program prints.
// (They are in GoogleTest's "internal" namespace, but they are stable and widely used.)

TEST(Log, FormatsTheMessage) {
	testing::internal::CaptureStdout();
	MOD_INFO("{} + {} = {}", 2, 3, 5);
	std::string output = testing::internal::GetCapturedStdout();

	EXPECT_NE(output.find("[INFO ]"), std::string::npos);
	EXPECT_NE(output.find("2 + 3 = 5"), std::string::npos);
}

TEST(Log, ErrorsGoToStderr) {
	testing::internal::CaptureStderr();
	MOD_ERROR("Failed with code {}", 404);
	std::string output = testing::internal::GetCapturedStderr();

	EXPECT_NE(output.find("[ERROR]"), std::string::npos);
	EXPECT_NE(output.find("Failed with code 404"), std::string::npos);
}
