#pragma once

#include <string>
#include <utility>

namespace modulus::platform {

	// Configuration struct to create a window
	struct WindowConfig {
		std::string title = "Modulus App";
		int width = 1280;
		int height = 720;
		bool vsync = true;
	};

	// A function that finds an OpenGL function by name (e.g. "glClear")
	// and returns its address. The graphics module uses it to load OpenGL.
	using GLLoader = void* (*)(const char* name);

	// The Abstract Interface (User sees this)
	class IWindow {
	public:
		virtual ~IWindow() = default;

		// Core loop functions
		virtual bool update() = 0;       // Polls events. Returns false if window should close.
		virtual void close() = 0;        // Manually signal close


		virtual void set_title(const std::string& title) = 0;
		virtual int width() const = 0;
		virtual int height() const = 0;
		virtual std::pair<int, int> get_size() const = 0;

		// OpenGL support
		// (With GLFW, the OpenGL context is created together with the window, so the window owns it)
		virtual void swap_buffers() = 0;             // Shows the frame that was just drawn
		virtual GLLoader get_gl_loader() const = 0;  // Used by the graphics module to load OpenGL

		// Returns the backend's own window (GLFWwindow* for GLFW, cast to void*).
		// Only needed to connect other libraries directly, for example Dear ImGui.
		virtual void* get_native_handle() const = 0;
	};

} // namespace modulus::platform