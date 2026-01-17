#include <modulus/core/file.hpp>
#include <modulus/core/log.hpp>
#include <fstream>
#include <sstream>

namespace modulus::core {

	std::optional<std::string> File::read_text(const std::string& filepath) {
		std::ifstream stream(filepath);
		if (!stream.is_open()) {
			MOD_ERROR("Failed to open file: {}", filepath);
			return std::nullopt;
		}

		// Read the file buffer into the string stream
		std::stringstream buffer;
		buffer << stream.rdbuf();

		return buffer.str();
	}

}