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
		bool visible = true; // false creates the window hidden (for example for tests or tools)
	};

	// A function that finds an OpenGL function by name (e.g. "glClear")
	// and returns its address. The graphics module uses it to load OpenGL.
	using GLLoader = void* (*)(const char* name);

	// A function the window calls when its drawing area changes size (new size in pixels)
	using FramebufferResizeCallback = void (*)(int width, int height);

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

		// Size of the area that is drawn into, in pixels. Usually the same as get_size(), but it can be
		// bigger on high-DPI screens (for example macOS Retina displays). Use this one for viewports.
		virtual std::pair<int, int> get_framebuffer_size() const = 0;

		// OpenGL support
		// (With GLFW, the OpenGL context is created together with the window, so the window owns it)
		virtual void swap_buffers() = 0;             // Shows the frame that was just drawn
		virtual GLLoader get_gl_loader() const = 0;  // Used by the graphics module to load OpenGL

		// Used by the graphics module to keep its viewport the size of the window.
		// (There is one callback per window, so setting your own replaces the graphics module's.)
		virtual void set_framebuffer_resize_callback(FramebufferResizeCallback callback) = 0;

		// Returns the backend's own window (GLFWwindow* for GLFW, cast to void*).
		// Only needed to connect other libraries directly, for example Dear ImGui.
		virtual void* get_native_handle() const = 0;
	};

} // namespace modulus::platform