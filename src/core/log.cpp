#include <modulus/core/log.hpp>

#include <cstdio>
#include <iostream>
#include <mutex>

// Each operating system has its own way to ask "is this output a terminal?"
#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN // Only include the basic parts of the Windows API
	#define NOMINMAX            // Don't let Windows define 'min' and 'max' macros
	#include <windows.h>
	#include <io.h>             // _isatty, _fileno, _get_osfhandle

	#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING // Missing from some older Windows headers
		#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
	#endif
#else
	#include <unistd.h>         // isatty, fileno
#endif

namespace modulus::core {

	// A static mutex hidden inside this translation unit.
	// It prevents multiple threads from writing to the console at once.
	static std::mutex s_log_mutex;

	// Colors only make sense in a terminal. When the output goes to a file or to another
	// program, the color codes would show up as garbage like "\033[32m".
	static bool supports_color(FILE* stream) {
#if defined(_WIN32)
		if (!_isatty(_fileno(stream)))
			return false;

		// Windows consoles only understand color codes once this mode is turned on
		HANDLE handle = (HANDLE)_get_osfhandle(_fileno(stream));
		DWORD mode = 0;
		if (!GetConsoleMode(handle, &mode))
			return false;
		return SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
#else
		return isatty(fileno(stream)) != 0;
#endif
	}

	void log_output(LogLevel level, std::string_view message) {
		// Decided once, the first time something is logged
		static const bool s_stdout_color = supports_color(stdout);
		static const bool s_stderr_color = supports_color(stderr);

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
		bool color = is_error ? s_stderr_color : s_stdout_color;

		// Print: Color -> Level -> Reset -> Message
		if (color)
			out << color_code << level_str << "\033[0m " << message << std::endl;
		else
			out << level_str << " " << message << std::endl;
	}

} // namespace modulus::core