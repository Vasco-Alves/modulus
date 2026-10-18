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
		g_scrollDeltaY += (float)yoffset;
	}

	static void key_callback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
		// GLFW_PRESS is sent once when a key goes down. Holding it sends GLFW_REPEAT instead,
		// so this only records real presses. (Unknown keys are -1, so we skip those.)
		if (action == GLFW_PRESS && key >= 0)
			g_keysPressed[key] = true;
	}

	GLFWwindow* g_mainWindowHandle = nullptr;

	float g_scrollDeltaY = 0.0f;

	bool g_keysPressed[GLFW_KEY_LAST + 1] = {};

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
			if (g_mainWindowHandle == nullptr) {
				g_mainWindowHandle = m_handle;
				glfwSetScrollCallback(m_handle, scroll_callback);
				glfwSetKeyCallback(m_handle, key_callback);
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

			if (g_mainWindowHandle == m_handle) {
				g_mainWindowHandle = nullptr;
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

			// A new frame starts: forget the main window's scroll and key presses from last frame
			if (m_handle == g_mainWindowHandle) {
				g_scrollDeltaY = 0.0f;
				for (bool& pressed : g_keysPressed)
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