#pragma once

#include <string>
#include <optional>

namespace modulus::core {

	struct File {
		// Reads an entire text file into a string.
		static std::optional<std::string> read_text(const std::string& filepath);
	};

}