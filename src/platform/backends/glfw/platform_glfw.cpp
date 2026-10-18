#include <modulus/platform/platform.hpp>
#include <modulus/core/log.hpp>
#include "glfw_internal.hpp"

#include <GLFW/glfw3.h>

namespace modulus::platform {

	// =========================================================================
	// Helpers
	// =========================================================================

	// This function catches errors from inside GLFW (e.g. invalid monitor)
	// and pipes them into our own Logging system.
	static void glfw_error_callback(int error, const char* description) {
		MOD_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	static void scroll_callback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset) {
		g_scroll_delta_y += (float)yoffset;
	}

	static void key_callback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
		// GLFW_PRESS is sent once when a key goes down. Holding it sends GLFW_REPEAT instead,
		// so this only records real presses. (Unknown keys are -1, so we skip those.)
		if (action == GLFW_PRESS && key >= 0)
			g_keys_pressed[key] = true;
	}

	static void mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) {
		// Like key_callback: only record the moment a button goes down
		if (action == GLFW_PRESS && button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
			g_mouse_buttons_pressed[button] = true;
	}

	GLFWwindow* g_main_window_handle = nullptr;

	float g_scroll_delta_y = 0.0f;

	bool g_keys_pressed[GLFW_KEY_LAST + 1] = {};

	bool g_mouse_buttons_pressed[GLFW_MOUSE_BUTTON_LAST + 1] = {};

	// How many windows are open right now.
	// GLFW starts when the first window opens and stops when the last one closes,
	// so users never have to start or stop the platform module themselves.
	static int s_window_count = 0;

	static bool start_glfw() {
		// Redirect GLFW errors to modulus logger
		glfwSetErrorCallback(glfw_error_callback);

		if (!glfwInit()) {
			MOD_FATAL("Failed to initialize GLFW!");
			return false;
		}

		MOD_INFO("Platform initialized (Backend: GLFW)");
		return true;
	}

	static void stop_glfw() {
		glfwTerminate();
		MOD_INFO("Platform shutdown.");
	}

	// =========================================================================
	// Internal Class: The actual GLFW implementation
	// =========================================================================
	class GlfwWindow : public IWindow {
	public:
		GlfwWindow(const WindowConfig& config) {
			// The first window starts GLFW
			if (s_window_count == 0 && !start_glfw())
				return;

			// Hint to GLFW that we want a modern OpenGL context (3.3+ Core)
			// This is required for MacOS and recommended for modern Windows apps.
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

			// A hidden window still gets an OpenGL context, so it can also be used to draw offscreen
			glfwWindowHint(GLFW_VISIBLE, config.visible ? GLFW_TRUE : GLFW_FALSE);

			// Debug builds ask for a debug context: in one, the driver describes every OpenGL error
			// (the graphics module logs those descriptions)
#ifndef NDEBUG
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#else
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
#endif

			// Create the window
			m_handle = glfwCreateWindow(
				config.width,
				config.height,
				config.title.c_str(),
				nullptr, // Monitor (nullptr = windowed mode)
				nullptr  // Share
			);

			if (!m_handle) {
				MOD_FATAL("Failed to create GLFW window!");

				// If no other window is open, nothing needs GLFW anymore
				if (s_window_count == 0)
					stop_glfw();
				return;
			}

			s_window_count++;

			// The first window is the "main" window: the Input functions read from it
			if (g_main_window_handle == nullptr) {
				g_main_window_handle = m_handle;
				glfwSetScrollCallback(m_handle, scroll_callback);
				glfwSetKeyCallback(m_handle, key_callback);
				glfwSetMouseButtonCallback(m_handle, mouse_button_callback);
			}

			// Associate this class instance with the raw GLFW window,
			// so static callbacks (like framebuffer_size_callback) can find it again
			glfwSetWindowUserPointer(m_handle, this);
			glfwSetFramebufferSizeCallback(m_handle, framebuffer_size_callback);

			// GLFW creates the OpenGL context together with the window, so the window owns it.
			// Making it "current" means OpenGL calls from now on draw into this window.
			glfwMakeContextCurrent(m_handle);

			// Enable VSync if requested
			glfwSwapInterval(config.vsync ? 1 : 0);

			MOD_INFO("Window created: {0}x{1} ({2})", config.width, config.height, config.title);
		}

		~GlfwWindow() override {
			// Window creation failed, so there is nothing to clean up
			if (!m_handle)
				return;

			if (g_main_window_handle == m_handle) {
				g_main_window_handle = nullptr;
			}

			glfwDestroyWindow(m_handle);
			m_handle = nullptr;

			// The last window to close stops GLFW
			s_window_count--;
			if (s_window_count == 0)
				stop_glfw();
		}

		bool update() override {
			if (!m_handle)
				return false;

			// A new frame starts: forget the main window's scroll, key presses and clicks from last frame
			if (m_handle == g_main_window_handle) {
				g_scroll_delta_y = 0.0f;
				for (bool& pressed : g_keys_pressed)
					pressed = false;
				for (bool& pressed : g_mouse_buttons_pressed)
					pressed = false;
			}

			glfwPollEvents();		// Process OS events (keyboard, mouse, window resize)

			return !glfwWindowShouldClose(m_handle);
		}

		void close() override {
			if (m_handle)
				glfwSetWindowShouldClose(m_handle, GLFW_TRUE);
		}

		void set_title(const std::string& title) override {
			if (m_handle)
				glfwSetWindowTitle(m_handle, title.c_str());
		}

		int width() const override {
			int w, h;
			glfwGetWindowSize(m_handle, &w, &h);
			return w;
		}

		int height() const override {
			int w, h;
			glfwGetWindowSize(m_handle, &w, &h);
			return h;
		}

		std::pair<int, int> get_size() const override {
			int w, h;
			glfwGetWindowSize(m_handle, &w, &h);
			return { w, h };
		}

		std::pair<int, int> get_framebuffer_size() const override {
			int w, h;
			glfwGetFramebufferSize(m_handle, &w, &h);
			return { w, h };
		}

		void swap_buffers() override {
			if (m_handle)
				glfwSwapBuffers(m_handle);
		}

		GLLoader get_gl_loader() const override {
			// GLFW's function has a slightly different return type, so it needs a cast
			return (GLLoader)glfwGetProcAddress;
		}

		void set_framebuffer_resize_callback(FramebufferResizeCallback callback) override {
			m_framebuffer_resize_callback = callback;
		}

		void* get_native_handle() const override {
			return m_handle;
		}

	private:
		// GLFW calls this when the drawing area changes size.
		// It finds the GlfwWindow through the user pointer and passes the new size on.
		static void framebuffer_size_callback(GLFWwindow* handle, int width, int height) {
			auto* window = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(handle));
			if (window && window->m_framebuffer_resize_callback)
				window->m_framebuffer_resize_callback(width, height);
		}

		GLFWwindow* m_handle = nullptr;
		FramebufferResizeCallback m_framebuffer_resize_callback = nullptr;
	};

	// =========================================================================
	// Factory Implementation
	// =========================================================================

	std::unique_ptr<IWindow> create_window(const WindowConfig& config) {
		auto window = std::make_unique<GlfwWindow>(config);

		// If creation failed, the error was already logged.
		// Return nothing, so the caller can check with: if (!window)
		if (!window->get_native_handle())
			return nullptr;

		return window;
	}

} // namespace modulus::platform