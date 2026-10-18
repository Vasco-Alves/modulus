#pragma once

#include <string_view>
#include <format>
#include <utility>
// Note: <source_location> later for advanced debugging

namespace modulus::core {

	enum class LogLevel {
		Trace,
		Info,
		Warn,
		Error,
		Fatal
	};

	// The raw function that actually writes to the console.
	// Users rarely call this directly; they use the helper macros below.
	void log_output(LogLevel level, std::string_view message);

	// Template helper to format the string BEFORE passing it to the logger.
	// std::format_string checks the format string while compiling, so a mistake like
	// MOD_INFO("Loaded {} textures") without the number is a compile error, not a crash.
	template<typename... Args>
	void log(LogLevel level, std::format_string<Args...> fmt, Args&&... args) {
		log_output(level, std::format(fmt, std::forward<Args>(args)...));
	}

} // namespace modulus::core

// =============================================================================
// Macros (The Public API)
// =============================================================================

// In the future, we can wrap these in #ifdef NDEBUG to disable them in Release builds
#define MOD_TRACE(...) ::modulus::core::log(::modulus::core::LogLevel::Trace, __VA_ARGS__)
#define MOD_INFO(...)  ::modulus::core::log(::modulus::core::LogLevel::Info, __VA_ARGS__)
#define MOD_WARN(...)  ::modulus::core::log(::modulus::core::LogLevel::Warn, __VA_ARGS__)
#define MOD_ERROR(...) ::modulus::core::log(::modulus::core::LogLevel::Error, __VA_ARGS__)
#define MOD_FATAL(...) ::modulus::core::log(::modulus::core::LogLevel::Fatal, __VA_ARGS__)