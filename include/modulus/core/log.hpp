#pragma once

#include <string_view>
#include <format> 
// Note: We might need <source_location> later for advanced debugging!

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
	template<typename... Args>
	void log(LogLevel level, std::string_view fmt, Args&&... args) {
		auto msg = std::vformat(fmt, std::make_format_args(args...));
		log_output(level, msg);
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