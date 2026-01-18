#include <modulus/audio/audio.hpp>
#include <modulus/core/log.hpp>

// This define tells the compiler: "Build the library code here!"
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace modulus::audio {

	// The main engine object
	static ma_engine s_engine;
	static bool s_initialized = false;

	bool init() {
		ma_result result;

		// Initialize the high-level audio engine
		result = ma_engine_init(NULL, &s_engine);

		if (result != MA_SUCCESS) {
			MOD_ERROR("Failed to initialize audio engine.");
			return false;
		}

		s_initialized = true;
		MOD_INFO("Audio Subsystem initialized.");
		return true;
	}

	void shutdown() {
		if (s_initialized) {
			ma_engine_uninit(&s_engine);
			s_initialized = false;
		}
	}

	void play_sound(const std::string& path) {
		if (!s_initialized) {
			MOD_WARN("Attempted to play sound before audio init.");
			return;
		}

		// "Fire and forget" playback
		// In a real engine, we would pre-load sounds into memory (SoundBank).
		// For now, streaming from disk is fine for testing.
		ma_result result = ma_engine_play_sound(&s_engine, path.c_str(), NULL);

		if (result != MA_SUCCESS) {
			MOD_ERROR("Failed to play sound: {}", path);
		}
	}

} // namespace modulus::audio