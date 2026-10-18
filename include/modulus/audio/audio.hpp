#pragma once

#include <memory>
#include <string>

// miniaudio's own types, for get_native_handle(). Declared here, so this header doesn't need
// miniaudio.h. Include <miniaudio.h> yourself to use them.
struct ma_engine;
struct ma_sound;

namespace modulus::audio {

	// Defined in audio.cpp, so miniaudio (the library that does the work) stays out of this header
	namespace detail {
		struct Engine;
		struct SoundData;
	}

	// A loaded sound you can play, stop, loop and change the volume of. Get one from Device::load_sound().
	// It keeps the audio running while it exists, so it's fine if it outlives the Device.
	class Sound {
	public:
		~Sound();

		// Owns the loaded sound, so copying is not allowed (both copies would free it)
		Sound(const Sound&) = delete;
		Sound& operator=(const Sound&) = delete;

		void play(); // Starts from the beginning (restarts it if it is already playing)
		void stop();
		bool is_playing() const;

		void set_volume(float volume);  // 0 = silent, 1 = as recorded
		void set_looping(bool looping); // A looping sound starts over when it reaches the end

		// The miniaudio sound underneath, for what Modulus doesn't wrap (pitch, panning, fades, a
		// position in 3D, ...): include <miniaudio.h> and call miniaudio's ma_sound_* functions on it.
		// Don't uninit it: the Sound does that.
		ma_sound* get_native_handle() const;

	private:
		friend class Device;
		Sound(std::shared_ptr<detail::Engine> engine, std::unique_ptr<detail::SoundData> data);

		std::shared_ptr<detail::Engine> m_engine; // Declared first, so it's let go of last (after the sound)
		std::unique_ptr<detail::SoundData> m_data;
	};

	// Plays sounds on the computer's default output (speakers or headphones).
	// Audio runs while the Device exists, like the platform module runs while a window exists,
	// so there is no init() or shutdown().
	class Device {
	public:
		~Device();

		// Owns the audio engine, so copying is not allowed
		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;

		// Plays a sound file once in the background ("fire and forget").
		// Calling it again before the sound ends plays another copy on top.
		void play_sound(const std::string& path);

		// Loads a sound to control yourself (play, stop, loop, volume).
		// Returns nullptr if the file can't be loaded (the error is logged).
		std::unique_ptr<Sound> load_sound(const std::string& path);

		void set_volume(float volume); // Volume of everything: 0 = silent, 1 = normal

		// The miniaudio engine underneath, for what Modulus doesn't wrap (the listener for 3D sound,
		// sound groups, effects, ...): include <miniaudio.h> and call miniaudio's functions on it.
		// Don't define MINIAUDIO_IMPLEMENTATION (Modulus already builds miniaudio), and don't uninit
		// the engine: the Device and its Sounds do that.
		ma_engine* get_native_handle() const;

	private:
		friend std::unique_ptr<Device> create_device();
		explicit Device(std::shared_ptr<detail::Engine> engine);

		std::shared_ptr<detail::Engine> m_engine;
	};

	// Starts audio on the default output device.
	// Returns nullptr if that fails, for example when there is no audio device (the error is logged),
	// so you can check it with: if (!audio)
	std::unique_ptr<Device> create_device();

} // namespace modulus::audio
