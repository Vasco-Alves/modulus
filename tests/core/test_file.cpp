#include <gtest/gtest.h>
#include <modulus/core/file.hpp>

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using modulus::core::File;

TEST(File, ReadTextReturnsTheWholeFile) {
	fs::path path = fs::temp_directory_path() / "modulus_test_read_text.txt";
	{
		std::ofstream out(path);
		out << "first line\nsecond line\n";
	}

	auto text = File::read_text(path.string());
	fs::remove(path);

	ASSERT_TRUE(text.has_value());
	EXPECT_EQ(*text, "first line\nsecond line\n");
}

TEST(File, ReadTextReturnsNothingForAMissingFile) {
	EXPECT_FALSE(File::read_text("this/file/does/not/exist.txt").has_value());
}

TEST(File, AppPathPointsNextToTheProgram) {
	fs::path path = File::app_path("assets/hello.txt");

	EXPECT_TRUE(path.is_absolute());
	EXPECT_EQ(path.filename().string(), "hello.txt");
	EXPECT_EQ(path.parent_path().filename().string(), "assets");

	// The folder above "assets" is the one this test program is in
	fs::path program_dir = path.parent_path().parent_path();
	EXPECT_TRUE(fs::exists(program_dir / "modulus_tests") || fs::exists(program_dir / "modulus_tests.exe"));
}
