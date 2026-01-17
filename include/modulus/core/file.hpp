#pragma once

#include <string>
#include <optional>

namespace modulus::core {

	struct File {
		// Reads an entire text file into a string.
		// Returns std::nullopt if file fails to open.
		static std::optional<std::string> read_text(const std::string& filepath);
	};

}