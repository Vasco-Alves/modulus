#include <modulus/audio/audio.hpp>
#include <modulus/core/log.hpp>

// This define tells the compiler: "Build the library code here!"
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace modulus::audio {

	namespace detail {

		// The miniaudio engine: it mixes everything that plays and sends it to the speakers.
		// The Device and every Sound share it (through std::shared_ptr), so it keeps running
		// until the last of them is gone.
		struct Engine {
			ma_engine engine;
			bool started = false;

			Engine() {
				started = ma_engine_init(nullptr, &engine) == MA_SUCCESS;
			}

			~Engine() {
				if (started) {
					ma_engine_uninit(&engine);
					MOD_INFO("Audio shutdown.");
				}
			}

			// miniaudio keeps pointers into 'engine', so it must never be copied
			Engine(const Engine&) = delete;
			Engine& operator=(const Engine&) = delete;
		};

		// One sound, loaded from a file
		struct SoundData {
			ma_sound sound;
			bool loaded = false;

			SoundData(Engine& engine, const std::string& path) {
				// Flags 0: the file is kept in memory as it is (compressed) and decoded while it plays
				loaded = ma_sound_init_from_file(&engine.engine, path.c_str(), 0, nullptr, nullptr, &sound) == MA_SUCCESS;
			}

			~SoundData() {
				if (loaded)
					ma_sound_uninit(&sound);
			}

			SoundData(const SoundData&) = delete;
			SoundData& operator=(const SoundData&) = delete;
		};

	} // namespace detail

	// =========================================================================
	// Device
	// =========================================================================

	std::unique_ptr<Device> create_device() {
		auto engine = std::make_shared<detail::Engine>();
		if (!engine->started) {
			MOD_ERROR("Failed to start audio (is there an audio device?)");
			return nullptr;
		}

		MOD_INFO("Audio initialized (Backend: miniaudio)");

		// The constructor is private (only this function makes devices), so std::make_unique can't call it
		return std::unique_ptr<Device>(new Device(std::move(engine)));
	}

	Device::Device(std::shared_ptr<detail::Engine> engine) : m_engine(std::move(engine)) {
	}

	Device::~Device() = default;

	void Device::play_sound(const std::string& path) {
		// "Fire and forget": miniaudio frees the sound by itself when it ends
		if (ma_engine_play_sound(&m_engine->engine, path.c_str(), nullptr) != MA_SUCCESS)
			MOD_ERROR("Failed to play sound: {}", path);
	}

	std::unique_ptr<Sound> Device::load_sound(const std::string& path) {
		auto data = std::make_unique<detail::SoundData>(*m_engine, path);
		if (!data->loaded) {
			MOD_ERROR("Failed to load sound: {}", path);
			return nullptr;
		}

		return std::unique_ptr<Sound>(new Sound(m_engine, std::move(data)));
	}

	void Device::set_volume(float volume) {
		ma_engine_set_volume(&m_engine->engine, volume);
	}

	// =========================================================================
	// Sound
	// =========================================================================

	Sound::Sound(std::shared_ptr<detail::Engine> engine, std::unique_ptr<detail::SoundData> data)
		: m_engine(std::move(engine)), m_data(std::move(data)) {
	}

	// m_data (the sound) is destroyed first, then m_engine lets go of the engine
	Sound::~Sound() = default;

	void Sound::play() {
		ma_sound_seek_to_pcm_frame(&m_data->sound, 0);
		ma_sound_start(&m_data->sound);
	}

	void Sound::stop() {
		ma_sound_stop(&m_data->sound);
	}

	bool Sound::is_playing() const {
		return ma_sound_is_playing(&m_data->sound) != MA_FALSE;
	}

	void Sound::set_volume(float volume) {
		ma_sound_set_volume(&m_data->sound, volume);
	}

	void Sound::set_looping(bool looping) {
		ma_sound_set_looping(&m_data->sound, looping ? MA_TRUE : MA_FALSE);
	}

} // namespace modulus::audio
