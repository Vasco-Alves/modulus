#include "modulus/core/file.hpp"
#include "modulus/core/log.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

// Each operating system has its own way to ask "where is this program?"
#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN // Only include the basic parts of the Windows API
	#define NOMINMAX            // Don't let Windows define 'min' and 'max' macros
	#include <windows.h>
#elif defined(__APPLE__)
	#include <mach-o/dyld.h>
	#include <climits>
#endif

namespace modulus::core {

	// Asks the operating system for the folder that contains the running program.
	// Returns an empty path if that fails, so app_path() falls back to the working directory.
	static std::filesystem::path find_app_dir() {
#if defined(_WIN32)
		wchar_t buffer[MAX_PATH];
		if (GetModuleFileNameW(nullptr, buffer, MAX_PATH) == 0)
			return {};
		return std::filesystem::path(buffer).parent_path();

#elif defined(__APPLE__)
		char buffer[PATH_MAX];
		uint32_t size = sizeof(buffer);
		if (_NSGetExecutablePath(buffer, &size) != 0)
			return {};
		return std::filesystem::path(buffer).parent_path();

#else
		// Linux: "/proc/self/exe" is a link that points to the running program
		std::error_code error;
		std::filesystem::path program = std::filesystem::read_symlink("/proc/self/exe", error);
		if (error)
			return {};
		return program.parent_path();
#endif
	}

	std::string File::app_path(const std::string& relative_path) {
		// The program can't move while it runs, so we only ask the OS once
		static const std::filesystem::path app_dir = find_app_dir();

		// make_preferred() uses the OS's slash style, e.g. "bin\assets\test.mp3" on Windows
		return (app_dir / relative_path).make_preferred().string();
	}

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