#pragma once

#include <string>
#include <optional>

namespace modulus::core {

	struct File {
		// Reads an entire text file into a string.
		static std::optional<std::string> read_text(const std::string& filepath);

		// Turns a path relative to the program's own folder into a full path.
		// Use it for files that ship next to your program (like an "assets" folder),
		// so they are found no matter which folder the program was started from.
		//   File::app_path("assets/test.mp3")  ->  "<folder of the program>/assets/test.mp3"
		static std::string app_path(const std::string& relative_path);
	};

}