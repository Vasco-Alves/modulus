#include <modulus/core/log.hpp>

#include <iostream>
#include <mutex>

namespace modulus::core {

	// A static mutex hidden inside this translation unit.
	// It prevents multiple threads from writing to the console at once.
	static std::mutex s_log_mutex;

	void log_output(LogLevel level, std::string_view message) {
		const char* color_code = "";
		const char* level_str = "";
		bool is_error = false;

		switch (level) {
		case LogLevel::Trace:
			color_code = "\033[90m"; // Gray
			level_str = "[TRACE]";
			break;

		case LogLevel::Info:
			color_code = "\033[32m"; // Green
			level_str = "[INFO ]";
			break;

		case LogLevel::Warn:
			color_code = "\033[33m"; // Yellow
			level_str = "[WARN ]";
			break;

		case LogLevel::Error:
			color_code = "\033[31m"; // Red
			level_str = "[ERROR]";
			is_error = true;
			break;

		case LogLevel::Fatal:
			color_code = "\033[41m"; // Red Background
			level_str = "[FATAL]";
			is_error = true;
			break;
		}

		// Lock the mutex!
		// The lock is released automatically when 'lock' goes out of scope (RAII).
		std::lock_guard<std::mutex> lock(s_log_mutex);

		// Select stream
		std::ostream& out = is_error ? std::cerr : std::cout;

		// Print: Color -> Level -> Reset -> Message
		out << color_code << level_str << "\033[0m " << message << std::endl;
	}

} // namespace modulus::core