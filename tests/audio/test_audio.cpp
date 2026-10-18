// These tests open the default audio device and skip themselves when there is none.
// They only ever play silence, so running them makes no noise.

#include <gtest/gtest.h>
#include <modulus/audio/audio.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <thread>
#include <vector>

using namespace modulus;
namespace fs = std::filesystem;

namespace {

	// Writes a WAV file with the given length of silence (16-bit mono)
	fs::path write_silent_wav(const char* name, int milliseconds) {
		const uint32_t sample_rate = 44100;
		const uint16_t channels = 1;
		const uint16_t bits = 16;
		const uint32_t frame_size = channels * bits / 8;
		const uint32_t data_size = sample_rate * milliseconds / 1000 * frame_size;

		fs::path path = fs::temp_directory_path() / name;
		std::ofstream out(path, std::ios::binary);
		auto write = [&out](auto value) { out.write(reinterpret_cast<const char*>(&value), sizeof(value)); };

		out.write("RIFF", 4);
		write(uint32_t(36 + data_size)); // Size of everything after this
		out.write("WAVE", 4);

		out.write("fmt ", 4);
		write(uint32_t(16));             // Size of this "fmt " part
		write(uint16_t(1));              // Uncompressed (PCM)
		write(channels);
		write(sample_rate);
		write(uint32_t(sample_rate * frame_size)); // Bytes per second
		write(uint16_t(frame_size));
		write(bits);

		out.write("data", 4);
		write(data_size);
		std::vector<char> silence(data_size, 0);
		out.write(silence.data(), silence.size());

		return path;
	}

	class AudioTest : public ::testing::Test {
	protected:
		void SetUp() override {
			device = audio::create_device();
			if (!device)
				GTEST_SKIP() << "No audio device available";
		}

		void TearDown() override {
			std::error_code ignored;
			fs::remove(short_sound, ignored);
			fs::remove(long_sound, ignored);
		}

		std::unique_ptr<audio::Device> device;
		fs::path short_sound = write_silent_wav("modulus_test_short.wav", 50);
		fs::path long_sound = write_silent_wav("modulus_test_long.wav", 2000);
	};

} // namespace

TEST_F(AudioTest, LoadSoundReturnsNullForAMissingFile) {
	EXPECT_EQ(device->load_sound("this/sound/does/not/exist.wav"), nullptr);
}

TEST_F(AudioTest, PlayAndStop) {
	auto sound = device->load_sound(long_sound.string());
	ASSERT_NE(sound, nullptr);
	EXPECT_FALSE(sound->is_playing());

	sound->play();
	EXPECT_TRUE(sound->is_playing());

	sound->stop();
	EXPECT_FALSE(sound->is_playing());
}

TEST_F(AudioTest, SoundStopsAtTheEndUnlessLooping) {
	auto once = device->load_sound(short_sound.string());
	auto looping = device->load_sound(short_sound.string());
	ASSERT_NE(once, nullptr);
	ASSERT_NE(looping, nullptr);

	looping->set_looping(true);
	once->play();
	looping->play();
	std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 10 times as long as the sound

	EXPECT_FALSE(once->is_playing());
	EXPECT_TRUE(looping->is_playing());
}

TEST_F(AudioTest, SoundCanOutliveTheDevice) {
	auto sound = device->load_sound(long_sound.string());
	ASSERT_NE(sound, nullptr);

	device.reset(); // The sound keeps the audio engine running...
	sound->play();
	EXPECT_TRUE(sound->is_playing());
	sound.reset();  // ...until it is gone too
}

TEST_F(AudioTest, PlaySoundIsFireAndForget) {
	device->play_sound(short_sound.string());
	device->play_sound(short_sound.string()); // A second copy on top of the first
	device->play_sound("this/sound/does/not/exist.wav"); // Logs an error, nothing else
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
