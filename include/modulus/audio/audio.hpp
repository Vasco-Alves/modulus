#pragma once

#include <string>

namespace modulus::audio {

	bool init();

	void shutdown();

	void play_sound(const std::string& path);

} // namespace modulus::audio